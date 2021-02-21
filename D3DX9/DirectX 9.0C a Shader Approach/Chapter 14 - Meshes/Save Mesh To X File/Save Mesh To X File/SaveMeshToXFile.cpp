//=============================================================================
// SaveMeshToXFileDemo.cpp
//
// Demonstrates saving a custom mesh to an x file.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//=============================================================================

#include "SaveMeshToXFile.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	SaveMeshToXFileDemo app(hInstance, "Save Mesh to X File Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

SaveMeshToXFileDemo::SaveMeshToXFileDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius    = 12.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 6.0f;

	mGridMtrl     = Mtrl(BLUE, BLUE, WHITE, 16.0f);

	mLight.dirW    = D3DXVECTOR3(0.0f, -1.0f, 2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec    = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	//LoadXFile("skullocc.x", &mMesh, mMtrl, mTex);
	D3DXMatrixIdentity(&mWorld);

	HR(D3DXCreateTextureFromFile(gd3dDevice, "whitetex.dds", &mWhiteTex));

	//buildGeoBuffers();
	convertGridToMesh();
	saveMeshToXFile();
	buildFX();

	//mGfxStats->addVertices(mMesh->GetNumVertices());
	//mGfxStats->addTriangles(mMesh->GetNumFaces());
	mGfxStats->addVertices(mNumGridVertices);
	mGfxStats->addTriangles(mNumGridTriangles);

	onResetDevice();
}

SaveMeshToXFileDemo::~SaveMeshToXFileDemo()
{
	delete mGfxStats;

	ReleaseCOM(mFX);

	ReleaseCOM(mMesh);
	//for(int i = 0; i < mTex.size(); ++i)
	//	ReleaseCOM(mTex[i]);

	ReleaseCOM(mWhiteTex);

	DestroyAllVertexDeclarations();
}

bool SaveMeshToXFileDemo::checkDeviceCaps()
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

void SaveMeshToXFileDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void SaveMeshToXFileDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void SaveMeshToXFileDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if( gDInput->keyDown(DIK_W) )	 
		mCameraHeight   += 25.0f * dt;
	if( gDInput->keyDown(DIK_S) )	 
		mCameraHeight   -= 25.0f * dt;

	// Divide by 50 to make mouse less sensitive. 
	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius    += gDInput->mouseDY() / 25.0f;

	// If we rotate over 360 degrees, just roll back to 0
	if( fabsf(mCameraRotationY) >= 2.0f * D3DX_PI ) 
		mCameraRotationY = 0.0f;

	// Don't let radius get too small.
	if( mCameraRadius < 2.0f )
		mCameraRadius = 2.0f;

	// The camera position/orientation relative to world space can 
	// change every frame based on input, so we need to rebuild the
	// view matrix every frame with the latest changes.
	buildViewMtx();
}

void SaveMeshToXFileDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));
	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mWorld));


	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	//HR(mFX->Begin(&numPasses, 0));
	//HR(mFX->BeginPass(0));

	//for(int j = 0; j < mMtrl.size(); ++j)
	//{
	//	HR(mFX->SetValue(mhMtrl, &mMtrl[j], sizeof(Mtrl)));

	//	// If there is a texture, then use.
	//	if(mTex[j] != 0)
	//	{
	//		HR(mFX->SetTexture(mhTex, mTex[j]));
	//	}

	//	// But if not, then set a pure white texture.  When the texture color
	//	// is multiplied by the color from lighting, it is like multiplying by
	//	// 1 and won't change the color from lighting.
	//	else
	//	{
	//		HR(mFX->SetTexture(mhTex, mWhiteTex));
	//	}

	//	HR(mFX->CommitChanges());
	//	HR(mMesh->DrawSubset(j));
	//}
	//HR(mFX->EndPass());
	//HR(mFX->End());

	// Begin passes.
	numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawGrid();

		HR(mFX->EndPass());
	}
	HR(mFX->End());

	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void SaveMeshToXFileDemo::drawGrid()
{
	//HR(gd3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexPNT)));
	//HR(gd3dDevice->SetIndices(mIB));
	//HR(gd3dDevice->SetVertexDeclaration(VertexPNT::Decl));

	D3DXMATRIX W, WIT;
	D3DXMatrixIdentity(&W);
	D3DXMatrixInverse(&WIT, 0, &W);
	D3DXMatrixTranspose(&WIT, &WIT);
	HR(mFX->SetMatrix(mhWorld, &W));
	HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
	HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));

	HR(mFX->SetTexture(mhTex, mWhiteTex));
	HR(mFX->SetValue(mhMtrl, &mGridMtrl, sizeof(Mtrl)));

	HR(mFX->CommitChanges());
	//HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumGridVertices, 0, mNumGridTriangles));
	HR(mMesh->DrawSubset(0));
}

void SaveMeshToXFileDemo::buildGeoBuffers()
{
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
	VertexPNT* v = 0;
	HR(mVB->Lock(0, 0, (void**)&v, 0));

	D3DXVECTOR3 normal(0.0f, 1.0f, 0.0f);
	for(int i = 0; i < 100; ++i)
	{
		for(int j = 0; j < 100; ++j)
		{
			DWORD index = i * 100 + j;
			v[index].pos    = verts[index];
			v[index].normal = normal;
			v[index].tex0 = D3DXVECTOR2((float)j, (float)i);
		}
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

void SaveMeshToXFileDemo::buildFX()
{
	// Create the FX from a .fx file.
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, "PhongDirLtTex.fx", 
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech            = mFX->GetTechniqueByName("PhongDirLtTexTech");
	mhWVP             = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans   = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhMtrl            = mFX->GetParameterByName(0, "gMtrl");
	mhLight           = mFX->GetParameterByName(0, "gLight");
	mhEyePos          = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld           = mFX->GetParameterByName(0, "gWorld");
	mhTex             = mFX->GetParameterByName(0, "gTex");
}

void SaveMeshToXFileDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 2.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void SaveMeshToXFileDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void SaveMeshToXFileDemo::convertGridToMesh()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;
	// Generate the grid's vertices and indices in-case buildGeoBuffers wasn't called.
	GenTriGrid(100, 100, 1.0f, 1.0f, 
		D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);
	//Set the number of grid vertices and triangles in-case buildGeoBuffers wasn't called.
	mNumGridVertices  = 100*100;
	mNumGridTriangles = 99*99*2;

	//Set vertex format to VertexPNT.
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::Decl->GetDeclaration(elements, &numElements);

	//Create the mesh
	HR(D3DXCreateMesh(mNumGridTriangles, mNumGridVertices, 0, elements, gd3dDevice, &mMesh));
	// Lock the vertex buffer and write the vertex info.
	VertexPNT* vertices = 0;
	HR(mMesh->LockVertexBuffer(0, (void**)&vertices));

	D3DXVECTOR3 normal(0.0f, 1.0f, 0.0f);
	for(UINT i = 0; i < 100; ++i)
	{
		for(UINT j = 0; j < 100; ++j)
		{
			DWORD index = i * 100 + j;
			vertices[index].pos    = verts[index];
			vertices[index].normal = normal;
			vertices[index].tex0 = D3DXVECTOR2((float)j, (float)i);
		}
	}
	HR(mMesh->UnlockVertexBuffer());

	// Lock the index buffer and write the indices info.
	WORD* k = 0;
	HR(mMesh->LockIndexBuffer(0, (void**)&k));

	for(DWORD i = 0; i < mNumGridTriangles*3; ++i)
		k[i] = (WORD)indices[i];

	HR(mMesh->UnlockIndexBuffer());
}

void SaveMeshToXFileDemo::saveMeshToXFile()
{
	D3DXMATERIAL mtrl[1];

	mtrl[0].MatD3D.Ambient = mGridMtrl.ambient;
	mtrl[0].MatD3D.Diffuse = mGridMtrl.diffuse;
	mtrl[0].MatD3D.Emissive = mGridMtrl.diffuse; // We don't have an emissive property so just set it to the diffuse property.
	mtrl[0].MatD3D.Power = mGridMtrl.specPower;
	mtrl[0].MatD3D.Specular = mGridMtrl.spec;
	mtrl[0].pTextureFilename = "whitetex.dds";
	HR(D3DXSaveMeshToX("grid.x", mMesh, NULL, mtrl, NULL, (DWORD)1, D3DXF_FILEFORMAT_BINARY | D3DXF_FILEFORMAT_COMPRESSED));

}
