#pragma once

#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include <ctime>
#include "Terrain.h"
#include "Camera.h"
#include "PSystem.h"

class Smoke : public PSystem
{
public:
	Smoke(const std::string& fxName, 
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
		out.initialPos = D3DXVECTOR3(60.0f, 30.0f, 60.0f);

		float speed = 10.0f;
		out.initialVelocity = D3DXVECTOR3(GetRandomFloat(-1.0f, 1.0f), speed, GetRandomFloat(-2.0f, 2.0f));

		out.initialTime      = mTime;
		out.lifeTime        = GetRandomFloat(2.0f, 4.0f);
		out.initialColor    = WHITE;
		out.initialSize     = GetRandomFloat(14.0f, 16.0f);
		out.mass            = GetRandomFloat(1.0f, 2.0f);
	}
};

class SmokeDemo : public D3DApp
{
public:
	SmokeDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~SmokeDemo();

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