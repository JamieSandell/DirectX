#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <vector>
#include "Vertex.h"

class ProgressiveMeshDemo : public D3DApp
{
public:
	ProgressiveMeshDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~ProgressiveMeshDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods

	void drawMesh(const std::vector<Mtrl>& mtrl, ID3DXMesh* mesh, const std::vector<IDirect3DTexture9*> &tex);
	void drawText();
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();
	void initFont();

private:
	GfxStats* mGfxStats;

	ID3DXPMesh* mXPMeshShip;
	ID3DXMesh* mMeshShip;
	std::vector<Mtrl> mMtrlShip;
	std::vector<IDirect3DTexture9*> mTexShip;

	IDirect3DTexture9* mWhiteTex;

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

	D3DXFONT_DESC mFontDesc;
	ID3DXFont* mFont;
};