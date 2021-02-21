#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <list>
#include "Vertex.h"


class Exercise04 : public D3DApp
{
public:
	Exercise04(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~Exercise04();

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

	void drawTeapot();

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	ID3DXMesh* mTeapot;

	IDirect3DVertexBuffer9* mVB;
	IDirect3DIndexBuffer9*  mIB;
	ID3DXEffect*            mFX;
	D3DXHANDLE              mhTech;
	D3DXHANDLE              mhWVP;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};