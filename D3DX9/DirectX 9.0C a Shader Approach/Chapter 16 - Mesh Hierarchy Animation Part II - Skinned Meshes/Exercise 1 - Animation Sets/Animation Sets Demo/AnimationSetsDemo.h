#include <vector>
#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"
#include "SkinnedMesh.h"

class AnimationSetsDemo : public D3DApp
{
public:
	AnimationSetsDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~AnimationSetsDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();
	void initFont();
	void drawText();

private:
	GfxStats* mGfxStats;

	SkinnedMesh* mSkinnedMesh;  

	DirLight mLight;
	Mtrl     mWhiteMtrl;
	IDirect3DTexture9* mTex;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhFinalXForms;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;

	D3DXFONT_DESC mFontDesc;
	ID3DXFont* mFont;
};