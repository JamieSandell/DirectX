#include "FireWork.h"

FireWork::FireWork(const std::string& fxName, 
	const std::string& techName, 
	const std::string& texName, 
	const D3DXVECTOR3& accel, 
	const AABB& box,
	int maxNumParticles,
	float timePerParticle)
	: PSystem(fxName, techName, texName, accel, box, 
	maxNumParticles, timePerParticle)
{
	for(int i = 0; i < mMaxNumParticles; ++i)
	{
		initParticle(mParticles[i]);
	}
}

void FireWork::initParticle(Particle& out)
{
	out.initialTime = 0.0f;
	out.initialSize  = GetRandomFloat(12.0f, 15.0f);
	out.lifeTime = 10.0f;

	// Generate Random Direction
	D3DXVECTOR3 d;
	GetRandomVec(d);

	// Compute velocity.
	float speed = GetRandomFloat(100.0f, 150.0f);
	out.initialVelocity = speed*d;

	out.initialColor = WHITE;
	out.mass = GetRandomFloat(2.0f, 4.0f);

	float r = GetRandomFloat(0.0f, 2.0f);
	out.initialPos = r*d;
}