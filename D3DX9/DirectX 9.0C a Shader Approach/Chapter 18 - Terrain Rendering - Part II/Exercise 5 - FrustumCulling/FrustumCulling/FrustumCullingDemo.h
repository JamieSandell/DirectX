#include <ctime>
#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Terrain.h"
#include "Camera.h"
#include "Water.h"

class FrustumCullingDemo : public D3DApp
{
public:
	FrustumCullingDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~FrustumCullingDemo();

private:
	struct Object3D
	{
		Object3D()
		{
			mesh = 0;
		}
		~Object3D()
		{
			ReleaseCOM(mesh);
			for(UINT i = 0; i < textures.size(); ++i)
				ReleaseCOM(textures[i]);
			ReleaseCOM(boundingSphereMesh);
			//ReleaseCOM(boundingAABoxMesh);
		}

		ID3DXMesh* mesh;
		std::vector<Mtrl> mtrls;
		std::vector<IDirect3DTexture9*> textures;
		D3DXMATRIX world;
		float scaling;
		D3DXVECTOR3 rotation;

		AABB box;
		ID3DXMesh* boundingAABoxMesh;
		D3DXMATRIX boxOffset;
		BoundingSphere sphere;
		D3DXVECTOR3 spherePos;
		D3DXMATRIX sphereOffset;
		ID3DXMesh* boundingSphereMesh;
		static Mtrl FrustumCullingDemo::Object3D::boundingVolumeMtrl;
	};

public:
	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	void initFont();
	void drawText();

	void buildFX();
	void drawObject(Object3D& obj, const D3DXMATRIX& toWorld);
	void drawBoundingVolume(ID3DXMesh* boundingVolumeMesh, const D3DXMATRIX& toWorld, const BoundingSphere &boundingVolume) const;
	void drawBoundingVolume(ID3DXMesh* boundingVolumeMesh, const D3DXMATRIX& toWorld, const AABB &boundingVolume) const;

	void buildCastle();
	void buildTrees();
	void buildGrass();
	void buildGrassFin(GrassVertex* v, WORD* k, int& indexOffset, 
		D3DXVECTOR3& worldPos, D3DXVECTOR3& scale);
	void buildBoundingVolumes(Object3D& obj);
	void buildBoundingVolumeMeshes(Object3D& obj);

private:
	void drawBoundingVolumeSharedCode(ID3DXMesh* boundingVolumeMesh, const D3DXMATRIX& toWorld) const;

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
	Water*    mWater;

	float mTime; // Time elapsed from program start.

	// Models
	Object3D mCastle;
	static const int NUM_TREES = 200;
	Object3D mTrees[NUM_TREES];

	static const int NUM_GRASS_BLOCKS = 4000;
	ID3DXMesh* mGrassMesh;
	IDirect3DTexture9* mGrassTex;

	// Grass FX
	ID3DXEffect* mGrassFX;
	D3DXHANDLE mhGrassTech;
	D3DXHANDLE mhGrassViewProj;
	D3DXHANDLE mhGrassTex;
	D3DXHANDLE mhGrassTime;
	D3DXHANDLE mhGrassEyePosW;
	D3DXHANDLE mhGrassDirToSunW;

	// General light/texture FX
	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePosW;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	// The sun.
	DirLight mLight;

	// Camera fixed to ground or can fly?
	bool mFreeCamera;

	// Default texture if no texture present for subset.
	IDirect3DTexture9* mWhiteTex;

	ID3DXFont* mFont;

	std::string mBoundingVolumeUsed;
	bool mIsBoundingVolumeSphere;
	bool mDrawBoundingVolumes;
	std::string mDrawBoundingVolumesStatus;
};