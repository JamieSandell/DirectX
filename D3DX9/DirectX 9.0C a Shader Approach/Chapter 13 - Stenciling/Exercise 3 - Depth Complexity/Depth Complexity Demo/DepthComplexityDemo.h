#include "tchar.h"
#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

class DepthComplexityDemo : public D3DApp
{
public:
	DepthComplexityDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~DepthComplexityDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildQuadBuffers();
	void buildGeoBuffers();
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

	void drawGrid();
	void drawCylinders();
	void drawSpheres();
	void drawFullScreenQuad(D3DXCOLOR col);

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	ID3DXMesh* mCylinder;
	ID3DXMesh* mSphere;

	IDirect3DVertexBuffer9* mVB;
	IDirect3DIndexBuffer9*  mIB;
	IDirect3DVertexBuffer9* mVBQuad;
	IDirect3DIndexBuffer9*  mIBQuad;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhAmbientMtrl;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhSpecMtrl;
	D3DXHANDLE   mhSpecPower;
	D3DXHANDLE   mhQuadCol;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhLight;

	Mtrl  mGridMtrl;
	Mtrl  mCylinderMtrl;
	Mtrl  mSphereMtrl;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};