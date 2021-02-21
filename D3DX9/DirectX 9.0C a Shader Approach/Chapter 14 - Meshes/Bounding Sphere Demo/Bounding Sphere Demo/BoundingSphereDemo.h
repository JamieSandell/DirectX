#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <list>
#include "Vertex.h"

class BoundingSphereDemo : public D3DApp
{
public:
	BoundingSphereDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~BoundingSphereDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

private:
	GfxStats* mGfxStats;

	ID3DXMesh* mMesh;
	std::vector<Mtrl> mMtrl;
	std::vector<IDirect3DTexture9*> mTex;

	IDirect3DTexture9* mWhiteTex;

	ID3DXMesh*  mSphere;
	Mtrl        mSphereMtrl;
	BoundingSphere       mBoundingSphere;
	D3DXMATRIX  mBoundingSphereOffset;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	DirLight mLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};