//=============================================================================
// HelixParticleSystemDemo.cpp.
//
// Demonstrates a a helix particle system.
//
// Controls: Use mouse to look and 'W', 'S', 'A', and 'D' keys to move.
//=============================================================================

#include "HelixParticleSystemDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	srand(time(0));

	// Construct camera before application, since the application uses the camera.
	Camera camera;
	gCamera = &camera;

	HelixDemo app(hInstance, "Helix Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

HelixDemo::HelixDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// World space units are meters.  
	mTerrain = new Terrain(257, 257, 2.0f, 2.0f, 
		"heightmap1_257.raw",  
		"mud.dds",
		"stone.dds",
		"snow.dds",
		"blend_hm1.dds",
		0.4f, 0.0f);

	D3DXVECTOR3 toSun(1.0f, 1.0f, 1.0f);
	D3DXVec3Normalize(&toSun, &toSun);
	mTerrain->setDirToSunW(toSun);

	// Initialize camera.
	gCamera->pos() = D3DXVECTOR3(55.0f, 50.0f, 25.0f);
	gCamera->setSpeed(40.0f);

	// Initialize the particle system.
	D3DXMATRIX psysWorld;
	D3DXMatrixTranslation(&psysWorld, 
		55.0f, 55.0f, 55.0f);

	// Don't cull, but in practice you'd want to figure out a
	// bounding box based on the settings of the system.
	AABB psysBox; 
	psysBox.maxPt = D3DXVECTOR3(INFTY, INFTY, INFTY);
	psysBox.minPt = D3DXVECTOR3(-INFTY, -INFTY, -INFTY);

	mEmitPoint = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	// Accelerate due to gravity.
	mPSys = new Helix("helix.fx", "HelixTech", "bolt.dds", 
		D3DXVECTOR3(0.0f, 0.0f, 0.0f), psysBox, 20000, 0.001f, &mEmitPoint);
	mPSys->setWorldMtx(psysWorld);

	mGfxStats->addVertices(mTerrain->getNumVertices());
	mGfxStats->addTriangles(mTerrain->getNumTriangles());

	onResetDevice();
}

HelixDemo::~HelixDemo()
{
	delete mGfxStats;
	delete mTerrain;
	delete mPSys;

	DestroyAllVertexDeclarations();
}

bool HelixDemo::checkDeviceCaps()
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

void HelixDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	mTerrain->onLostDevice();
	mPSys->onLostDevice();
}

void HelixDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	mTerrain->onResetDevice();
	mPSys->onResetDevice();


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	gCamera->setLens(D3DX_PI * 0.25f, w/h, 0.01f, 5000.0f);
}

void HelixDemo::updateScene(float dt)
{
	mGfxStats->update(dt);

	gDInput->poll();

	gCamera->update(dt, 0, 0);

	//mEmitPoint.z += dt;
	//x = r cos(kt).	r = radius, k = speed the emitter traverses the helix, t = time
	//y = r sin(kt)
	//z = ct

	static float r = 1.0f; // radius of the circle
	static float k = 4.0f; // how fast points rotate on the circle
	static float c = 1.0f; // how fast points move down the circle
	static float t = 0.0f; // time
	t += dt;

	mEmitPoint.x = r * cosf(k*t);
	mEmitPoint.y = r * sinf(k*t);
	mEmitPoint.z = c * t;

	if (gDInput->keyDown(DIK_SPACE))
		t = 0.0f;

	mPSys->update(dt);
}

void HelixDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff666666, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	mTerrain->draw();
	mPSys->draw();

	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}
