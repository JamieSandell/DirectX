#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include <ctime>
#include "Terrain.h"
#include "Camera.h"
#include "PSystem.h"

class Flamethrower : public PSystem
{
public:
	Flamethrower(const std::string& fxName, 
		const std::string& techName, 
		const std::string& texName, 
		const D3DXVECTOR3& accel, 
		const AABB& box,
		int maxNumParticles,
		float timePerSecond)
		: PSystem(fxName, techName, texName, accel, box, maxNumParticles,
		timePerSecond)
	{
	}

	void initParticle(Particle& out)
	{
		// Generate at camera.
		out.initialPos = gCamera->pos();

		// Set down a bit so it looks like player is carrying the gun.
		out.initialPos.y -= 3.0f;

		// Fire in camera's look direction.
		float speed = 70.0f;
		out.initialVelocity = speed*gCamera->look();

		out.initialTime      = mTime;
		out.lifeTime        = GetRandomFloat(1.0f, 2.0f);;
		out.initialColor    = WHITE;
		out.initialSize     = GetRandomFloat(80.0f, 90.0f);
		out.mass            = GetRandomFloat(1.0f, 2.0f);;
	}
};

class FlamethrowerDemo : public D3DApp
{
public:
	FlamethrowerDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~FlamethrowerDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
	PSystem*  mPSys;
};