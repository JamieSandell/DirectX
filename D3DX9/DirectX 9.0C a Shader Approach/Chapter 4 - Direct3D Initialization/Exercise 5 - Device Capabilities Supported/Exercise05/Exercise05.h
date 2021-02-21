#pragma once

#include "d3dApp.h"

class Exercise05 : public D3DApp
{
public:
	Exercise05(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~Exercise05();

	void onLostDevice();
	void onResetDevice();
	void drawScene();

private:
	ID3DXLine *mpline;
};