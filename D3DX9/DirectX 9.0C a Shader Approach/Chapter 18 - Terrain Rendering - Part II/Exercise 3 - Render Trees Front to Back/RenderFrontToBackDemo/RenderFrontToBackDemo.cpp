//=============================================================================
// RenderFrontToBackDemo.cpp. Modified from Frank D Luna's Props Demo.
//
// Adds various props to our terrain scene like trees, water, and a castle.
// Demonstrates rendering of trees in a front to back order
//
// Controls: Use mouse to look and 'W', 'S', 'A', and 'D' keys to move.
//           Use 'M' to enable free camera, 'N' to disable free camera.
//			 Use 'R' to toggle rendering of the trees in a front to back order.
//=============================================================================

#include <list>
#include <tchar.h>
#include "RenderFrontToBackDemo.h"

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

	RenderFrontToBackDemo app(hInstance, "Render Front to Back Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	gDInput = &di;

	return gd3dApp->run();
}

RenderFrontToBackDemo::RenderFrontToBackDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if(!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mFont = NULL;

	mTime = 0.0f;

	mGfxStats = new GfxStats();

	// Set path to art resources.
	SetCurrentDirectory("Art/");

	// World space units are meters.  
	mTerrain = new Terrain(257, 257, 2.0f, 2.0f, 
		"castlehm257.raw", "grass.dds",	"dirt.dds",	
		"rock.dds", "blend_castle.dds", 0.5f, 0.0f);

	D3DXVECTOR3 toSun(-1.0f, 3.0f, 1.0f);
	D3DXVec3Normalize(&toSun, &toSun);
	mTerrain->setDirToSunW(toSun);

	// Setup water.
	D3DXMATRIX waterWorld;
	D3DXMatrixTranslation(&waterWorld, 8.0f, 35.0f, -80.0f);
	mWater = new Water(33, 33, 20, 20, waterWorld);

	// Initialize camera.
	gCamera->pos() = D3DXVECTOR3(8.0f, 35.0f, -100.0f);
	gCamera->setSpeed(20.0f);
	mFreeCamera = false;

	buildCastle();
	buildTrees();
	buildGrass();

	HR(D3DXCreateTextureFromFile(gd3dDevice, "grassfin0.dds", &mGrassTex));
	HR(D3DXCreateTextureFromFile(gd3dDevice, "whitetex.dds", &mWhiteTex));

	buildFX();

	mLight.dirW    = -toSun;
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.spec    = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));
	HR(mGrassFX->SetValue(mhGrassDirToSunW, &(-mLight.dirW), sizeof(D3DXVECTOR3)));

	mGfxStats->addVertices(mTerrain->getNumVertices());
	mGfxStats->addTriangles(mTerrain->getNumTriangles());
	mGfxStats->addVertices(mWater->getNumVertices());
	mGfxStats->addTriangles(mWater->getNumTriangles());
	mGfxStats->addVertices(mCastle.mesh->GetNumVertices());
	mGfxStats->addTriangles(mCastle.mesh->GetNumFaces());
	mGfxStats->addVertices(mTrees[0].mesh->GetNumVertices()*NUM_TREES/4);
	mGfxStats->addTriangles(mTrees[0].mesh->GetNumFaces()*NUM_TREES/4);
	mGfxStats->addVertices(mTrees[1].mesh->GetNumVertices()*NUM_TREES/4);
	mGfxStats->addTriangles(mTrees[1].mesh->GetNumFaces()*NUM_TREES/4);
	mGfxStats->addVertices(mTrees[2].mesh->GetNumVertices()*NUM_TREES/4);
	mGfxStats->addTriangles(mTrees[2].mesh->GetNumFaces()*NUM_TREES/4);
	mGfxStats->addVertices(mTrees[3].mesh->GetNumVertices()*NUM_TREES/4);
	mGfxStats->addTriangles(mTrees[3].mesh->GetNumFaces()*NUM_TREES/4);
	mGfxStats->addVertices(mGrassMesh->GetNumVertices());
	mGfxStats->addTriangles(mGrassMesh->GetNumFaces());

	frontToBackRendering = "Enabled";
	isFrontToBackRendering = true;

	initFont();

	onResetDevice();
}

RenderFrontToBackDemo::~RenderFrontToBackDemo()
{
	delete mGfxStats;
	delete mTerrain;
	delete mWater;
	ReleaseCOM(mWhiteTex);
	ReleaseCOM(mFX);
	ReleaseCOM(mGrassMesh);
	ReleaseCOM(mGrassTex);
	ReleaseCOM(mGrassFX);
	ReleaseCOM(mFont);

	DestroyAllVertexDeclarations();
}

bool RenderFrontToBackDemo::checkDeviceCaps()
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

void RenderFrontToBackDemo::onLostDevice()
{
	mGfxStats->onLostDevice();
	mTerrain->onLostDevice();
	mWater->onLostDevice();
	HR(mFX->OnLostDevice());
	HR(mGrassFX->OnLostDevice());
	HR(mFont->OnLostDevice());
}

void RenderFrontToBackDemo::onResetDevice()
{
	mGfxStats->onResetDevice();
	mTerrain->onResetDevice();
	mWater->onResetDevice();
	HR(mFX->OnResetDevice());
	HR(mGrassFX->OnResetDevice());
	HR(mFont->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;
	gCamera->setLens(D3DX_PI * 0.25f, w/h, 1.0f, 1000.0f);
}

void RenderFrontToBackDemo::updateScene(float dt)
{
	mTime += dt;

	mGfxStats->update(dt);

	gDInput->poll();

	// Fix camera to ground or free flying camera?
	if( gDInput->keyDown(DIK_N) )
		mFreeCamera = false;
	if( gDInput->keyDown(DIK_M) )
		mFreeCamera = true;
	if (gDInput->keyPressed(DIK_R))
	{
		isFrontToBackRendering = !isFrontToBackRendering;
		if (isFrontToBackRendering)
			frontToBackRendering = "Enabled";
		else
			frontToBackRendering = "Disabled";
	}

	if( mFreeCamera )
	{
		gCamera->update(dt, 0, 0);
	}
	else
	{
		gCamera->update(dt, mTerrain, 2.5f);
	}


	mWater->update(dt);
}

void RenderFrontToBackDemo::drawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff888888, 1.0f, 0));

	HR(gd3dDevice->BeginScene());

	HR(mFX->SetValue(mhEyePosW, &gCamera->pos(), sizeof(D3DXVECTOR3)));
	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	drawObject(mCastle, mCastleWorld);

	// Use alpha test to block non leaf pixels from being rendered in the
	// trees (i.e., use alpha mask).
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true));
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
	HR(gd3dDevice->SetRenderState(D3DRS_ALPHAREF, 200));

	std::list<D3DXMATRIX> visibleTrees;
	AABB box;
	for (UINT i = 0; i < NUM_TREES; ++i)
	{
		// Transform AABB into the world space.
		mTrees[0].box.xform(mTreeWorlds[i], box);
		if( gCamera->isVisible( box ) )
			visibleTrees.push_back(mTreeWorlds[i]);
	}

	// Sort front-to-back from camera.
	if (isFrontToBackRendering)
		visibleTrees.sort();

	UINT i = 0;
	for (std::list<D3DXMATRIX>::iterator iter = visibleTrees.begin(); iter != visibleTrees.end(); ++iter)
	{
		++i;
		drawObject(mTrees[0], *(iter));
	}

	HR(gd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false));

	HR(mFX->EndPass());
	HR(mFX->End());


	HR(mGrassFX->SetValue(mhGrassEyePosW, &gCamera->pos(), sizeof(D3DXVECTOR3)));
	HR(mGrassFX->SetMatrix(mhGrassViewProj, &(gCamera->viewProj())));
	HR(mGrassFX->SetFloat(mhGrassTime, mTime));
	HR(mGrassFX->Begin(&numPasses, 0));
	HR(mGrassFX->BeginPass(0));

	// Draw to depth buffer only.
	HR(mGrassMesh->DrawSubset(0));

	HR(mGrassFX->EndPass());
	HR(mGrassFX->End());

	mTerrain->draw();

	mWater->draw(); // draw alpha blended objects last.

	mGfxStats->display();
	drawText();

	HR(gd3dDevice->EndScene());

	// Present the backbuffer.
	HR(gd3dDevice->Present(0, 0, 0, 0));
}

void RenderFrontToBackDemo::buildFX()
{
	// Create the generic Light & Tex FX from a .fx file.
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gd3dDevice, "DirLightTex.fx", 
		0, 0, 0, 0, &mFX, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech            = mFX->GetTechniqueByName("DirLightTexTech");
	mhWVP             = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans   = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhMtrl            = mFX->GetParameterByName(0, "gMtrl");
	mhLight           = mFX->GetParameterByName(0, "gLight");
	mhEyePosW         = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld           = mFX->GetParameterByName(0, "gWorld");
	mhTex             = mFX->GetParameterByName(0, "gTex");


	// Create the grass FX from a .fx file.
	HR(D3DXCreateEffectFromFile(gd3dDevice, "grass.fx", 
		0, 0, 0, 0, &mGrassFX, &errors));
	if( errors )
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhGrassTech     = mGrassFX->GetTechniqueByName("GrassTech");
	mhGrassViewProj = mGrassFX->GetParameterByName(0, "gViewProj");
	mhGrassTex      = mGrassFX->GetParameterByName(0, "gTex");
	mhGrassTime     = mGrassFX->GetParameterByName(0, "gTime");
	mhGrassEyePosW  = mGrassFX->GetParameterByName(0, "gEyePosW");
	mhGrassDirToSunW= mGrassFX->GetParameterByName(0, "gDirToSunW");

	HR(mGrassFX->SetTechnique(mhGrassTech));
	HR(mGrassFX->SetTexture(mhGrassTex, mGrassTex));
}

void RenderFrontToBackDemo::drawObject(Object3D& obj, const D3DXMATRIX& toWorld)
{
	// Transform AABB into the world space.

	AABB box;
	obj.box.xform(toWorld, box);

	// Only draw if AABB is visible.
	if( gCamera->isVisible( box ) )
	{
		HR(mFX->SetMatrix(mhWVP, &(toWorld*gCamera->viewProj())));
		D3DXMATRIX worldInvTrans;
		D3DXMatrixInverse(&worldInvTrans, 0, &toWorld);
		D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
		HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
		HR(mFX->SetMatrix(mhWorld, &toWorld));

		for(UINT j = 0; j < obj.mtrls.size(); ++j)
		{
			HR(mFX->SetValue(mhMtrl, &obj.mtrls[j], sizeof(Mtrl)));

			// If there is a texture, then use.
			if(obj.textures[j] != 0)
			{
				HR(mFX->SetTexture(mhTex, obj.textures[j]));
			}

			// But if not, then set a pure white texture.  When the texture color
			// is multiplied by the color from lighting, it is like multiplying by
			// 1 and won't change the color from lighting.
			else
			{
				HR(mFX->SetTexture(mhTex, mWhiteTex));
			}

			HR(mFX->CommitChanges());
			HR(obj.mesh->DrawSubset(j));
		}
	}
}

void RenderFrontToBackDemo::buildCastle()
{
	// Load the castle mesh.
	D3DXMATRIX T, Ry;
	LoadXFile("castle.x", &mCastle.mesh, mCastle.mtrls, mCastle.textures);

	// Compute castle AABB.
	VertexPNT* v = 0;
	HR(mCastle.mesh->LockVertexBuffer(0, (void**)&v));
	HR(D3DXComputeBoundingBox(&v->pos, mCastle.mesh->GetNumVertices(),
		mCastle.mesh->GetNumBytesPerVertex(), 
		&mCastle.box.minPt, &mCastle.box.maxPt));
	HR(mCastle.mesh->UnlockVertexBuffer());

	// Manually set castle materials.
	for(UINT i = 0; i < mCastle.mtrls.size(); ++i)
	{
		mCastle.mtrls[i].ambient = WHITE*0.5f;
		mCastle.mtrls[i].diffuse = WHITE;
		mCastle.mtrls[i].spec = WHITE*0.8f;
		mCastle.mtrls[i].specPower = 28.0f;
	}

	// Build castle's world matrix.
	D3DXMatrixRotationY(&Ry, D3DX_PI);
	D3DXMatrixTranslation(&T, 8.0f, 35.0f, -80.0f);
	mCastleWorld = Ry*T;
}

void RenderFrontToBackDemo::buildTrees()
{
	// Load 4 unique meshes.  To draw more than 4 trees, we just draw these
	// 4 trees repeatedly, with different world matrices applied.
	LoadXFile("tree0.x", &mTrees[0].mesh, mTrees[0].mtrls, mTrees[0].textures);
	LoadXFile("tree1.x", &mTrees[1].mesh, mTrees[1].mtrls, mTrees[1].textures);
	LoadXFile("tree2.x", &mTrees[2].mesh, mTrees[2].mtrls, mTrees[2].textures);
	LoadXFile("tree3.x", &mTrees[3].mesh, mTrees[3].mtrls, mTrees[3].textures);



	// Build tree bounding boxes.
	for(int i = 0; i < 4; ++i)
	{
		VertexPNT* v = 0;
		HR(mTrees[i].mesh->LockVertexBuffer(0, (void**)&v));
		HR(D3DXComputeBoundingBox(&v->pos, mTrees[i].mesh->GetNumVertices(),
			mTrees[i].mesh->GetNumBytesPerVertex(), 
			&mTrees[i].box.minPt, &mTrees[i].box.maxPt));
		HR(mTrees[i].mesh->UnlockVertexBuffer());
	}


	// Build world matrices for NUM_TREES trees.  To do this, we generate a
	// random position on the terrain surface for each tree.  In reality, 
	// this is not the best way to do it, as we'd like to have more control and
	// manually place trees in the scene by an artist.  Nevertheless, this is 
	// an easy way to get trees in the scene of our demo.  To prevent trees 
	// from being placed on mountain peaks, or in the water, we can specify to 
	// only generate trees in an allowed height range.  By inspecting the heightmap
	// used in this demo, castlehm257.raw, the range [35, 50] seems to be a good
	// one to generate trees in.  Note that this method does not prevent trees from
	// interpenetrating with one another and it does not prevent the trees from
	// interpenetrating with the castle.

	// Scale down a bit do we ignore the borders of the terrain as candidates.
	int w = (int)(mTerrain->getWidth() * 0.8f);
	int d = (int)(mTerrain->getDepth() * 0.8f);
	D3DXMATRIX S, T;
	for(int i = 0; i < NUM_TREES; ++i)
	{
		float x = (float)((rand() % w) - (w*0.5f));
		float z = (float)((rand() % d) - (d*0.5f));

		// Subtract off height to embed trunk in ground.
		float y = mTerrain->getHeight(x, z) - 0.5f; 

		// Trees modeled to a different scale then ours, so scale them down to make sense.
		// Also randomize the height a bit.
		float treeScale = GetRandomFloat(0.15f, 0.25f);

		// Build tree's world matrix.
		D3DXMatrixTranslation(&T, x, y, z);	
		D3DXMatrixScaling(&S, treeScale, treeScale, treeScale);
		mTreeWorlds[i] = S*T;

		// Only generate trees in this height range.  If the height
		// is outside this range, generate a new random position and 
		// try again.
		if(y < 35.0f || y > 50.0f)
			--i; // We are trying again, so decrement back the index.
	}
}

void RenderFrontToBackDemo::buildGrass()
{
	D3DVERTEXELEMENT9 elems[MAX_FVF_DECL_SIZE];
	UINT numElems = 0;
	HR(GrassVertex::Decl->GetDeclaration(elems, &numElems));

	HR(D3DXCreateMesh(NUM_GRASS_BLOCKS*2, NUM_GRASS_BLOCKS*4, D3DXMESH_MANAGED, 
		elems, gd3dDevice, &mGrassMesh));

	GrassVertex* v = 0;
	WORD* k = 0;
	HR(mGrassMesh->LockVertexBuffer(0, (void**)&v));
	HR(mGrassMesh->LockIndexBuffer(0, (void**)&k));

	int indexOffset = 0;

	// Scale down the region in which we generate grass.
	int w = (int)(mTerrain->getWidth() * 0.15f);
	int d = (int)(mTerrain->getDepth() * 0.15f);

	// Randomly generate a grass block (three intersecting quads) around the 
	// terrain in the height range [35, 50] (similar to the trees).
	for(int i = 0; i < NUM_GRASS_BLOCKS; ++i)
	{
		//============================================
		// Construct vertices.

		// Generate random position in region.  Note that we also shift
		// this region to place it in the world.
		float x = (float)((rand() % w) - (w*0.5f)) - 30.0f;
		float z = (float)((rand() % d) - (d*0.5f)) - 20.0f;
		float y = mTerrain->getHeight(x, z); 

		// Only generate grass blocks in this height range.  If the height
		// is outside this range, generate a new random position and 
		// try again.
		if(y < 37.0f || y > 40.0f)
		{
			--i; // We are trying again, so decrement back the index.
			continue;
		}

		float sx = GetRandomFloat(0.75f, 1.25f); 
		float sy = GetRandomFloat(0.75f, 1.25f);
		float sz = GetRandomFloat(0.75f, 1.25f);
		D3DXVECTOR3 pos(x, y, z);
		D3DXVECTOR3 scale(sx, sy, sz);

		buildGrassFin(v, k, indexOffset, pos, scale);
		v += 4;
		k += 6;
	}

	HR(mGrassMesh->UnlockVertexBuffer());
	HR(mGrassMesh->UnlockIndexBuffer());


	// Fill in the attribute buffer (everything in subset 0)
	DWORD* attributeBufferPtr = 0;
	HR(mGrassMesh->LockAttributeBuffer(0, &attributeBufferPtr));
	for(UINT i = 0; i < mGrassMesh->GetNumFaces(); ++i)
		attributeBufferPtr[i] = 0;
	HR(mGrassMesh->UnlockAttributeBuffer());

	DWORD* adj = new DWORD[mGrassMesh->GetNumFaces()*3];
	HR(mGrassMesh->GenerateAdjacency(EPSILON, adj));
	HR(mGrassMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT|D3DXMESHOPT_VERTEXCACHE,
		adj, 0, 0, 0));

	delete [] adj;
}

void RenderFrontToBackDemo::buildGrassFin(GrassVertex* v, WORD* k, int& indexOffset, 
	D3DXVECTOR3& worldPos, D3DXVECTOR3& scale)
{
	// Only top vertices have non-zero amplitudes: 
	// The bottom vertices are fixed to the ground.
	float amp = GetRandomFloat(0.5f, 1.0f);
	v[0] = GrassVertex(D3DXVECTOR3(-1.0f,-0.5f, 0.0f), D3DXVECTOR2(0.0f, 1.0f), 0.0f);
	v[1] = GrassVertex(D3DXVECTOR3(-1.0f, 0.5f, 0.0f), D3DXVECTOR2(0.0f, 0.0f), amp);
	v[2] = GrassVertex(D3DXVECTOR3( 1.0f, 0.5f, 0.0f), D3DXVECTOR2(1.0f, 0.0f), amp);
	v[3] = GrassVertex(D3DXVECTOR3( 1.0f,-0.5f, 0.0f), D3DXVECTOR2(1.0f, 1.0f), 0.0f);

	// Set indices of fin.
	k[0] = 0 + indexOffset;
	k[1] = 1 + indexOffset;
	k[2] = 2 + indexOffset;
	k[3] = 0 + indexOffset;
	k[4] = 2 + indexOffset;
	k[5] = 3 + indexOffset;

	// Offset the indices by four to have the indices index into
	// the next four elements of the vertex buffer for the next fin.
	indexOffset += 4;

	// Scale the fins and randomize green color intensity.
	for(int i = 0; i < 4; ++i)
	{
		v[i].pos.x *= scale.x;
		v[i].pos.y *= scale.y;
		v[i].pos.z *= scale.z;

		// Generate random offset color (mostly green).
		v[i].colorOffset = D3DXCOLOR(
			GetRandomFloat(0.0f, 0.1f),
			GetRandomFloat(0.0f, 0.2f),
			GetRandomFloat(0.0f, 0.1f),
			0.0f);
	}

	// Add offset so that the bottom of fin touches the ground
	// when placed on terrain.  Otherwise, the fin's center point
	// will touch the ground and only half of the fin will show.
	float heightOver2 = (v[1].pos.y - v[0].pos.y) / 2;
	worldPos.y += heightOver2;

	// Set world center position for the quad.
	v[0].quadPos = worldPos;
	v[1].quadPos = worldPos;
	v[2].quadPos = worldPos;
	v[3].quadPos = worldPos;
}

void RenderFrontToBackDemo::initFont()
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

void RenderFrontToBackDemo::drawText()
{
	// Make static so memory is not allocated every frame.
	static char buffer[256];

	sprintf(buffer, "Controls:\n"
		"Use mouse to look and 'W', 'S', 'A', and 'D' keys to move.\n"
		"Use 'M' to enable free camera, 'N' to disable free camera.\n"
		"Use 'R' to toggle rendering of the trees in a front to back order.");

	RECT R = {5, md3dPP.BackBufferHeight-100, 0, 0};
	HR(mFont->DrawText(0, buffer, -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0,0,0)));

	sprintf(buffer, "Front to Back Rendering:\t%s%",
		frontToBackRendering.c_str());
	R.left = md3dPP.BackBufferWidth-200; R.top = 5;
	HR(mFont->DrawText(0, buffer, -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0,0,0)));
}

// Sort by distance from nearest to farthest from the camera.  In this
// way, we draw objects in front to back order to reduce overdraw 
// (i.e., depth test will prevent them from being processed further.
bool RenderFrontToBackDemo::Object3D::operator<(const Object3D& rhs) const
{
	D3DXVECTOR3 d1 = box.center() - gCamera->pos();
	D3DXVECTOR3 d2 = rhs.box.center() - gCamera->pos();
	return D3DXVec3LengthSq(&d1) < D3DXVec3LengthSq(&d2);
}