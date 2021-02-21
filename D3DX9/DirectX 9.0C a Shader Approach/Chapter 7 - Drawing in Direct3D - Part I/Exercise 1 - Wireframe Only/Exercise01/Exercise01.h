#pragma once

#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

class Exercise01 : public D3DApp
{
public:
	Exercise01(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~Exercise01();

	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();

	//Helper methods
	void buildVertexBuffer();
	void buildIndexBuffer();
	void buildViewMatrix();
	void buildProjectionMatrix();

private:
	GfxStats* mGfxStats;

	IDirect3DVertexBuffer9* mLineStripVB;
	IDirect3DIndexBuffer9* mLineStripIB;
	IDirect3DVertexBuffer9* mTriangleListVB;
	IDirect3DIndexBuffer9* mTriangleListIB;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};