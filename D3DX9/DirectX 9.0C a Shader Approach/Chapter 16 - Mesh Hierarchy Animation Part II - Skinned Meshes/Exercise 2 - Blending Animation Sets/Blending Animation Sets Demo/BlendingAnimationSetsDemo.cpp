//=============================================================================
// BlendingAnimationSetsDemo.cpp.
//
// Demonstrates how to blend different animation sets together.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//			 Use '1', '2', '3' to switch between the different blended
//			 animation sets.
//=============================================================================

#include <tchar.h>
#include "BlendingAnimationSetsDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	BlendingAnimationSetsDemo app(hInstance, "Blending Animation Sets Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

BlendingAnimationSetsDemo::BlendingAnimationSetsDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius    = 8.0f;
	mCameraRotationY = 1.3 * D3DX_PI;
	mCameraHeight    = 3.0f;

	mLight.dirW    = D3DXVECTOR3(1.0f, 1.0f, 2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec    = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	mWhiteMtrl.ambient = WHITE*0.9f;
	mWhiteMtrl.diffuse = WHITE*0.6f;
	mWhiteMtrl.spec    = WHITE*0.6f;
	mWhiteMtrl.specPower = 48.0f;

	// Load the skinned mesh and its texture.
	mSkinnedMesh = new SkinnedMesh("tiny_4anim.x");
	HR(D3DXCreateTextureFromFile(gd3dDevice, "Tiny_skin.bmp", &mTex));
	// Setup the tracks
	mSkinnedMesh->setTrackParams(0, 1.0f, 0.8f, D3DXPRIORITY_HIGH); //Wave
	mSkinnedMesh->setTrackParams(1, 1.0f, 0.2f, D3DXPRIORITY_HIGH); //Run
	mSkinnedMesh->setTrackAnimationSet(0, 0);
	mSkinnedMesh->setTrackAnimationSet(1, 1);
	mSkinnedMesh->enableTrack(0, true);
	mSkinnedMesh->enableTrack(1, true);

	// Scale the mesh down.
	D3DXMatrixScaling(&mWorld, 0.01f, 0.01f, 0.01f);

	mGfxStats->addVertices(mSkinnedMesh->numVertices());
	mGfxStats->addTriangles(mSkinnedMesh->numTriangles());

	buildFX();
	initFont();

	onResetDevice();
}

BlendingAnimationSetsDemo::~BlendingAnimationSetsDemo()
{
	delete mGfxStats;
	delete mSkinnedMesh;

	ReleaseCOM(mFX);
	ReleaseCOM(mTex);
	ReleaseCOM(mFont);

	DestroyAllVertexDeclarations();
}

bool BlendingAnimationSetsDemo::checkDeviceCaps()
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

void BlendingAnimationSetsDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
	HR(mFont->OnLostDevice());
}

void BlendingAnimationSetsDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());
	HR(mFont->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void BlendingAnimationSetsDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if( gDInput->keyDown(DIK_W) )	 
		mCameraHeight   += 25.0f * dt;
	if( gDInput->keyDown(DIK_S) )	 
		mCameraHeight   -= 25.0f * dt;
	if( gDInput->keyDown(DIK_1))
	{
		// Run-Wave Blending
		mSkinnedMesh->setTrackParams(0, 1.0f, 0.8f, D3DXPRIORITY_HIGH); //Wave
		mSkinnedMesh->setTrackParams(1, 1.0f, 0.2f, D3DXPRIORITY_HIGH); //Run
		mSkinnedMesh->setTrackAnimationSet(0, 0);
		mSkinnedMesh->setTrackAnimationSet(1, 1);
	}
	if( gDInput->keyDown(DIK_2))
	{
		// Loiter-Wave Blending
		mSkinnedMesh->setTrackParams(0, 1.0f, 0.8f, D3DXPRIORITY_HIGH); //Wave
		mSkinnedMesh->setTrackParams(1, 1.0f, 0.2f, D3DXPRIORITY_HIGH); //Loiter
		mSkinnedMesh->setTrackAnimationSet(0, 0);
		mSkinnedMesh->setTrackAnimationSet(1, 3);
	}
	if( gDInput->keyDown(DIK_3))
	{
		// Walk-Wave Blending
		mSkinnedMesh->setTrackParams(0, 1.0f, 0.8f, D3DXPRIORITY_HIGH); //Wave
		mSkinnedMesh->setTrackParams(1, 1.0f, 0.2f, D3DXPRIORITY_HIGH); //Walk
		mSkinnedMesh->setTrackAnimationSet(0, 0);
		mSkinnedMesh->setTrackAnimationSet(1, 2);
	}

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


	// Animate the skinned mesh.
	mSkinnedMesh->update(dt);
}

void BlendingAnimationSetsDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	// Set FX Parameters.  In particular, for this demo note that we set the 
	// final transformation matrix array for vertex blending.

	D3DXMATRIX T, R, S, W, WIT;

	HR(mFX->SetMatrixArray(mhFinalXForms, mSkinnedMesh->getFinalXFormArray(), mSkinnedMesh->numBones()));
	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));
	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mWorld));
	HR(mFX->SetValue(mhMtrl, &mWhiteMtrl, sizeof(Mtrl)));
	HR(mFX->SetTexture(mhTex, mTex));

	D3DXMatrixRotationYawPitchRoll(&R, -(D3DX_PI*0.75f), -(D3DX_PI*0.5f), 0.0f);
	D3DXMatrixTranslation(&T, 0.0f, -2.5f, 0.0f);
	// Scale the mesh down.
	D3DXMatrixScaling(&S, 0.01f, 0.01f, 0.01f);
	W = S*R*T;
	D3DXMatrixInverse(&WIT, 0, &W);
	D3DXMatrixTranspose(&WIT, &WIT);
	HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
	HR(mFX->SetMatrix(mhWorld, &W));
	HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));

	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	mSkinnedMesh->draw();

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->display();
	drawText();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void BlendingAnimationSetsDemo::buildFX()
{
	// Create the FX from a .fx file.
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, "vblend2.fx", 
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech            = mFX->GetTechniqueByName("VBlend2Tech");
	mhWVP             = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans   = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhFinalXForms     = mFX->GetParameterByName(0, "gFinalXForms");
	mhMtrl            = mFX->GetParameterByName(0, "gMtrl");
	mhLight           = mFX->GetParameterByName(0, "gLight");
	mhEyePos          = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld           = mFX->GetParameterByName(0, "gWorld");
	mhTex             = mFX->GetParameterByName(0, "gTex");
}

void BlendingAnimationSetsDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void BlendingAnimationSetsDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void BlendingAnimationSetsDemo::initFont()
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

void BlendingAnimationSetsDemo::drawText()
{
	// Make static so memory is not allocated every frame.
	static char buffer[1024];
	std::string controls = "Controls:\nUse mouse to orbit and zoom.\nUse the 'W' and 'S' keys to alter the height of the camera.\nUse '1', '2', '3' to switch between the different blended animation sets.";

	UINT h = md3dPP.BackBufferHeight;
	RECT R = {5, h-(h/5), 0, 0};
	sprintf(buffer, "%s", controls.c_str());
	HR(mFont->DrawTextA(0, buffer, -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0,0,0)));
}
