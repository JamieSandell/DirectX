#pragma once

#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Vertex.h"

class Exercise02 : public D3DApp
{
public:
	Exercise02(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~Exercise02();

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

	IDirect3DVertexBuffer9* mTetrahedronVB;
	IDirect3DIndexBuffer9* mTetrahedronIB;
	IDirect3DVertexBuffer9* mCubeVB;
	IDirect3DIndexBuffer9* mCubeIB;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};