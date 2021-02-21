#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

struct BoneFrame
{
	// Note: The root bone's "parent" frame is the world space.

	D3DXVECTOR3 pos; // Relative to parent frame.
	float zAngle;    // Relative to parent frame.
	float yAngle;    // Relative to parent frame.

	D3DXMATRIX toParentXForm;
	D3DXMATRIX toWorldXForm;
};

class RobotArmRotationDemo : public D3DApp
{
public:
	RobotArmRotationDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~RobotArmRotationDemo();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	// Helper methods
	void buildFX();
	void buildViewMtx();
	void buildProjMtx();

	void buildBoneWorldTransforms();

private:
	GfxStats* mGfxStats;

	// We only need one bone mesh.  To draw several bones we just draw the
	// same mesh several times, but with a different transformation
	// applied so that it is drawn in a different place.
	ID3DXMesh* mBoneMesh;
	std::vector<Mtrl> mMtrl;
	std::vector<IDirect3DTexture9*> mTex;

	D3DXCOLOR selected;
	D3DXCOLOR notSelected;

	// Our robot arm has five bones.
	static const int NUM_BONES = 5;
	BoneFrame mBones[NUM_BONES];

	// Index into the bone array to the currently selected bone.
	// The user can select a bone and rotate it.
	int mBoneSelected;


	IDirect3DTexture9* mWhiteTex;

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

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};