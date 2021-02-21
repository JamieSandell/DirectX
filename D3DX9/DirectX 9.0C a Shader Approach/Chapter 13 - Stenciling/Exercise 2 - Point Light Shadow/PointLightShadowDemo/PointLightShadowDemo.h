#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

class PointLightShadowDemo : public D3DApp
{
public:
	PointLightShadowDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~PointLightShadowDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildRoomGeometry();
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

	void initFont();

	void drawRoom();
	void drawLight();
	void drawMirror();
	void drawTeapot();
	void drawReflectedTeapot();
	void drawReflectedLight();
	void drawTeapotShadow();
	void displayControls();

	void genSphericalTexCoords();
	void genColor();
	void clipShadow();

private:
	GfxStats* mGfxStats;

	IDirect3DVertexBuffer9* mRoomVB;
	ID3DXMesh*              mTeapot;

	IDirect3DTexture9*     mFloorTex;
	IDirect3DTexture9*     mWallTex;
	IDirect3DTexture9*     mMirrorTex;
	IDirect3DTexture9*     mTeapotTex;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhLightVecW;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhDiffuseLight;
	D3DXHANDLE   mhAmbientMtrl;
	D3DXHANDLE   mhAmbientLight;
	D3DXHANDLE   mhSpecularMtrl;
	D3DXHANDLE   mhSpecularLight;
	D3DXHANDLE   mhSpecularPower;
	D3DXHANDLE   mhAttenuation012;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;

	Mtrl mWhiteMtrl;
	Mtrl mShadowMtrl;

	D3DXVECTOR3 mLightVecW;
	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecularLight;
	D3DXVECTOR3 mAttenuation012;
	ID3DXMesh* mLightShape;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mRoomWorld;
	D3DXMATRIX mTeapotWorld;
	D3DXMATRIX mLightWorld;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;

	D3DXFONT_DESC mFontDesc;
	ID3DXFont* mFont;
};