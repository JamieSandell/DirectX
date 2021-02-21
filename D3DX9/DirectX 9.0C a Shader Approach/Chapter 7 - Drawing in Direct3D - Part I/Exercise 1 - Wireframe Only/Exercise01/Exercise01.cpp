//////////////////////////////////////////////////////////////////////////
//Recall that the first argument we pass to both
//IDirect3DDevice9::Draw-Primitive and
//IDirect3DDevice9::DrawIndexedPrimitive is a member of the
//D3DPRIMITIVETYPE enumerated type. We said that in this book we primarily
//concern ourselves with drawing triangle lists, and thus always specify
//D3DPT_TRIANSLELIST for this member. In this exercise, you investigate
//some of the other primitive types. To start, look up D3DPRIMITIVETYPE in
//the DirectX SDK documentation, and read about each member, and also
//follow the Triangle Strips and Triangle Fans links in the "remarks"
//section. Now draw (wireframe only — no shading) Figure 7.6a using the
//D3DPT_LINESTRIP type and draw (wireframe only — no shading) Figure 7.6b
//using the D3DPT_TRIANGLESFAN type.
//
//Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to alter
//the height of the camera.
//////////////////////////////////////////////////////////////////////////

#include "Exercise01.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	// Enable run-time memory check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise01 app(hInstance, "Exercise 01", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE | DISCL_FOREGROUND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

Exercise01::Exercise01(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!checkDeviceCaps())
	{
		MessageBoxA(0, "checkDeviceCaps() Failed.", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraHeight = 5.0f;
	mCameraRadius = 10.0f;
	mCameraRotationY = 1.2f * D3DX_PI;

	buildVertexBuffer();
	buildIndexBuffer();

	onResetDevice();

	InitAllVertexDeclarations();
}

Exercise01::~Exercise01()
{
	delete mGfxStats;
	ReleaseCOM(mLineStripVB);
	ReleaseCOM(mLineStripIB);
//	ReleaseCOM(mTriangleListVB);
	//ReleaseCOM(mTriangleListIB);

	DestroyAllVertexDeclarations();
}

bool Exercise01::checkDeviceCaps()
{
	return true;
}

void Exercise01::onLostDevice()
{
	mGfxStats->onLostDevice();
}

void Exercise01::onResetDevice()
{
	mGfxStats->onResetDevice();

	//The aspect ratio depends on the backbuffer dimensions, which can
	//possibly change after a reset. So rebuild the projection matrix
	buildProjectionMatrix();
}

void Exercise01::updateScene(float dt)
{
	// LineStrip shape has 6 vertices and 0 triangles
	mGfxStats->setVertexCount(6);
	mGfxStats->setTriCount(0);
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
	if( mCameraRadius < 5.0f )
		mCameraRadius = 5.0f;

	// The camera position/orientation relative to world space can 
	// change every frame based on input, so we need to rebuild the
	// view matrix every frame with the latest changes.
	buildViewMatrix();
}

void Exercise01::drawScene()
{
	//Clear the back buffer and depth buffer
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	//Let Direct3D know the vertex buffer, index buffer and vertex
	//declaration we are using
	HR(gd3dDevice->SetStreamSource(0, mLineStripVB, 0, sizeof(VertexPos)));
	HR(gd3dDevice->SetIndices(mLineStripIB));
	HR(gd3dDevice->SetVertexDeclaration(VertexPos::Decl));

	//World matrix is identity
	D3DXMATRIX w;
	D3DXMatrixIdentity(&w);
	HR(gd3dDevice->SetTransform(D3DTS_WORLD, &w));
	HR(gd3dDevice->SetTransform(D3DTS_VIEW, &mView));
	HR(gd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj));

	HR(gd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));
	HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, 6, 0, 6));

	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void Exercise01::buildIndexBuffer()
{
	// Obtain a pointer to a new index buffer
	HR(gd3dDevice->CreateIndexBuffer(7 * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mLineStripIB, 0));

	// Now lock it to obtain a pointer to its internal data, and write the shape's index data

	WORD* k = 0;

	HR(mLineStripIB->Lock(0, 0, (void**)&k, 0));

	k[0] = 0;
	k[1] = 1;
	k[2] = 2;
	k[3] = 3;
	k[4] = 4;
	k[5] = 5;
	k[6] = 0;

	HR(mLineStripIB->Unlock());
}

void Exercise01::buildVertexBuffer()
{
	//Obtain a pointer to a new vertex buffer
	HR(gd3dDevice->CreateVertexBuffer(6 * sizeof(VertexPos), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mLineStripVB, 0));

	//Now lock it to obtain a pointer to its internal data, and write the shape's vertex data

	VertexPos* v = 0;
	HR(mLineStripVB->Lock(0, 0, (void**)&v, 0));

	v[0] = VertexPos(0.0f, 0.0f, -1.0f);
	v[1] = VertexPos(0.3f, -0.7f, -1.0f);
	v[2] = VertexPos(1.0f, -0.35f, -1.0f);
	v[3] = VertexPos(0.5f, -0.2f, -1.0f);
	v[4] = VertexPos(0.6f, 0.1f, -1.0f);
	v[5] = VertexPos(0.3f, 0.5f, -1.0f);
	v[6] = VertexPos(0.0f, 0.0f, -1.0f);

	HR(mLineStripVB->Unlock());
}

void Exercise01::buildViewMatrix()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void Exercise01::buildProjectionMatrix()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
