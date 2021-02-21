#include "d3dApp.h"
#include "DirectInput.h"
#include <crtdbg.h>
#include "GfxStats.h"
#include <list>
#include <ctime>
#include "Terrain.h"
#include "Camera.h"
#include "PSystem.h"

class Firework : public PSystem
{
public:
	Firework(const std::string& fxName, 
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
		// Generate about the origin.
		out.initialPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		out.initialTime     = mTime;
		out.lifeTime        = GetRandomFloat(4.0f, 5.0f);
		out.initialColor    = WHITE;
		out.initialSize     = GetRandomFloat(8.0f, 12.0f);
		out.mass            = GetRandomFloat(0.8f, 1.2f);

		// Generate Random Direction
		D3DXVECTOR3 d;
		GetRandomVec(d);

		// Compute velocity.
		float speed = GetRandomFloat(10.0f, 15.0f);
		out.initialVelocity = speed*d;
	}

	void update(float dt)
	{
		mTime += dt;

		// Rebuild the dead and alive list.  Note that resize(0) does
		// not deallocate memory (i.e., the capacity of the vector does
		// not change).
		mDeadParticles.resize(0);
		mAliveParticles.resize(0);

		// For each particle.
		for(int i = 0; i < mMaxNumParticles; ++i)
		{
			// Is the particle dead?
			if( (mTime - mParticles[i].initialTime) > mParticles[i].lifeTime)
			{
				mDeadParticles.push_back(&mParticles[i]);
			}
			else
			{
				mAliveParticles.push_back(&mParticles[i]);
			}
		}


		// A negative or zero mTimePerParticle value denotes
		// not to emit any particles.
		if( mTimePerParticle > 0.0f )
		{
			//Once all the particles are dead, reinitialise them all.
			if (mAliveParticles.size() == 0)
			{
				while(mDeadParticles.size() != 0)
					addParticle();
			}
		}
	}
};

class FireworkDemo : public D3DApp
{
public:
	FireworkDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~FireworkDemo();

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