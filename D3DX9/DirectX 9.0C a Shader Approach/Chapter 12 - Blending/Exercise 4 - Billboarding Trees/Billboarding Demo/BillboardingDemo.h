#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

class BillboardingDemo : public D3DApp
{
public:
	BillboardingDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~BillboardingDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildGeoBuffers();
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

	void drawGrid();
	void drawTrees();

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	IDirect3DVertexBuffer9* mVB;
	IDirect3DIndexBuffer9*  mIB;
	IDirect3DVertexBuffer9* mVBTrees;
	IDirect3DIndexBuffer9*  mIBTrees;

	IDirect3DTexture9* mTreeTex;
	IDirect3DTexture9* mGridTex;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhBBOffset;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhAmbientLight;
	D3DXHANDLE   mhDiffuseLight;
	D3DXHANDLE   mhSpecLight;
	D3DXHANDLE   mhLightVecW;
	D3DXHANDLE   mhAmbientMtrl;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhSpecMtrl;
	D3DXHANDLE   mhSpecPower;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE	 mhViewProj;

	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecLight;
	D3DXVECTOR3 mLightVecW;

	Mtrl  mGridMtrl;
	Mtrl  mTreeMtrl;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mViewProj;
};