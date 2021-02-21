#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

class SphericalMappingDemo : public D3DApp
{
public:
	SphericalMappingDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~SphericalMappingDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	//Helper methods
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();
	void drawTeapot();

	enum AXIS
	{
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	};

	void genSphericalTexCoords();

private:
	GfxStats*			mGfxStats;

	ID3DXMesh*			mTeapot;
	IDirect3DTexture9*	mTeapotTex;

	ID3DXEffect*		mFX;
	D3DXHANDLE			mhTech;
	D3DXHANDLE			mhWVP;
	D3DXHANDLE			mhWorldInvTrans;
	D3DXHANDLE			mhAmbientLight;
	D3DXHANDLE			mhDiffuseLight;
	D3DXHANDLE			mhSpecLight;
	D3DXHANDLE			mhLightVecW;
	D3DXHANDLE			mhAmbientMtrl;
	D3DXHANDLE			mhDiffuseMtrl;
	D3DXHANDLE			mhSpecMtrl;
	D3DXHANDLE			mhSpecPower;
	D3DXHANDLE			mhEyePos;
	D3DXHANDLE			mhWorld;
	D3DXHANDLE			mhTex;

	D3DXCOLOR			mAmbientLight;
	D3DXCOLOR			mDiffuseLight;
	D3DXCOLOR			mSpecLight;
	D3DXVECTOR3			mLightVecW;

	Mtrl	mTeapotMtrl;

	float	mCameraRotationY;
	float	mCameraRadius;
	float	mCameraHeight;

	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};