//=============================================================================
// BasicTerrainDemo.cpp.
//
// Demonstrates using a heightmap to alter a grid's vertices to produce terrain.
// A model walks across the terrain. N.B. The model does not traverse
// hills/slopes "correctly".
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//=============================================================================

#include "BasicTerrainDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Construct camera before application, since the application uses the camera.
	Camera camera;
	gCamera = &camera;

	BasicTerrainDemo app(hInstance, "Basic Terrain Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

BasicTerrainDemo::BasicTerrainDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// World space units are meters.  So (256*10.0f)x(256*10.0f) is (2.56)^2 square
	// kilometers.
	mTerrain = new Terrain(257, 257, 10.0f, 10.0f, 
		"heightmap17_257.raw",  
		"grass.dds",
		"dirt.dds",
		"stone.dds",
		"blend_hm17.dds",
		3.0f, 0.0f);

	mTerrain->setDirToSunW(D3DXVECTOR3(0.0f, 1.0f, 0.0f));

	mCameraRadius    = 80.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 40.0f;

	mWhiteMtrl.ambient = WHITE*0.9f;
	mWhiteMtrl.diffuse = WHITE*0.6f;
	mWhiteMtrl.spec    = WHITE*0.6f;
	mWhiteMtrl.specPower = 48.0f;

	mLight.dirW    = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec    = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	// Load the skinned mesh and its texture.
	mSkinnedMesh = new SkinnedMesh("tiny.x");
	HR(D3DXCreateTextureFromFile(gd3dDevice, "Tiny_skin.bmp", &mTex));
	// Setup the tracks
	mSkinnedMesh->setAnimationSet(0);
	mSkinnedMesh->enableTrack(0, true);
	// Scale the mesh down.
	mSkinnedMeshPos = D3DXVECTOR3(-10.0f, 0.0f, 40.0f);
	D3DXMatrixTranslation(&mWorldSkinnedMesh, mSkinnedMeshPos.x, mSkinnedMeshPos.y, mSkinnedMeshPos.z);
	D3DXMatrixScaling(&mWorldSkinnedMesh, 1.0f, 1.0f, 1.0f);

	D3DXMatrixIdentity(&mWorld);

	mGfxStats->addVertices(mTerrain->getNumVertices());
	mGfxStats->addTriangles(mTerrain->getNumTriangles());
	mGfxStats->addVertices(mSkinnedMesh->numVertices());
	mGfxStats->addTriangles(mSkinnedMesh->numTriangles());

	buildFX(mFX, "Terrain.fx");
	//setFXTerrainParams();
	buildFX(mFXVBlend2, "vblend2.fx");
	setFXSkinnedMeshParams();

	onResetDevice();
}

BasicTerrainDemo::~BasicTerrainDemo()
{
	delete mGfxStats;
	delete mSkinnedMesh;
	delete mTerrain;

	ReleaseCOM(mTex);
	ReleaseCOM(mFXVBlend2);

	DestroyAllVertexDeclarations();
}

bool BasicTerrainDemo::checkDeviceCaps()
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

void BasicTerrainDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	mTerrain->onLostDevice();
	HR(mFX->OnLostDevice());
}

void BasicTerrainDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());
	mTerrain->onResetDevice();

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	gCamera->setLens(D3DX_PI * 0.25f, w/h, 0.01f, 5000.0f);
}

void BasicTerrainDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Animate the skinned mesh.
	mSkinnedMesh->update(dt);

	//mSkinnedMeshPos.y = mTerrain->getHeight(mSkinnedMeshPos.x, mSkinnedMeshPos.z);
	// Offset the height so it walks on the terrain properly
	D3DXVECTOR3 newPos = mSkinnedMeshPos;
	newPos.z -= 20.0f * dt;

	// New position might not be on terrain, so project the
	// point onto the terrain.
	newPos.y = mTerrain->getHeight(newPos.x, newPos.z) + 2.5f;

	// Now the difference of the new position and old (current) 
	// position approximates a tangent vector on the terrain.
	D3DXVECTOR3 tangent = newPos - mSkinnedMeshPos;
	D3DXVec3Normalize(&tangent, &tangent);

	// Now move camera along tangent vector.
	mSkinnedMeshPos += tangent*20.0f*dt;

	// After update, there may be errors in the camera height since our
	// tangent is only an approximation.  So force camera to correct height,
	// and offset by the specified amount so that camera does not sit
	// exactly on terrain, but instead, slightly above it.
	mSkinnedMeshPos.y = mTerrain->getHeight(mSkinnedMeshPos.x, mSkinnedMeshPos.z) + 2.5f;

	D3DXVECTOR3 target(mSkinnedMeshPos.x, 0.0f, mSkinnedMeshPos.z+25);
	D3DXVECTOR3 pos(mSkinnedMeshPos.x, -10.0f, mSkinnedMeshPos.z-25.0f);

	gCamera->lookAt(pos, target, D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	gCamera->update(dt, mTerrain, 2.5f);

	////reset the position if it goes too far
	if (mSkinnedMeshPos.z <= -200.0f)
		mSkinnedMeshPos.z = 40.0f;
}

void BasicTerrainDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	mTerrain->draw();

	mGfxStats->display();

	// Skinned mesh scene
	// Set FX Parameters.  In particular, for this demo note that we set the 
	// final transformation matrix array for vertex blending.

	setFXSkinnedMeshParams();
	HR(mFXVBlend2->SetTechnique(mhTech));
	HR(mFXVBlend2->CommitChanges());
	D3DXMATRIX T, R, S, W, WIT;
	D3DXVECTOR3 d(0.0f, 1.0f, 0.0f);

	D3DXMatrixScaling(&S, 0.01f, 0.01f, 0.01f);
	D3DXMatrixTranslation(&T, mSkinnedMeshPos.x, mSkinnedMeshPos.y, mSkinnedMeshPos.z);

	mWorldSkinnedMesh = S*T;

	HR(mFXVBlend2->SetMatrixArray(mhFinalXForms, mSkinnedMesh->getFinalXFormArray(), mSkinnedMesh->numBones()));
	HR(mFXVBlend2->SetValue(mhLight, &mLight, sizeof(DirLight)));
	HR(mFXVBlend2->SetMatrix(mhWVP, &(mWorldSkinnedMesh*gCamera->viewProj())));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mWorldSkinnedMesh);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFXVBlend2->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFXVBlend2->SetMatrix(mhWorld, &mWorldSkinnedMesh));
	HR(mFXVBlend2->SetValue(mhMtrl, &mWhiteMtrl, sizeof(Mtrl)));
	HR(mFXVBlend2->SetTexture(mhTex, mTex));
	HR(mFXVBlend2->CommitChanges());

	UINT numPasses = 0;
	HR(mFXVBlend2->Begin(&numPasses, 0));
	HR(mFXVBlend2->BeginPass(0));

	mSkinnedMesh->draw();

	HR(mFXVBlend2->EndPass());
	HR(mFXVBlend2->End());

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void BasicTerrainDemo::buildGridGeometry()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	int vertRows = 129;
	int vertCols = 129;
	float dx = 1.0f;
	float dz = 1.0f;

	GenTriGrid(vertRows, vertCols, dx, dz, 
		D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	int numVerts = vertRows*vertCols;
	int numTris  = (vertRows-1)*(vertCols-1)*2;

	// Create the mesh.
	D3DVERTEXELEMENT9 elems[MAX_FVF_DECL_SIZE];
	UINT numElems = 0;
	HR(VertexPNT::Decl->GetDeclaration(elems, &numElems));
	HR(D3DXCreateMesh(numTris, numVerts, 
		D3DXMESH_MANAGED, elems, gd3dDevice, &mTerrainMesh));

	// Write the vertices.
	VertexPNT* v = 0;
	HR(mTerrainMesh->LockVertexBuffer(0,(void**)&v));

	// width/depth
	float w = (vertCols-1) * dx; 
	float d = (vertRows-1) * dz;
	for(int i = 0; i < vertRows; ++i)
	{
		for(int j = 0; j < vertCols; ++j)
		{
			DWORD index = i * vertCols + j;
			v[index].pos    = verts[index];
			v[index].pos.y  = mHeightmap(i, j);
			v[index].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			v[index].tex0.x = (v[index].pos.x + (0.5f*w)) / w;
			v[index].tex0.y = (v[index].pos.z - (0.5f*d)) / -d;
		}
	}

	HR(mTerrainMesh->UnlockVertexBuffer());


	// Write the indices and attribute buffer.
	WORD* k = 0;
	HR(mTerrainMesh->LockIndexBuffer(0, (void**)&k));
	DWORD* attBuffer = 0;
	HR(mTerrainMesh->LockAttributeBuffer(0, &attBuffer));

	// Compute the indices for each triangle.
	for(int i = 0; i < numTris; ++i)
	{
		k[i*3+0] = (WORD)indices[i*3+0];
		k[i*3+1] = (WORD)indices[i*3+1];
		k[i*3+2] = (WORD)indices[i*3+2];

		attBuffer[i] = 0; // Always subset 0
	}

	HR(mTerrainMesh->UnlockIndexBuffer());
	HR(mTerrainMesh->UnlockAttributeBuffer());

	// Generate normals and then optimize the mesh.
	HR(D3DXComputeNormals(mTerrainMesh, 0));

	DWORD* adj = new DWORD[mTerrainMesh->GetNumFaces()*3];
	HR(mTerrainMesh->GenerateAdjacency(EPSILON, adj));
	HR(mTerrainMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_ATTRSORT,
		adj, 0, 0, 0));
	delete[] adj;
}

void BasicTerrainDemo::setFXTerrainParams()
{
	mhTech      = mFX->GetTechniqueByName("TerrainTech");
	mhViewProj  = mFX->GetParameterByName(0, "gViewProj");
	mhDirToSunW = mFX->GetParameterByName(0, "gDirToSunW");
	mhTex0      = mFX->GetParameterByName(0, "gTex0");
	mhTex1      = mFX->GetParameterByName(0, "gTex1");
	mhTex2      = mFX->GetParameterByName(0, "gTex2");
	mhBlendMap  = mFX->GetParameterByName(0, "gBlendMap");

	HR(mFX->SetTexture(mhTex0, mTex0));
	HR(mFX->SetTexture(mhTex1, mTex1));
	HR(mFX->SetTexture(mhTex2, mTex2));
	HR(mFX->SetTexture(mhBlendMap, mBlendMap));

	D3DXVECTOR3 d(0.0f, 1.0f, 0.0f);
	HR(mFX->SetValue(mhDirToSunW, &d, sizeof(D3DXVECTOR3)));
}

void BasicTerrainDemo::setFXSkinnedMeshParams()
{
	// Obtain handles.
			//VBlend2Tech
	mhTech            = mFXVBlend2->GetTechniqueByName("VBlend2Tech");
	mhWVP             = mFXVBlend2->GetParameterByName(0, "gWVP");
	mhWorldInvTrans   = mFXVBlend2->GetParameterByName(0, "gWorldInvTrans");
	mhFinalXForms     = mFXVBlend2->GetParameterByName(0, "gFinalXForms");
	mhMtrl            = mFXVBlend2->GetParameterByName(0, "gMtrl");
	mhLight           = mFXVBlend2->GetParameterByName(0, "gLight");
	mhEyePos          = mFXVBlend2->GetParameterByName(0, "gEyePosW");
	mhWorld           = mFXVBlend2->GetParameterByName(0, "gWorld");
	mhTex             = mFXVBlend2->GetParameterByName(0, "gTex");
}

void BasicTerrainDemo::buildFX(ID3DXEffect* &fx, std::string file)
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, file.c_str(),
		0, 0, D3DXSHADER_DEBUG, 0, &fx, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);
	ReleaseCOM(errors);
}

void BasicTerrainDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void BasicTerrainDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}