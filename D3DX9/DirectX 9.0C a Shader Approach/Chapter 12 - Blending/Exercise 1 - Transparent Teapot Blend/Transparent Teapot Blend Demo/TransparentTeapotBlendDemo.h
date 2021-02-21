#include <vector>
#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

class TransparentTeapotBlendDemo : public D3DApp
{
public:
	TransparentTeapotBlendDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~TransparentTeapotBlendDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildBoxGeometry();
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();
	void initFont();

	void drawCrate();
	void drawTeapot();
	void genSphericalTexCoords();
	void displayBlendStateAndControls();

private:
	GfxStats* mGfxStats;

	IDirect3DVertexBuffer9* mBoxVB;
	IDirect3DIndexBuffer9* mBoxIB;
	ID3DXMesh*             mTeapot;
	IDirect3DTexture9*     mCrateTex;
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
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;

	Mtrl mCrateMtrl;
	Mtrl mTeapotMtrl;

	D3DXVECTOR3 mLightVecW;
	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecularLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mCrateWorld;
	D3DXMATRIX mTeapotWorld;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;

	std::vector<D3DBLEND> mSrcBlend;
	std::vector<D3DBLEND> mDestBlend;
	int mintSrcBlend;
	int mintDestBlend;

	D3DXFONT_DESC mFontDesc;
	ID3DXFont* mFont;
};