#include "SphericalMappingDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	SphericalMappingDemo app(hInstance, "Spherical Mapping Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

SphericalMappingDemo::SphericalMappingDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius    = 5.0f;
	mCameraRotationY = 5.0f;
	mCameraHeight    = 2.0f;

	mAmbientLight   = WHITE;
	mDiffuseLight   = WHITE;
	mSpecLight      = WHITE;
	mLightVecW      = D3DXVECTOR3(0.0, 0.0f, -1.0f);

	D3DXMatrixIdentity(&mWorld);

	mTeapotMtrl   = Mtrl(WHITE*0.4f, WHITE, WHITE*0.8f, 8.0f);
	HR(D3DXCreateTeapot(gd3dDevice, &mTeapot, 0));

	genSphericalTexCoords();

	HR(D3DXCreateTextureFromFile(gd3dDevice, "stone2.dds", &mTeapotTex));

	buildFX();

	//Calculate the number of vertices and faces for use with GfxStats
	int numTeapotVerts = mTeapot->GetNumVertices();
	int numTeapotTris  = mTeapot->GetNumFaces();
	mGfxStats->addVertices(numTeapotVerts);
	mGfxStats->addTriangles(numTeapotTris);

	onResetDevice();
}

SphericalMappingDemo::~SphericalMappingDemo()
{
	delete mGfxStats;
	ReleaseCOM(mFX);
	ReleaseCOM(mTeapot);
	ReleaseCOM(mTeapotTex);

	DestroyAllVertexDeclarations();
}

bool SphericalMappingDemo::checkDeviceCaps()
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

void SphericalMappingDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void SphericalMappingDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void SphericalMappingDemo::updateScene(float dt)
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

void SphericalMappingDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	// Setup the rendering FX
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecLight, &mSpecLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));

	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mWorld));

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		drawTeapot();

		HR(mFX->EndPass());
	}
	HR(mFX->End());


	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void SphericalMappingDemo::buildFX()
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
}

void SphericalMappingDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	//D3DXVECTOR3 pos(0, 0, 10);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void SphericalMappingDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void SphericalMappingDemo::drawTeapot()
{
	HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAP_U));

	HR(mFX->SetValue(mhAmbientMtrl, &mTeapotMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mTeapotMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecMtrl, &mTeapotMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecPower, mTeapotMtrl.specPower));

	HR(mFX->SetTexture(mhTex, mTeapotTex));

	HR(mFX->CommitChanges());
	HR(mTeapot->DrawSubset(0));
	// Disable.
	HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, 0));
}

void SphericalMappingDemo::genSphericalTexCoords()
{
	// D3DXCreate* functions generate vertices with position 
	// and normal data.  But for texturing, we also need
	// tex-coords.  So clone the mesh to change the vertex
	// format to a format with tex-coords.

	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::Decl->GetDeclaration(elements, &numElements);

	ID3DXMesh* temp = 0;
	HR(mTeapot->CloneMesh(D3DXMESH_SYSTEMMEM, 
		elements, gd3dDevice, &temp));

	ReleaseCOM(mTeapot);

	// Now generate texture coordinates for each vertex.
	VertexPNT* vertices = 0;
	HR(temp->LockVertexBuffer(0, (void**)&vertices));

	for(UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		// Convert to spherical coordinates.
		D3DXVECTOR3 p = vertices[i].pos;

		float theta = atan2f(p.z, p.x);
		float phi   = acosf(p.y / sqrtf(p.x*p.x+p.y*p.y+p.z*p.z));

		// Phi and theta give the texture coordinates, but are not in 
		// the range [0, 1], so scale them into that range.

		float u = theta / (2.0f*D3DX_PI);
		float v = phi   / D3DX_PI;

		// Save texture coordinates.

		vertices[i].tex0.x = u;
		vertices[i].tex0.y = v;
	}
	HR(temp->UnlockVertexBuffer());

	// Clone back to a hardware mesh.
	HR(temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY,
		elements, gd3dDevice, &mTeapot));

	ReleaseCOM(temp);
}