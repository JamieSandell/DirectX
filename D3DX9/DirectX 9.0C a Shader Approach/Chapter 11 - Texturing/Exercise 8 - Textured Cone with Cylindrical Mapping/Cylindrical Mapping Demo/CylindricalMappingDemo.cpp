#include "CylindricalMappingDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	CylindricalMappingDemo app(hInstance, "Cylindrical Mapping Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

CylindricalMappingDemo::CylindricalMappingDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
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

	mConeMtrl   = Mtrl(WHITE*0.4f, WHITE, WHITE*0.2f, 10.0f);
	HR(D3DXCreateCylinder(gd3dDevice, 0.0f, 1.0f, 3.0f, 30, 30, &mCone, 0));

	genCylTexCoords(Z_AXIS);

	HR(D3DXCreateTextureFromFile(gd3dDevice, "stone2.dds", &mConeTex));

	buildFX();

	//Calculate the number of vertices and faces for use with GfxStats
	int numTeapotVerts = mCone->GetNumVertices();
	int numTeapotTris  = mCone->GetNumFaces();
	mGfxStats->addVertices(numTeapotVerts);
	mGfxStats->addTriangles(numTeapotTris);

	onResetDevice();
}

CylindricalMappingDemo::~CylindricalMappingDemo()
{
	delete mGfxStats;
	ReleaseCOM(mFX);
	ReleaseCOM(mCone);
	ReleaseCOM(mConeTex);

	DestroyAllVertexDeclarations();
}

bool CylindricalMappingDemo::checkDeviceCaps()
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

void CylindricalMappingDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
}

void CylindricalMappingDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void CylindricalMappingDemo::updateScene(float dt)
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

void CylindricalMappingDemo::drawScene()
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

		drawCone();

		HR(mFX->EndPass());
	}
	HR(mFX->End());


	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void CylindricalMappingDemo::buildFX()
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

void CylindricalMappingDemo::buildViewMtx()
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

void CylindricalMappingDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void CylindricalMappingDemo::drawCone()
{
	HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAP_U));

	HR(mFX->SetValue(mhAmbientMtrl, &mConeMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mConeMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecMtrl, &mConeMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecPower, mConeMtrl.specPower));

	HR(mFX->SetTexture(mhTex, mConeTex));

	D3DXMATRIX T, R, W, WIT;
	D3DXMatrixRotationX(&R, D3DX_PI*0.5f);

	//D3DXMatrixTranslation(&T, -10.0f, 3.0f, 0.0f);
	W = R;//*T;
	D3DXMatrixInverse(&WIT, 0, &W);
	D3DXMatrixTranspose(&WIT, &WIT);

	HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
	HR(mFX->SetMatrix(mhWorld, &W));
	HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));

	HR(mFX->CommitChanges());
	HR(mCone->DrawSubset(0));
	// Disable.
	HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, 0));
}

void CylindricalMappingDemo::genCylTexCoords(AXIS axis)
{
	// D3DXCreate* functions generate vertices with position 
	// and normal data.  But for texturing, we also need
	// tex-coords.  So clone the mesh to change the vertex
	// format to a format with tex-coords.

	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::Decl->GetDeclaration(elements, &numElements);

	ID3DXMesh* temp = 0;
	HR(mCone->CloneMesh(D3DXMESH_SYSTEMMEM, 
		elements, gd3dDevice, &temp));

	ReleaseCOM(mCone);

	// Now generate texture coordinates for each vertex.
	VertexPNT* vertices = 0;
	HR(temp->LockVertexBuffer(0, (void**)&vertices));

	// We need to get the height of the cylinder we are projecting the
	// vertices onto.  That height depends on which axis the client has
	// specified that the cylinder lies on.  The height is determined by 
	// finding the height of the bounding cylinder on the specified axis.

	D3DXVECTOR3 maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	D3DXVECTOR3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);

	for(UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		D3DXVec3Maximize(&maxPoint, &maxPoint, &vertices[i].pos);
		D3DXVec3Minimize(&minPoint, &minPoint, &vertices[i].pos);
	}

	float a = 0.0f;
	float b = 0.0f;
	float h = 0.0f;
	switch( axis )
	{
	case X_AXIS:
		a = minPoint.x;
		b = maxPoint.x;
		h = b-a;
		break;
	case Y_AXIS:
		a = minPoint.y;
		b = maxPoint.y;
		h = b-a;
		break;
	case Z_AXIS:
		a = minPoint.z;
		b = maxPoint.z;
		h = b-a;
		break;
	}


	// Iterate over each vertex and compute its texture coordinate.

	for(UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		// Get the coordinates along the axes orthogonal to the
		// axis the cylinder is aligned with.

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		switch( axis )
		{
		case X_AXIS:
			x = vertices[i].pos.y;
			z = vertices[i].pos.z;
			y = vertices[i].pos.x;
			break;
		case Y_AXIS:
			x = vertices[i].pos.x;
			z = vertices[i].pos.z;
			y = vertices[i].pos.y;
			break;
		case Z_AXIS:
			x = vertices[i].pos.x;
			z = vertices[i].pos.y;
			y = vertices[i].pos.z;
			break;
		}

		// Convert to cylindrical coordinates.

		float theta = atan2f(z, x);
		float y2    = y - b; // Transform [a, b]-->[-h, 0]

		// Transform theta from [0, 2*pi] to [0, 1] range and
		// transform y2 from [-h, 0] to [0, 1].

		float u = theta / (2.0f*D3DX_PI);
		float v = y2 / -h; 

		// Save texture coordinates.

		vertices[i].tex0.x = u;
		vertices[i].tex0.y = v;
	}

	HR(temp->UnlockVertexBuffer());

	// Clone back to a hardware mesh.
	HR(temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY,
		elements, gd3dDevice, &mCone));

	ReleaseCOM(temp);
}