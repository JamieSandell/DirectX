//=============================================================================
// BoundingSphereDemo.cpp.
//
// Demonstrates how to compute the bounding sphere of a mesh with
// D3DXComputeBoundingSphere.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//=============================================================================

#include "BoundingSphereDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	BoundingSphereDemo app(hInstance, "Bounding Sphere Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

BoundingSphereDemo::BoundingSphereDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
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

	D3DXMATRIX S;
	float scaling = 1.0f;
	D3DXMatrixScaling(&S, scaling, scaling, scaling);
	LoadXFile("castle.x", &mMesh, mMtrl, mTex);
	D3DXMatrixIdentity(&mWorld);
	D3DXMatrixTranslation(&mWorld, 1.0f, 1.0f, 2.0f);
	mWorld = S*mWorld;

	// Compute the bounding sphere.
	VertexPNT* v = 0;
	HR(mMesh->LockVertexBuffer(0, (void**)&v));

	HR(D3DXComputeBoundingSphere(&v[0].pos, mMesh->GetNumVertices(), sizeof(VertexPNT), &mBoundingSphere.pos, &mBoundingSphere.radius));
	mBoundingSphere.radius *= scaling;

	HR(mMesh->UnlockVertexBuffer());

	// Build a sphere mesh so that we can render the bounding sphere visually.
	HR(D3DXCreateSphere(gd3dDevice, mBoundingSphere.radius, 50, 50, &mSphere, 0));


	// It is possible that the mesh was not centered about the origin
	// when it was modeled.  But the bounding sphere mesh is built around the
	// origin.  So offset the bounding sphere (mesh) center so that it
	// matches the true mathematical bounding sphere center.
	D3DXVECTOR3 center = mBoundingSphere.pos;
	D3DXMatrixTranslation(&mBoundingSphereOffset, 
		center.x, center.y, center.z);
	// Define the sphere material--make semi-transparent.
	mSphereMtrl.ambient   = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	mSphereMtrl.diffuse   = D3DXCOLOR(0.0f, 0.0f, 1.0f, 0.5f);
	mSphereMtrl.spec      = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mSphereMtrl.specPower = 8.0f;

	D3DXVec3TransformCoord(&mBoundingSphere.pos, &mBoundingSphere.pos, &mWorld);
	D3DXMatrixTranslation(&mBoundingSphereOffset, 
		mBoundingSphere.pos.x, mBoundingSphere.pos.y, mBoundingSphere.pos.z);

	// Create the white dummy texture.
	HR(D3DXCreateTextureFromFile(gd3dDevice, "whitetex.dds", &mWhiteTex));

	// Add main mesh geometry count.
	mGfxStats->addVertices(mMesh->GetNumVertices());
	mGfxStats->addTriangles(mMesh->GetNumFaces());

	// Add bounding box geometry count.
	mGfxStats->addVertices(mSphere->GetNumVertices());
	mGfxStats->addTriangles(mSphere->GetNumFaces());

	buildFX();

	onResetDevice();
}

BoundingSphereDemo::~BoundingSphereDemo()
{
	delete mGfxStats;
	ReleaseCOM(mMesh);
	ReleaseCOM(mFX);

	for(UINT i = 0; i < mTex.size(); ++i)
		ReleaseCOM(mTex[i]);

	ReleaseCOM(mWhiteTex);
	ReleaseCOM(mSphere);
	DestroyAllVertexDeclarations();
}

bool BoundingSphereDemo::checkDeviceCaps()
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

void BoundingSphereDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void BoundingSphereDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void BoundingSphereDemo::updateScene(float dt)
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

void BoundingSphereDemo::drawScene()
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

	// Draw the mesh.
	for(UINT j = 0; j < mMtrl.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mMtrl[j], sizeof(Mtrl)));

		// If there is a texture, then use.
		if(mTex[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, mTex[j]));
		}

		// But if not, then set a pure white texture.  When the texture color
		// is multiplied by the color from lighting, it is like multiplying by
		// 1 and won't change the color from lighting.
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

		HR(mFX->CommitChanges());
		HR(mMesh->DrawSubset(j));
	}

	// Draw the bounding box with alpha blending.
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true));
	HR(gd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	HR(gd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	HR(mFX->SetMatrix(mhWVP, &(mBoundingSphereOffset*mView*mProj)));
	D3DXMatrixInverse(&worldInvTrans, 0, &mBoundingSphereOffset);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mBoundingSphereOffset));
	HR(mFX->SetValue(mhMtrl, &mSphereMtrl, sizeof(Mtrl)));
	HR(mFX->SetTexture(mhTex, mWhiteTex));
	HR(mFX->CommitChanges());
	HR(mSphere->DrawSubset(0));
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false));

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void BoundingSphereDemo::buildFX()
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

void BoundingSphereDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void BoundingSphereDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}