#include "PSystem.h"

// Create a simple firework particle system which goes off
// whenever the user picks an asteroid.
class FireWork : public PSystem
{
public:
	FireWork(const std::string& fxName, 
		const std::string& techName, 
		const std::string& texName, 
		const D3DXVECTOR3& accel, 
		const AABB& box,
		int maxNumParticles,
		float timePerParticle);

	void initParticle(Particle& out);
};