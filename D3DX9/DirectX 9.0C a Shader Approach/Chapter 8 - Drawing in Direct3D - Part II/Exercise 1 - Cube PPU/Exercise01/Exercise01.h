#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <list>
#include "Vertex.h"

class Exercise01 : public D3DApp
{
public:
	Exercise01(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~Exercise01();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	//Helper methods
	void buildGeoBuffers(void);
	void buildFX(void);
	void buildViewMatrix(void);
	void buildProjectionMatrix(void);
	void drawCube(void);

private:
	GfxStats* mGfxStats;

	IDirect3DVertexBuffer9* mVB;
	IDirect3DIndexBuffer9* mIB;
	ID3DXEffect* mFX;
	D3DXHANDLE mhTech;
	D3DXHANDLE mhWVP;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProjection;
};