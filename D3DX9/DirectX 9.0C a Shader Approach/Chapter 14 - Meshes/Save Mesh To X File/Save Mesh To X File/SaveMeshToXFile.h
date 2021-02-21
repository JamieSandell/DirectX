#include "tchar.h"
#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"
#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <list>
#include "Vertex.h"

class SaveMeshToXFileDemo : public D3DApp
{
public:
	SaveMeshToXFileDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~SaveMeshToXFileDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void drawGrid();
	void buildGeoBuffers();
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();
	void convertGridToMesh();
	void saveMeshToXFile();

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	ID3DXMesh* mMesh;
	//std::vector<Mtrl> mMtrl;
	//std::vector<IDirect3DTexture9*> mTex;

	IDirect3DVertexBuffer9* mVB;
	IDirect3DIndexBuffer9*  mIB;
	Mtrl mGridMtrl;

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