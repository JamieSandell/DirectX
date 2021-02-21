//=============================================================================
// ProgressiveMeshDemo.cpp.
//
// Demonstrates how to create a progressive mesh to demonstrate Level of Detail
// (LoD) altering.
//
// Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to 
//           alter the height of the camera.
//=============================================================================

#include <tchar.h>
#include "ProgressiveMeshDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetBreakAlloc(645);
#endif

	ProgressiveMeshDemo app(hInstance, "Progressive Mesh Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

ProgressiveMeshDemo::ProgressiveMeshDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
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
	// Create the progressive mesh
	DWORD* adjaceny = NULL;
	adjaceny = new DWORD[3 * mMeshShip->GetNumFaces() * sizeof(DWORD)];
	DWORD* outAdjaceny = NULL;
	outAdjaceny = new DWORD[3 * mMeshShip->GetNumFaces() * sizeof(DWORD)];

	HR(mMeshShip->GenerateAdjacency(EPSILON, adjaceny));
	// Clean the mesh because we'll be performing simplifications
	LPD3DXBUFFER buffErr = NULL;
	// Have to create a temp mesh to store the cleaned mesh, as D3DXCleanMesh returns a new mesh,
	// this avoids memory leaks.
	ID3DXMesh* temp = NULL;
	HR(D3DXCleanMesh(D3DXCLEAN_SIMPLIFICATION, mMeshShip, adjaceny, &temp, outAdjaceny, &buffErr));
	ReleaseCOM(mMeshShip);
	mMeshShip = temp;
	temp = NULL;
	// Generate the progressive mesh
	HR(D3DXGeneratePMesh(mMeshShip, adjaceny, NULL, NULL, mMeshShip->GetNumFaces(), D3DXMESHSIMP_FACE, &mXPMeshShip));
	//Set vertex format to VertexPNT.
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::Decl->GetDeclaration(elements, &numElements);

	D3DXMatrixIdentity(&mWorld);

	// Create the white dummy texture.
	HR(D3DXCreateTextureFromFile(gd3dDevice, "whitetex.dds", &mWhiteTex));

	// Add main mesh geometry count.
	mGfxStats->addVertices(mMeshShip->GetNumVertices());
	mGfxStats->addTriangles(mMeshShip->GetNumFaces());

	buildFX();

	initFont();
	onResetDevice();

	delete [] adjaceny;
	delete [] outAdjaceny;
	ReleaseCOM(buffErr);
}

ProgressiveMeshDemo::~ProgressiveMeshDemo()
{
	delete mGfxStats;
	ReleaseCOM(mXPMeshShip);
	ReleaseCOM(mMeshShip);
	ReleaseCOM(mFX);
	ReleaseCOM(mFont);

	for(UINT i = 0; i < mTexShip.size(); ++i)
		ReleaseCOM(mTexShip[i]);

	ReleaseCOM(mWhiteTex);
	DestroyAllVertexDeclarations();
}

bool ProgressiveMeshDemo::checkDeviceCaps()
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

void ProgressiveMeshDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	HR(mFX->OnLostDevice());
	HR(mFont->OnLostDevice());
}

void ProgressiveMeshDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	HR(mFX->OnResetDevice());
	HR(mFont->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	buildProjMtx();
}

void ProgressiveMeshDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if( gDInput->keyDown(DIK_W) )	 
		mCameraHeight   += 25.0f * dt;
	if( gDInput->keyDown(DIK_S) )	 
		mCameraHeight   -= 25.0f * dt;
	if(gDInput->keyDown(DIK_MINUS)) // Decrease level of detail
	{
		// Sometimes it's not possible to just remove one face increase the number of faces to remove until one or more faces have been removed
		if (mXPMeshShip->GetNumFaces() > mXPMeshShip->GetMinFaces())
		{
			DWORD oldNumFaces = mXPMeshShip->GetNumFaces();
			DWORD newNumFaces = oldNumFaces;
			while (mXPMeshShip->GetNumFaces() == oldNumFaces)
			{
				newNumFaces--;
				HR(mXPMeshShip->SetNumFaces(newNumFaces));
			}
		}
	}
	if(gDInput->keyDown(DIK_EQUALS)) // Increase level of detail
	{
		// Sometimes it's not possible to just add one face increase the number of faces to add until one or more faces have been added
		if (mXPMeshShip->GetNumFaces() < mXPMeshShip->GetMaxFaces())
		{
			DWORD oldNumFaces = mXPMeshShip->GetNumFaces();
			DWORD newNumFaces = oldNumFaces;
			while (mXPMeshShip->GetNumFaces() == oldNumFaces)
			{
				newNumFaces++;
				HR(mXPMeshShip->SetNumFaces(newNumFaces));
			}
		}
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

void ProgressiveMeshDemo::drawScene()
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
	drawMesh(mMtrlShip, (ID3DXMesh*)mXPMeshShip, mTexShip);

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->display();
	//drawText();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void ProgressiveMeshDemo::drawMesh(const std::vector<Mtrl>& mtrl, ID3DXMesh* mesh, const std::vector<IDirect3DTexture9*>& tex)
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

void ProgressiveMeshDemo::drawText()
{
	// Make static so memory is not allocated every frame.
	static char buffer[1024];
	std::string lod = "Current Number of Faces:\t";
	std::string controls = "\nControls:\nUse mouse to orbit and zoom\nUse the 'W' and 'S' keys to alter the height of the camera.\nUse '-' to decrease the level of detail.\nUse '=' to increase the level of detail.";

	UINT h = md3dPP.BackBufferHeight;
	RECT R = {5, h-(h/5), 0, 0};
	sprintf(buffer, "%s%i%s",
		lod.c_str(), (int)mXPMeshShip->GetNumFaces(), controls.c_str());
	HR(mFont->DrawTextA(0, buffer, -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0,0,0)));
}

void ProgressiveMeshDemo::buildFX()
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

	ReleaseCOM(errors);
}

void ProgressiveMeshDemo::buildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void ProgressiveMeshDemo::buildProjMtx()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}

void ProgressiveMeshDemo::initFont()
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