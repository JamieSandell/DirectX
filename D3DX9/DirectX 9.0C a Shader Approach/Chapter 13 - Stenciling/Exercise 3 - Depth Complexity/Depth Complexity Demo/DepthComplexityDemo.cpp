//=============================================================================
// DepthComplexityDemo.cpp
//
// Demonstrates Depth Complexity.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//=============================================================================

#include "DepthComplexityDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	DepthComplexityDemo app(hInstance, "Depth Complexity Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

DepthComplexityDemo::DepthComplexityDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	InitAllVertexDeclarations();

	mCameraRadius    = 50.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 20.0f;

	mGridMtrl     = Mtrl(BLUE, BLUE, WHITE, 16.0f);
	mCylinderMtrl = Mtrl(RED, RED, WHITE, 8.0f);
	mSphereMtrl   = Mtrl(GREEN, GREEN, WHITE, 8.0f);

	HR(D3DXCreateCylinder(gd3dDevice, 1.0f, 1.0f, 6.0f, 20, 20, &mCylinder, 0));
	HR(D3DXCreateSphere(gd3dDevice, 1.0f, 20, 20, &mSphere, 0));

	buildGeoBuffers();
	buildQuadBuffers();
	buildFX();

	// If you look at the drawCylinders and drawSpheres functions, you see
	// that we draw 14 cylinders and 14 spheres.
	int numCylVerts    = mCylinder->GetNumVertices() * 14;
	int numSphereVerts = mSphere->GetNumVertices()   * 14;
	int numCylTris     = mCylinder->GetNumFaces()    * 14;
	int numSphereTris  = mSphere->GetNumFaces()      * 14;

	mGfxStats->addVertices(mNumGridVertices);
	mGfxStats->addVertices(numCylVerts);
	mGfxStats->addVertices(numSphereVerts);
	mGfxStats->addTriangles(mNumGridTriangles);
	mGfxStats->addTriangles(numCylTris);
	mGfxStats->addTriangles(numSphereTris);
	// Add the full screen quads
	mGfxStats->addVertices(4*3);
	mGfxStats->addTriangles(2*3);

	onResetDevice();
}

DepthComplexityDemo::~DepthComplexityDemo()
{
	delete mGfxStats;
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mVBQuad);
	ReleaseCOM(mIBQuad);
	ReleaseCOM(mFX);
	ReleaseCOM(mCylinder);
	ReleaseCOM(mSphere);

	DestroyAllVertexDeclarations();
}

bool DepthComplexityDemo::checkDeviceCaps()
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

void DepthComplexityDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void DepthComplexityDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void DepthComplexityDemo::updateScene(float dt)
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

void DepthComplexityDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xffffffff, 1.0f, 0));

	HR(gd3dDevice->SetRenderState(D3DRS_STENCILENABLE,    true));
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILFUNC,      D3DCMP_ALWAYS));
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILREF,       0x0));
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILMASK,      0xffffffff));
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff));
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILPASS,      D3DSTENCILOP_INCRSAT));

	HR(gd3dDevice->BeginScene());

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawGrid();
		drawCylinders();
		drawSpheres();

		HR(mFX->EndPass());
	}
	HR(mFX->End());

	// Begin passes.
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP));
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL));
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILREF, 0x1));

	HR(gd3dDevice->SetRenderState(D3DRS_ZENABLE, false));
	HR(gd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false));

	D3DXHANDLE oldTech = mhTech;
	mhTech = mFX->GetTechniqueByName("FullScreenQuadTech");
	HR(mFX->SetTechnique(mhTech));
	HR(mFX->CommitChanges());

	numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawFullScreenQuad(RED);

		HR(mFX->EndPass());
	}
	HR(mFX->End());

	HR(gd3dDevice->SetRenderState(D3DRS_STENCILREF, 0x2));
	numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawFullScreenQuad(GREEN);

		HR(mFX->EndPass());
	}
	HR(mFX->End());

	HR(gd3dDevice->SetRenderState(D3DRS_STENCILREF, 0x3));
	numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawFullScreenQuad(BLUE);

		HR(mFX->EndPass());
	}
	HR(mFX->End());

	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));

	//reset values
	mhTech = oldTech;
	HR(mFX->SetTechnique(mhTech));
	HR(mFX->CommitChanges());
	HR(gd3dDevice->SetRenderState(D3DRS_STENCILENABLE, false));
	HR(gd3dDevice->SetRenderState(D3DRS_ZENABLE, true));
	HR(gd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true));
}

void DepthComplexityDemo::drawFullScreenQuad(D3DXCOLOR col)
{
	HR(mFX->SetValue(mhQuadCol, &col, sizeof(D3DXCOLOR)));

	HR(gd3dDevice->SetStreamSource(0, mVBQuad, 0, sizeof(VertexCol)));
	HR(gd3dDevice->SetIndices(mIBQuad));
	HR(gd3dDevice->SetVertexDeclaration(VertexCol::Decl));

	HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2));
}

void DepthComplexityDemo::buildQuadBuffers()
{
	// Obtain a pointer to a new vertex buffer.
	HR(gd3dDevice->CreateVertexBuffer(4 * sizeof(VertexCol), 
		D3DUSAGE_WRITEONLY,	0, D3DPOOL_MANAGED, &mVBQuad, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// quad's vertex data.
	VertexCol* v = 0;
	HR(mVBQuad->Lock(0, 0, (void**)&v, 0));

	v[0].pos = D3DXVECTOR3(-1.0f, -1.0f, 1.0f); //bottom left
	v[1].pos = D3DXVECTOR3(-1.0f, 1.0f, 1.0f); //top left
	v[2].pos = D3DXVECTOR3(1.0f, 1.0f, 1.0f); //top right
	v[3].pos = D3DXVECTOR3(1.0f, -1.0f, 1.0f); //bottom right

	HR(mVBQuad->Unlock());


	// Obtain a pointer to a new index buffer.
	HR(gd3dDevice->CreateIndexBuffer(6*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIBQuad, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// quad's index data.

	WORD* k = 0;
	HR(mIBQuad->Lock(0, 0, (void**)&k, 0));

	k[0] = 0;
	k[1] = 1;
	k[2] = 2;
	k[3] = 2;
	k[4] = 3;
	k[5] = 0;

	HR(mIBQuad->Unlock());
}

void DepthComplexityDemo::buildGeoBuffers()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, 
		D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	// Save vertex count and triangle count for DrawIndexedPrimitive arguments.
	mNumGridVertices  = 100*100;
	mNumGridTriangles = 99*99*2;

	// Obtain a pointer to a new vertex buffer.
	HR(gd3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexCol), 
		D3DUSAGE_WRITEONLY,	0, D3DPOOL_MANAGED, &mVB, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// grid's vertex data.
	VertexCol* v = 0;
	HR(mVB->Lock(0, 0, (void**)&v, 0));

	for(DWORD i = 0; i < mNumGridVertices; ++i)
	{
		v[i].pos = verts[i];
	}

	HR(mVB->Unlock());


	// Obtain a pointer to a new index buffer.
	HR(gd3dDevice->CreateIndexBuffer(mNumGridTriangles*3*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIB, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// grid's index data.

	WORD* k = 0;
	HR(mIB->Lock(0, 0, (void**)&k, 0));

	for(DWORD i = 0; i < mNumGridTriangles*3; ++i)
		k[i] = (WORD)indices[i];

	HR(mIB->Unlock());
}

void DepthComplexityDemo::buildFX()
{
	// Create the FX from a .fx file.
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, "pointlight.fx", 
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech           = mFX->GetTechniqueByName("PosColTech");
	mhWVP            = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans  = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhQuadCol        = mFX->GetParameterByName(0, "gQuadCol");
	mhWorld          = mFX->GetParameterByName(0, "gWorld");
}

void DepthComplexityDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void DepthComplexityDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void DepthComplexityDemo::drawGrid()
{
	HR(gd3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexCol)));
	HR(gd3dDevice->SetIndices(mIB));
	HR(gd3dDevice->SetVertexDeclaration(VertexCol::Decl));

	D3DXMATRIX W, WIT;
	D3DXMatrixIdentity(&W);
	D3DXMatrixInverse(&WIT, 0, &W);
	D3DXMatrixTranspose(&WIT, &WIT);
	HR(mFX->SetMatrix(mhWorld, &W));
	HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
	HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));

	HR(mFX->CommitChanges());
	HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumGridVertices, 0, mNumGridTriangles));
}

void DepthComplexityDemo::drawCylinders()
{
	D3DXMATRIX T, R, W, WIT;

	D3DXMatrixRotationX(&R, D3DX_PI*0.5f);

	for(int z = -30; z <= 30; z+= 10)
	{
		D3DXMatrixTranslation(&T, -10.0f, 3.0f, (float)z);
		W = R*T;
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mCylinder->DrawSubset(0));

		D3DXMatrixTranslation(&T, 10.0f, 3.0f, (float)z);
		W = R*T;
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mCylinder->DrawSubset(0));
	}
}

void DepthComplexityDemo::drawSpheres()
{
	D3DXMATRIX W, WIT;

	for(int z = -30; z <= 30; z+= 10)
	{
		D3DXMatrixTranslation(&W, -10.0f, 7.5f, (float)z);
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mSphere->DrawSubset(0));

		D3DXMatrixTranslation(&W, 10.0f, 7.5f, (float)z);
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mSphere->DrawSubset(0));
	}
}