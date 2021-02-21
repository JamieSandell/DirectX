#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <vector>
#include "Vertex.h"

class ConcatenateMeshesDemo : public D3DApp
{
public:
	ConcatenateMeshesDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~ConcatenateMeshesDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods

	void drawMesh(const std::vector<Mtrl>& mtrl, ID3DXMesh* mesh, const std::vector<IDirect3DTexture9*> &tex);
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

private:
	GfxStats* mGfxStats;

	ID3DXMesh* mMeshShip;
	ID3DXMesh* mMeshGrid;
	ID3DXMesh* mMeshJoined;
	std::vector<Mtrl> mMtrlShip;
	std::vector<Mtrl> mMtrlGrid;
	std::vector<Mtrl> mMtrlJoined;
	std::vector<IDirect3DTexture9*> mTexShip;
	std::vector<IDirect3DTexture9*> mTexGrid;
	std::vector<IDirect3DTexture9*> mTexJoined;

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
};