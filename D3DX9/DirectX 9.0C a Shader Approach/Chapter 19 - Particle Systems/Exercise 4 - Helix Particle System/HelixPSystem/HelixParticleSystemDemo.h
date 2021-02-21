#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <list>
#include <ctime>
#include "Terrain.h"
#include "Camera.h"
#include "PSystem.h"

class Helix : public PSystem
{
private:
	D3DXVECTOR3* mEmitPoint;

public:
	Helix(const std::string& fxName, 
		const std::string& techName, 
		const std::string& texName, 
		const D3DXVECTOR3& accel, 
		const AABB& box,
		int maxNumParticles,
		float timePerSecond,
		D3DXVECTOR3* emitPoint)
		: PSystem(fxName, techName, texName, accel, box, maxNumParticles,
		timePerSecond)
	{
		mEmitPoint = emitPoint;
	}

	void initParticle(Particle& out)
	{
		// Generate about the origin.
		out.initialPos = *mEmitPoint;//D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		out.initialTime     = mTime;
		out.lifeTime       = 10.0f;
		out.initialColor    = WHITE;
		out.initialSize     = GetRandomFloat(8.0f, 12.0f);
		out.mass            = GetRandomFloat(0.8f, 1.2f);

		// Generate Random Direction
		D3DXVECTOR3 d;
		GetRandomVec(d);

		// Compute velocity.
		float speed = GetRandomFloat(10.0f, 15.0f);
		//out.initialVelocity = speed*d;
		out.initialVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}
};

class HelixDemo : public D3DApp
{
public:
	HelixDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~HelixDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
	PSystem*  mPSys;

	D3DXVECTOR3	mEmitPoint;
};