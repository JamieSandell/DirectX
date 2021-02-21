//=============================================================================
// BillboardingDemo.cpp.
//
// Demonstrates how to do billboarding in HLSL.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//=============================================================================

#include "BillboardingDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	BillboardingDemo app(hInstance, "Billboarding in HLSL Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

BillboardingDemo::BillboardingDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius    = 50.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 20.0f;

	mAmbientLight   = WHITE;
	mDiffuseLight   = WHITE;
	mSpecLight      = WHITE;
	mLightVecW      = D3DXVECTOR3(0.0, 0.0f, -1.0f);

	mGridMtrl     = Mtrl(WHITE*0.7f, WHITE, WHITE*0.5f, 16.0f);
	mTreeMtrl = Mtrl(WHITE*0.7f, WHITE, WHITE*0.5f, 16.0f);

	HR(D3DXCreateTextureFromFile(gd3dDevice, "tree.dds", &mTreeTex));
	HR(D3DXCreateTextureFromFile(gd3dDevice, "ground0.dds", &mGridTex));

	buildGeoBuffers();
	buildFX();

	int numTreeVerts    = 4 * 14;
	int numTreeTris     = 2 * 14;

	mGfxStats->addVertices(mNumGridVertices);
	mGfxStats->addVertices(numTreeVerts);
	mGfxStats->addTriangles(mNumGridTriangles);
	mGfxStats->addTriangles(numTreeTris);

	onResetDevice();
}

BillboardingDemo::~BillboardingDemo()
{
	delete mGfxStats;
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mVBTrees);
	ReleaseCOM(mIBTrees);
	ReleaseCOM(mFX);
	ReleaseCOM(mTreeTex);
	ReleaseCOM(mGridTex);

	DestroyAllVertexDeclarations();
}

bool BillboardingDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	// Check for vertex shader version 2.0 support.
	if( caps.VertexShaderVersion < D3DVS_VERSION(2, 0) )
		return false;

	// Check for pixel shader version 2.0 support.
	if( caps.PixelShaderVersion < D3DPS_VERSION(2, 0) )
		return false;

	return true;
}

void BillboardingDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void BillboardingDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void BillboardingDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if( gDInput->keyDown(DIK_W) )	 
		mCameraHeight   += 25.0f * dt;
	if( gDInput->keyDown(DIK_S) )	 
		mCameraHeight   -= 25.0f * dt;

	// Divide to make mouse less sensitive. 
	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius    += gDInput->mouseDY() / 25.0f;

	// If we rotate over 360 degrees, just roll back to 0
	if( fabsf(mCameraRotationY) >= 2.0f * D3DX_PI ) 
		mCameraRotationY = 0.0f;

	// Don't let radius get too small.
	if( mCameraRadius < 5.0f )
		mCameraRadius = 5.0f;

	// The camera position/orientation relative to world space can 
	// change every frame based on input, so we need to rebuild the
	// view matrix every frame with the latest changes.
	buildViewMtx();
}

void BillboardingDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	// Setup the rendering FX
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecLight, &mSpecLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));

	HR(mFX->SetMatrix(mhViewProj, &(mView*mProj)));

	// Begin passes.
	mhTech = mFX->GetTechniqueByName("DirLightTexTech");
	HR(mFX->SetTechnique(mhTech));
	HR(mFX->CommitChanges());
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawGrid();

		HR(mFX->EndPass());
	}
	HR(mFX->End());

	mhTech = mFX->GetTechniqueByName("BillBoard");
	HR(mFX->SetTechnique(mhTech));
	HR(mFX->CommitChanges());
	numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawTrees();

		HR(mFX->EndPass());
	}
	HR(mFX->End());


	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void BillboardingDemo::buildGeoBuffers()
{
	// Trees generation
	// Obtain a pointer to a new vertex buffer.
	HR(gd3dDevice->CreateVertexBuffer(4 * sizeof(VertexPNT), 
		D3DUSAGE_WRITEONLY,	0, D3DPOOL_MANAGED, &mVBTrees, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// tree's vertex data.
	VertexPNT* v = 0;
	HR(mVBTrees->Lock(0, 0, (void**)&v, 0));

	float treeSize = 10.0f;
	v[0].pos = D3DXVECTOR3(-treeSize, -treeSize, 0.0f); //bottom left
	v[0].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	v[0].tex0 = D3DXVECTOR2(0.0f, 1.0f);

	//top left
	v[1].pos = D3DXVECTOR3(-treeSize, +treeSize, 0.0f);
	v[1].normal = v[0].normal;
	v[1].tex0 = D3DXVECTOR2(0.0f, 0.0f);

	//bottom right
	v[2].pos = D3DXVECTOR3(+treeSize, -treeSize, 0.0f);
	v[2].normal = v[0].normal;
	v[2].tex0 = D3DXVECTOR2(1.0f, 1.0f);

	// top right
	v[3].pos = D3DXVECTOR3(+treeSize, +treeSize, 0.0f);
	v[3].normal = v[0].normal;
	v[3].tex0 = D3DXVECTOR2(1.0f, 0.0f);

	HR(mVBTrees->Unlock());

	// Obtain a pointer to a new index buffer.
	HR(gd3dDevice->CreateIndexBuffer(2*3*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIBTrees, 0)); //numberOfTriangles*numberOfTrianglerVertices*sizeof(WORD)

	// Now lock it to obtain a pointer to its internal data, and write the
	// tree's index data.

	WORD* k = 0;
	HR(mIBTrees->Lock(0, 0, (void**)&k, 0));

	k[0] = 0;
	k[1] = 1;
	k[2] = 2;
	k[3] = 1;
	k[4] = 3;
	k[5] = 2;

	HR(mIBTrees->Unlock());


	// Ground generation
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, 
		D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	// Save vertex count and triangle count for DrawIndexedPrimitive arguments.
	mNumGridVertices  = 100*100;
	mNumGridTriangles = 99*99*2;

	// Obtain a pointer to a new vertex buffer.
	HR(gd3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexPNT), 
		D3DUSAGE_WRITEONLY,	0, D3DPOOL_MANAGED, &mVB, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// grid's vertex data.
	VertexPNT* v2 = 0;
	HR(mVB->Lock(0, 0, (void**)&v2, 0));

	float texScale = 0.2f;
	for(int i = 0; i < 100; ++i)
	{
		for(int j = 0; j < 100; ++j)
		{
			DWORD index = i * 100 + j;
			v2[index].pos    = verts[index];
			v2[index].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			v2[index].tex0 = D3DXVECTOR2((float)j, (float)i) * texScale;
		}
	}

	HR(mVB->Unlock());


	// Obtain a pointer to a new index buffer.
	HR(gd3dDevice->CreateIndexBuffer(mNumGridTriangles*3*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIB, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// grid's index data.

	WORD* k2 = 0;
	HR(mIB->Lock(0, 0, (void**)&k2, 0));

	for(DWORD i = 0; i < mNumGridTriangles*3; ++i)
		k2[i] = (WORD)indices[i];

	HR(mIB->Unlock());
}

void BillboardingDemo::buildFX()
{
	// Create the FX from a .fx file.
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, "dirLightTex.fx", 
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech           = mFX->GetTechniqueByName("DirLightTexTech");
	mhWVP            = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans  = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhEyePos         = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld          = mFX->GetParameterByName(0, "gWorld");
	mhAmbientLight   = mFX->GetParameterByName(0, "gAmbientLight");
	mhDiffuseLight   = mFX->GetParameterByName(0, "gDiffuseLight");
	mhSpecLight      = mFX->GetParameterByName(0, "gSpecularLight");
	mhLightVecW      = mFX->GetParameterByName(0, "gLightVecW");
	mhAmbientMtrl    = mFX->GetParameterByName(0, "gAmbientMtrl");
	mhDiffuseMtrl    = mFX->GetParameterByName(0, "gDiffuseMtrl");
	mhSpecMtrl       = mFX->GetParameterByName(0, "gSpecularMtrl");
	mhSpecPower      = mFX->GetParameterByName(0, "gSpecularPower");
	mhTex            = mFX->GetParameterByName(0, "gTex");
	mhViewProj		 = mFX->GetParameterByName(0, "gViewProj");
	mhBBOffset		 = mFX->GetParameterByName(0, "gBBOffset");
}

void BillboardingDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void BillboardingDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void BillboardingDemo::drawGrid()
{
	HR(gd3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexPNT)));
	HR(gd3dDevice->SetIndices(mIB));
	HR(gd3dDevice->SetVertexDeclaration(VertexPNT::Decl));

	D3DXMATRIX W, WIT;
	D3DXMatrixIdentity(&W);
	D3DXMatrixInverse(&WIT, 0, &W);
	D3DXMatrixTranspose(&WIT, &WIT);
	HR(mFX->SetMatrix(mhWorld, &W));
	HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
	HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));

	HR(mFX->SetValue(mhAmbientMtrl, &mGridMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mGridMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecMtrl, &mGridMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecPower, mGridMtrl.specPower));

	HR(mFX->SetTexture(mhTex, mGridTex));

	HR(mFX->CommitChanges());
	HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumGridVertices, 0, mNumGridTriangles));
}

void BillboardingDemo::drawTrees()
{
	//HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAP_U));
	HR(gd3dDevice->SetStreamSource(0, mVBTrees, 0, sizeof(VertexPNT)));
	HR(gd3dDevice->SetIndices(mIBTrees));
	HR(gd3dDevice->SetVertexDeclaration(VertexPNT::Decl));

	D3DXMATRIX T, R, W, WIT;

	D3DXMatrixRotationX(&R, -D3DX_PI*0.5f);

	HR(mFX->SetValue(mhAmbientMtrl, &mTreeMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mTreeMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecMtrl, &mTreeMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecPower, mTreeMtrl.specPower));

	HR(mFX->SetTexture(mhTex, mTreeTex));
	HR(mFX->SetMatrix(mhViewProj, &(mView*mProj)));
	for(int z = -30; z <= 30; z+= 10)
	{
		//D3DXMatrixTranslation(&T, 10.0f, 10.0f, float(z));
		////W = R*T;
		//W = T;
		//D3DXMatrixInverse(&WIT, 0, &W);
		//D3DXMatrixTranspose(&WIT, &WIT);

		//HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		//HR(mFX->SetMatrix(mhWorld, &W));
		//HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		D3DXVECTOR3 bbOffset(10.0f, 9.5f, float(z));
		HR(mFX->SetValue(mhBBOffset, &bbOffset, sizeof(D3DXVECTOR3)));
		HR(mFX->CommitChanges());
		HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2));

		//D3DXMatrixTranslation(&T, -10.0f, 10.0f, float(z));
		//W = T;
		//D3DXMatrixInverse(&WIT, 0, &W);
		//D3DXMatrixTranspose(&WIT, &WIT);

		//HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		//HR(mFX->SetMatrix(mhWorld, &W));
		//HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		bbOffset = D3DXVECTOR3(-10.0f, 9.5f, float(z));
		HR(mFX->SetValue(mhBBOffset, &bbOffset, sizeof(D3DXVECTOR3)));
		HR(mFX->CommitChanges());
		HR(mFX->CommitChanges());
		HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2));
	}
	// Disable.
	HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, 0));
}