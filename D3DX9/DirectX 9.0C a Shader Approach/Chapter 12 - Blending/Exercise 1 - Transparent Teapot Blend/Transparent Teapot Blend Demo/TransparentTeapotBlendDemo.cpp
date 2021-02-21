//=============================================================================
// TransparentTeapotBlendDemo.cpp
//
// Demonstrates material alpha blending and transparency.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//			 Press NUMPAD_MINUS/NUMPAS_PLUS to change the source blending mode.
//			 Press NUMPAD_STAR/NUMPAD_9 to change the destination blending mode.
//			 Press MINUS/EQUALS to change the alpha value of the teapot.
//=============================================================================
#define stringify( name ) # name

#include <tchar.h>
#include "TransparentTeapotBlendDemo.h"

using std::vector;

// Used for easy printing of the current blend modes
const char* enBlendValueNames[] = 
{
	stringify( D3DBLEND_ZERO ),
	stringify( D3DBLEND_ONE ),
	stringify( D3DBLEND_SRCCOLOR ),
	stringify( D3DBLEND_INVSRCCOLOR ),
	stringify( D3DBLEND_SRCALPHA ),
	stringify( D3DBLEND_INVSRCALPHA ),
	stringify( D3DBLEND_DESTALPHA ),
	stringify( D3DBLEND_INVDESTALPHA ),
	stringify( D3DBLEND_DESTCOLOR ),
	stringify( D3DBLEND_INVDESTCOLOR ),
	stringify( D3DBLEND_SRCALPHASAT ),
	stringify( D3DBLEND_BLENDFACTOR ),
	stringify( D3DBLEND_INVBLENDFACTOR ),
	stringify( D3DBLEND_BOTHINVSRCALPHA )
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	TransparentTeapotBlendDemo app(hInstance, "Transparent Teapot Blend Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

TransparentTeapotBlendDemo::TransparentTeapotBlendDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	initFont();

	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	//Insert the blend values that are valid for both source and destination
	mSrcBlend.push_back(D3DBLEND_ZERO);
	mSrcBlend.push_back(D3DBLEND_ONE);
	mSrcBlend.push_back(D3DBLEND_SRCCOLOR);
	mSrcBlend.push_back(D3DBLEND_INVSRCCOLOR);
	mSrcBlend.push_back(D3DBLEND_SRCALPHA);
	mSrcBlend.push_back(D3DBLEND_INVSRCALPHA);
	mSrcBlend.push_back(D3DBLEND_DESTALPHA);
	mSrcBlend.push_back(D3DBLEND_INVDESTALPHA);
	mSrcBlend.push_back(D3DBLEND_DESTCOLOR);
	mSrcBlend.push_back(D3DBLEND_SRCALPHASAT);
	mSrcBlend.push_back(D3DBLEND_SRCALPHASAT);
	mSrcBlend.push_back(D3DBLEND_BLENDFACTOR);
	mSrcBlend.push_back(D3DBLEND_INVBLENDFACTOR);

	//Copy the valid ones to the destination
	mDestBlend.insert(mDestBlend.begin(), mSrcBlend.begin(), mSrcBlend.end());

	//Add the remaining valid one to source, but not to destination
	mSrcBlend.push_back(D3DBLEND_BOTHINVSRCALPHA);

	mGfxStats = new GfxStats();

	mintSrcBlend = 4;
	mintDestBlend = 5;

	mCameraRadius    = 6.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight    = 3.0f;

	mLightVecW     = D3DXVECTOR3(0.0, 0.0f, -1.0f);
	mDiffuseLight  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientLight  = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	mSpecularLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	mCrateMtrl.ambient   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mCrateMtrl.diffuse   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mCrateMtrl.spec      = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);
	mCrateMtrl.specPower = 8.0f;

	mTeapotMtrl.ambient   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mTeapotMtrl.diffuse   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
	mTeapotMtrl.spec      = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mTeapotMtrl.specPower = 8.0f;

	// Set the crate back a bit.
	D3DXMatrixTranslation(&mCrateWorld, 0.0f, 0.0f, 2.0f);
	D3DXMatrixIdentity(&mTeapotWorld);

	HR(D3DXCreateTextureFromFile(gd3dDevice, "crate.jpg", &mCrateTex));
	HR(D3DXCreateTextureFromFile(gd3dDevice, "brick0.dds", &mTeapotTex));

	HR(D3DXCreateTeapot(gd3dDevice, &mTeapot, 0));
	// Generate texture coordinates for the teapot.
	genSphericalTexCoords();

	mGfxStats->addVertices(24);
	mGfxStats->addTriangles(12);
	mGfxStats->addVertices(mTeapot->GetNumVertices());
	mGfxStats->addTriangles(mTeapot->GetNumFaces());

	buildBoxGeometry();
	buildFX();

	onResetDevice();
}

TransparentTeapotBlendDemo::~TransparentTeapotBlendDemo()
{
	delete mGfxStats;
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mTeapot);
	ReleaseCOM(mCrateTex);
	ReleaseCOM(mTeapotTex);
	ReleaseCOM(mFX);
	ReleaseCOM(mFont);

	DestroyAllVertexDeclarations();
}

bool TransparentTeapotBlendDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gd3dDevice->GetDeviceCaps(&caps));

	// Check for vertex shader version 2.0 support.
	if( caps.VertexShaderVersion < D3DVS_VERSION(2, 0) )
		return false;

	// Check for pixel shader version 2.0 support.
	if( caps.PixelShaderVersion < D3DPS_VERSION(2, 0) )
		return false;

	// Check to see if the hardware is capable of the other blend factors, if it is, add them to the lists
	if (!(caps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR) && !(caps.DestBlendCaps & D3DPBLENDCAPS_BLENDFACTOR))
		return false;

	return true;
}

void TransparentTeapotBlendDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFont->OnLostDevice());
	HR(mFX->OnLostDevice());
}

void TransparentTeapotBlendDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());
	HR(mFont->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void TransparentTeapotBlendDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if( gDInput->keyDown(DIK_W) )	 
		mCameraHeight   += 25.0f * dt;
	if( gDInput->keyDown(DIK_S) )	 
		mCameraHeight   -= 25.0f * dt;
	if (gDInput->keyDown(DIK_EQUALS))//Increase teapot alpha value
	{
		mTeapotMtrl.diffuse.a += dt;
		if (mTeapotMtrl.diffuse.a > 1.0f)
			mTeapotMtrl.diffuse.a = 1.0f;
	}
	if (gDInput->keyDown(DIK_MINUS))//Decrease teapot alpha value
	{
		mTeapotMtrl.diffuse.a -= dt;
		if (mTeapotMtrl.diffuse.a < 0.0f)
			mTeapotMtrl.diffuse.a = 0.0f;
	}
	if (gDInput->keyPressed(DIK_NUMPADPLUS))//Change source blend factor
	{
		mintSrcBlend++;
		if (mintSrcBlend>=mSrcBlend.size())
			mintSrcBlend = 0;
	}
	if (gDInput->keyPressed(DIK_NUMPADMINUS))//Change source blend factor
	{
		mintSrcBlend--;
		if (mintSrcBlend<0)
			mintSrcBlend = mSrcBlend.size()-1;
	}
	if (gDInput->keyPressed(DIK_NUMPAD9))//Change destination blend factor
	{
		mintDestBlend++;
		if (mintDestBlend>=mDestBlend.size())
			mintDestBlend = 0;
	}
	if (gDInput->keyPressed(DIK_NUMPADSTAR))//Change destination blend factor
	{
		mintDestBlend--;
		if (mintDestBlend<0)
			mintDestBlend = mDestBlend.size()-1;
	}

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

void TransparentTeapotBlendDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	HR(mFX->SetTechnique(mhTech));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularLight, &mSpecularLight, sizeof(D3DXCOLOR)));

	//drawCrate();
	drawTeapot();

	mGfxStats->display();
	displayBlendStateAndControls();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void TransparentTeapotBlendDemo::buildBoxGeometry()
{
	// Create the vertex buffer.
	HR(gd3dDevice->CreateVertexBuffer(24 * sizeof(VertexPNT), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED,	&mBoxVB, 0));

	// Write box vertices to the vertex buffer.
	VertexPNT* v = 0;
	HR(mBoxVB->Lock(0, 0, (void**)&v, 0));

	// Fill in the front face vertex data.
	v[0] = VertexPNT(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[1] = VertexPNT(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[2] = VertexPNT( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[3] = VertexPNT( 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = VertexPNT(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[5] = VertexPNT( 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[6] = VertexPNT( 1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[7] = VertexPNT(-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8]  = VertexPNT(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[9]  = VertexPNT(-1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[10] = VertexPNT( 1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[11] = VertexPNT( 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = VertexPNT(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[13] = VertexPNT( 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = VertexPNT( 1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[15] = VertexPNT(-1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = VertexPNT(-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[17] = VertexPNT(-1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[18] = VertexPNT(-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[19] = VertexPNT(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = VertexPNT( 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[21] = VertexPNT( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[22] = VertexPNT( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[23] = VertexPNT( 1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	HR(mBoxVB->Unlock());


	// Create the index buffer.
	HR(gd3dDevice->CreateIndexBuffer(36 * sizeof(WORD),	D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,	D3DPOOL_MANAGED, &mBoxIB, 0));

	// Write box indices to the index buffer.
	WORD* i = 0;
	HR(mBoxIB->Lock(0, 0, (void**)&i, 0));

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7]  = 5; i[8]  = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] =  9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	HR(mBoxIB->Unlock());
}

void TransparentTeapotBlendDemo::buildFX()
{
	// Create the FX from a .fx file.
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, "DirLightTex.fx", 
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech          = mFX->GetTechniqueByName("DirLightTexTech");
	mhWVP           = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhLightVecW     = mFX->GetParameterByName(0, "gLightVecW");
	mhDiffuseMtrl   = mFX->GetParameterByName(0, "gDiffuseMtrl");
	mhDiffuseLight  = mFX->GetParameterByName(0, "gDiffuseLight");
	mhAmbientMtrl   = mFX->GetParameterByName(0, "gAmbientMtrl");
	mhAmbientLight  = mFX->GetParameterByName(0, "gAmbientLight");
	mhSpecularMtrl  = mFX->GetParameterByName(0, "gSpecularMtrl");
	mhSpecularLight = mFX->GetParameterByName(0, "gSpecularLight");
	mhSpecularPower = mFX->GetParameterByName(0, "gSpecularPower");
	mhEyePos        = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld         = mFX->GetParameterByName(0, "gWorld");
	mhTex           = mFX->GetParameterByName(0, "gTex");
}

void TransparentTeapotBlendDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void TransparentTeapotBlendDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void TransparentTeapotBlendDemo::drawCrate()
{
	HR(mFX->SetValue(mhAmbientMtrl, &mCrateMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mCrateMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mCrateMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mCrateMtrl.specPower));

	HR(mFX->SetMatrix(mhWVP, &(mCrateWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mCrateWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mCrateWorld));
	HR(mFX->SetTexture(mhTex, mCrateTex));

	HR(gd3dDevice->SetVertexDeclaration(VertexPNT::Decl));
	HR(gd3dDevice->SetStreamSource(0, mBoxVB, 0, sizeof(VertexPNT)));
	HR(gd3dDevice->SetIndices(mBoxIB));

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12));
		HR(mFX->EndPass());
	}
	HR(mFX->End());
}

void TransparentTeapotBlendDemo::drawTeapot()
{
	// Cylindrically interpolate texture coordinates.
	HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAPCOORD_0));

	// Enable alpha blending.
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true));
	HR(gd3dDevice->SetRenderState(D3DRS_SRCBLEND, mSrcBlend[mintSrcBlend]));
	HR(gd3dDevice->SetRenderState(D3DRS_DESTBLEND, mDestBlend[mintDestBlend]));

	HR(mFX->SetValue(mhAmbientMtrl, &mTeapotMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mTeapotMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mTeapotMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mTeapotMtrl.specPower));

	HR(mFX->SetMatrix(mhWVP, &(mTeapotWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mTeapotWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mTeapotWorld));
	HR(mFX->SetTexture(mhTex, mTeapotTex));

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for(UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(mTeapot->DrawSubset(0));
		HR(mFX->EndPass());
	}
	HR(mFX->End());

	// Disable wrap.
	HR(gd3dDevice->SetRenderState(D3DRS_WRAP0, 0));

	// Disable alpha blending.
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false));
}

void TransparentTeapotBlendDemo::genSphericalTexCoords()
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

void TransparentTeapotBlendDemo::displayBlendStateAndControls()
{
	// Make static so memory is not allocated every frame.
	static char buffer[256];
	std::string controls = "Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to alter the height of the camera.\nUse 'A' and 'D' keys to translate the teapot on the x-axis.\nUse 'Numpad 7' to move the light into the screen.\nUse 'Numpad 9' to move the light out of the screen.\nUse 'Numpad 4' to move the light left.\nUse 'Numpad 6' to move the light right.\nUse 'Numpad 8' to move the light up.\nUse 'Numpad 2' to move the light down.\nUse 'Esc/F' to toggle window/full-screen.";

	UINT h = md3dPP.BackBufferHeight;
	RECT R = {5, h-(h/5), 0, 0};
	sprintf(buffer, "%s\n",
		controls.c_str());
	HR(mFont->DrawTextA(0, buffer, -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0,0,0)));
}

void TransparentTeapotBlendDemo::initFont()
{
	D3DXFONT_DESC fontDesc;
	fontDesc.Height          = 18;
	fontDesc.Width           = 0;
	fontDesc.Weight          = 0;
	fontDesc.MipLevels       = 1;
	fontDesc.Italic          = false;
	fontDesc.CharSet         = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality         = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy(fontDesc.FaceName, _T("Times New Roman"));

	HR(D3DXCreateFontIndirect(gd3dDevice, &fontDesc, &mFont));
}