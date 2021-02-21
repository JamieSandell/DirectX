//////////////////////////////////////////////////////////////////////////
//Recall that exercise 2 of Chapter 6 asked you to construct the vertices
//and indices for the mesh in Figure 6.22. Now write those vertices and
//indices into vertex and index buffers and render the triangle mesh.
//
//Controls: Use mouse to orbit and zoom; use the 'W' and 'S' keys to alter
//the height of the camera.
//////////////////////////////////////////////////////////////////////////

#include "Exercise02.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	// Enable run-time memory check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise02 app(hInstance, "Exercise 02", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE | DISCL_FOREGROUND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

Exercise02::Exercise02(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
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

Exercise02::~Exercise02()
{
	delete mGfxStats;
	ReleaseCOM(mTetrahedronVB);
	ReleaseCOM(mTetrahedronIB);
//	ReleaseCOM(mTriangleListVB);
	//ReleaseCOM(mTriangleListIB);

	DestroyAllVertexDeclarations();
}

bool Exercise02::checkDeviceCaps()
{
	return true;
}

void Exercise02::onLostDevice()
{
	mGfxStats->onLostDevice();
}

void Exercise02::onResetDevice()
{
	mGfxStats->onResetDevice();

	//The aspect ratio depends on the backbuffer dimensions, which can
	//possibly change after a reset. So rebuild the projection matrix
	buildProjectionMatrix();
}

void Exercise02::updateScene(float dt)
{
	// Tetrahedron shape has 4 vertices and 4 triangles
	mGfxStats->setVertexCount(4);
	mGfxStats->setTriCount(4);
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

void Exercise02::drawScene()
{
	//Clear the back buffer and depth buffer
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	//Let Direct3D know the vertex buffer, index buffer and vertex
	//declaration we are using
	HR(gd3dDevice->SetStreamSource(0, mTetrahedronVB, 0, sizeof(VertexPos)));
	HR(gd3dDevice->SetIndices(mTetrahedronIB));
	HR(gd3dDevice->SetVertexDeclaration(VertexPos::Decl));

	//World matrix is identity
	D3DXMATRIX w;
	D3DXMatrixIdentity(&w);
	HR(gd3dDevice->SetTransform(D3DTS_WORLD, &w));
	HR(gd3dDevice->SetTransform(D3DTS_VIEW, &mView));
	HR(gd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj));

	HR(gd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));
	D3DXMATRIX t;
	D3DXMatrixTranslation(&t, 0.0f, 0.0f, -2.0f);
	HR(gd3dDevice->SetTransform(D3DTS_WORLD, &t));
	HR(gd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 4));

	mGfxStats->display();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void Exercise02::buildIndexBuffer()
{
	// Obtain a pointer to a new index buffer
	HR(gd3dDevice->CreateIndexBuffer(12 * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mTetrahedronIB, 0));

	// Now lock it to obtain a pointer to its internal data, and write the shape's index data

	WORD* k = 0;

	HR(mTetrahedronIB->Lock(0, 0, (void**)&k, 0));

	// Front face
	k[0] = 0;
	k[1] = 1;
	k[2] = 2;
	// Right face
	k[3] = 2;
	k[4] = 1;
	k[5] = 3;
	// Left face
	k[6] = 3;
	k[7] = 1;
	k[8] = 0;
	// Bottom face
	k[9] = 2;
	k[10] = 0;
	k[11] = 3;

	HR(mTetrahedronIB->Unlock());
}

void Exercise02::buildVertexBuffer()
{
	//Obtain a pointer to a new vertex buffer
	HR(gd3dDevice->CreateVertexBuffer(4 * sizeof(VertexPos), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mTetrahedronVB, 0));

	//Now lock it to obtain a pointer to its internal data, and write the shape's vertex data

	VertexPos* v = 0;
	HR(mTetrahedronVB->Lock(0, 0, (void**)&v, 0));

	float tetrahedronSize = 2.0f;
	v[0] = VertexPos(-tetrahedronSize, -tetrahedronSize, +tetrahedronSize); // Bottom left
	v[1] = VertexPos((+tetrahedronSize)/2, +tetrahedronSize, -tetrahedronSize/2); // Top middle
	v[2] = VertexPos(+tetrahedronSize, -tetrahedronSize, +tetrahedronSize); // Bottom right
	v[3] = VertexPos((+tetrahedronSize)/2, -tetrahedronSize, -tetrahedronSize/2); // Bottom top middle

	HR(mTetrahedronVB->Unlock());
}

void Exercise02::buildViewMatrix()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void Exercise02::buildProjectionMatrix()
{
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
