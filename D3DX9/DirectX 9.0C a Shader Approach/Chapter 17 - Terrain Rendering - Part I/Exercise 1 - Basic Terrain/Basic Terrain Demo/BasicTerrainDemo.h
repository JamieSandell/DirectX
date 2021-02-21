#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"
#include "Heightmap.h"
#include "SkinnedMesh.h"
#include "Terrain.h"
#include "Camera.h"

class BasicTerrainDemo : public D3DApp
{
public:
	BasicTerrainDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~BasicTerrainDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildGridGeometry();
	void buildFX(ID3DXEffect* &fx, std::string file);
	void buildViewMtx();
	void buildProjMtx();
	void setFXTerrainParams();
	void setFXSkinnedMeshParams();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;

	Heightmap mHeightmap;

	ID3DXMesh* mTerrainMesh;
	SkinnedMesh* mSkinnedMesh;

	Mtrl     mWhiteMtrl;
	IDirect3DTexture9* mTex;

	DirLight mLight;

	IDirect3DTexture9* mTex0;
	IDirect3DTexture9* mTex1;
	IDirect3DTexture9* mTex2;
	IDirect3DTexture9* mBlendMap;

	ID3DXEffect* mFX;
	ID3DXEffect* mFXVBlend2;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhViewProj;
	D3DXHANDLE   mhDirToSunW;
	D3DXHANDLE   mhTex0;
	D3DXHANDLE   mhTex1;
	D3DXHANDLE   mhTex2;
	D3DXHANDLE   mhBlendMap;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhFinalXForms;
	D3DXHANDLE   mhLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXVECTOR3 mSkinnedMeshPos;
	D3DXMATRIX mWorldSkinnedMesh;

	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};