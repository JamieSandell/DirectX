//=============================================================================
// ConcatenateMeshesDemo.cpp.
//
// Demonstrates how to concatenate two different meshes using
// D3DXConcatenateMeshes
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//=============================================================================

#include "ConcatenateMeshesDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	ConcatenateMeshesDemo app(hInstance, "Concatenate Meshes Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

ConcatenateMeshesDemo::ConcatenateMeshesDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius    = 30.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 10.0f;

	mLight.dirW    = D3DXVECTOR3(1.0f, -1.0f, -2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec    = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	//Load the X Files
	LoadXFile("bigship1.x", &mMeshShip, mMtrlShip, mTexShip);
	LoadXFile("grid.x", &mMeshGrid, mMtrlGrid, mTexGrid);
	// Concatenate mesh code
	LPD3DXMESH meshes[2];
	meshes[0] = mMeshShip;
	meshes[1] = mMeshGrid;
	// Create the transformation Matrices
	D3DXMATRIX shipWorld, gridWorld;
	D3DXMatrixTranslation(&shipWorld, 0.0f, 10.0f, 0.0f); // Translate the ship above the ground
	D3DXMatrixIdentity(&gridWorld);
	D3DXMATRIX matrices[2];
	matrices[0] = shipWorld;
	matrices[1] = gridWorld;
	//Set vertex format to VertexPNT.
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::Decl->GetDeclaration(elements, &numElements);
	// Join the two meshes
	D3DXConcatenateMeshes(meshes, 2, NULL,
		matrices, NULL, elements, gd3dDevice, &mMeshJoined);
	// Clean the mesh
	DWORD* adjaceny;
	adjaceny = new DWORD[3 * mMeshJoined->GetNumFaces() * sizeof(DWORD)];
	DWORD* outAdjaceny;
	outAdjaceny = new DWORD[3 * mMeshJoined->GetNumFaces() * sizeof(DWORD)];
	LPD3DXBUFFER buffErr;

	HR(mMeshJoined->GenerateAdjacency(EPSILON, adjaceny));
	HR(D3DXCleanMesh(D3DXCLEAN_OPTIMIZATION, mMeshJoined, adjaceny, &mMeshJoined, outAdjaceny, &buffErr));
	// Optimise the mesh
	LPD3DXBUFFER vertexRemap;

	HR(mMeshJoined->Optimize(D3DXMESH_MANAGED | 
		D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, adjaceny, NULL, NULL, &vertexRemap, &mMeshJoined));
	// Copy over the texture and material data
	mMtrlJoined.insert(mMtrlJoined.begin(), mMtrlShip.begin(), mMtrlShip.end());
	mMtrlJoined.insert(mMtrlJoined.end(), mMtrlGrid.begin(), mMtrlGrid.end());
	mTexJoined.insert(mTexJoined.begin(), mTexShip.begin(), mTexShip.end());
	mTexJoined.insert(mTexJoined.end(), mTexGrid.begin(), mTexGrid.end());
	// Validate the texture pointers and increase the ref counter if necessary.
	for (UINT i = 0; i < mTexShip.size(); ++i)
	{
		if (mTexShip[i] != 0) // Make sure the texture is set to point to a valid texture
			// LoadXFile sets it to 0 if one isn't supplied.
			mTexShip[i]->AddRef();
	}
	for (UINT i = 0; i < mTexGrid.size(); ++i)
	{
		if (mTexGrid[i] != 0)// Make sure the texture is set to point to a valid texture
			// LoadXFile sets it to 0 if one isn't supplied.
			mTexGrid[i]->AddRef();
	}


	D3DXMatrixIdentity(&mWorld);

	// Create the white dummy texture.
	HR(D3DXCreateTextureFromFile(gd3dDevice, "whitetex.dds", &mWhiteTex));

	// Add main mesh geometry count.
	mGfxStats->addVertices(mMeshGrid->GetNumVertices());
	mGfxStats->addTriangles(mMeshGrid->GetNumFaces());
	mGfxStats->addVertices(mMeshShip->GetNumVertices());
	mGfxStats->addTriangles(mMeshShip->GetNumFaces());

	buildFX();

	onResetDevice();

	delete [] adjaceny;
	delete [] outAdjaceny;
}

ConcatenateMeshesDemo::~ConcatenateMeshesDemo()
{
	delete mGfxStats;
	ReleaseCOM(mMeshShip);
	ReleaseCOM(mMeshGrid);
	ReleaseCOM(mMeshJoined);
	ReleaseCOM(mFX);

	for(UINT i = 0; i < mTexGrid.size(); ++i)
		ReleaseCOM(mTexGrid[i]);
	for(UINT i = 0; i < mTexShip.size(); ++i)
		ReleaseCOM(mTexShip[i]);
	for(UINT i = 0; i < mTexJoined.size(); ++i)
		ReleaseCOM(mTexJoined[i]);

	ReleaseCOM(mWhiteTex);
	DestroyAllVertexDeclarations();
}

bool ConcatenateMeshesDemo::checkDeviceCaps()
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

void ConcatenateMeshesDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void ConcatenateMeshesDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void ConcatenateMeshesDemo::updateScene(float dt)
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
	if( mCameraRadius < 3.0f )
		mCameraRadius = 3.0f;

	// The camera position/orientation relative to world space can 
	// change every frame based on input, so we need to rebuild the
	// view matrix every frame with the latest changes.
	buildViewMtx();
}

void ConcatenateMeshesDemo::drawScene()
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
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));
	
	HR(mFX->CommitChanges());
	//drawMesh(mMtrlGrid, mMeshGrid, mTexGrid);
	//drawMesh(mMtrlShip, mMeshShip, mTexShip);
	drawMesh(mMtrlJoined, mMeshJoined, mTexJoined);
	//HR(mMeshShip->DrawSubset(j));
	//HR(mMeshGrid->DrawSubset(j));

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void ConcatenateMeshesDemo::drawMesh(const std::vector<Mtrl>& mtrl, ID3DXMesh* mesh, const std::vector<IDirect3DTexture9*>& tex)
{
	// Draw the mesh.
	for(UINT j = 0; j < mtrl.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mtrl[j], sizeof(Mtrl)));

		// If there is a texture, then use.
		if(tex[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, tex[j]));
		}

		// But if not, then set a pure white texture.  When the texture color
		// is multiplied by the color from lighting, it is like multiplying by
		// 1 and won't change the color from lighting.
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

		HR(mFX->CommitChanges());
		HR(mesh->DrawSubset(j));
	}
}

void ConcatenateMeshesDemo::buildFX()
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

void ConcatenateMeshesDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void ConcatenateMeshesDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}