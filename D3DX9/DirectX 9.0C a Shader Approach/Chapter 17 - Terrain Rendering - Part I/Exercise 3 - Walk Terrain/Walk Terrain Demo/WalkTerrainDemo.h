#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Terrain.h"
#include "Camera.h"

class WalkTerrainDemo : public D3DApp
{
public:
	WalkTerrainDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~WalkTerrainDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
};