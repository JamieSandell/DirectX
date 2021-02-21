#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <list>
#include <vector>
#include <ctime>
#include "Terrain.h"
#include "Camera.h"
#include "FireWork.h"

// In order to not duplicate firework systems, we create
// a firework "instance" structure, which stores the position of
// the instance in world space and its relative time since being
// created.  In this way, we can draw several fireworks with only 
// one actual system by drawing the system in different world space
// positions and at different times.
struct FireWorkInstance
{
	float time;
	D3DXMATRIX toWorld;
};

// A simple asteroid structure to maintain the rotation, position, 
// and velocity of an asteroid.  
struct Asteroid
{
	D3DXVECTOR3 axis;
	float theta;
	D3DXVECTOR3 pos;
	D3DXVECTOR3 vel;
};

class AsteroidsDemo : public D3DApp
{
public:
	AsteroidsDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~AsteroidsDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	void initAsteroids();
	void buildFX();
	void getWorldPickingRay(D3DXVECTOR3& originW, D3DXVECTOR3& dirW);

private:
	GfxStats* mGfxStats;

	// We only need one firework system, as we just draw the same system
	// several times per frame in different positions and at different
	// relative times to simulate multiple systems.
	PSystem* mFireWork;

	// A list of firework *instances*
	std::list<FireWorkInstance> mFireWorkInstances;

	// A list of asteroids.
	static const int NUM_ASTEROIDS = 300;
	std::list<Asteroid> mAsteroids;

	// We only need one actual mesh, as we just draw the same mesh several
	// times per frame in different positions to simulate multiple asteroids.
	ID3DXMesh* mAsteroidMesh;
	std::vector<Mtrl> mAsteroidMtrls;
	std::vector<IDirect3DTexture9*> mAsteroidTextures;
	AABB mAsteroidBox;
	BoundingSphere mAsteroidSphere;

	// General light/texture FX
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

	// Default texture if no texture present for subset.
	IDirect3DTexture9* mWhiteTex;
};