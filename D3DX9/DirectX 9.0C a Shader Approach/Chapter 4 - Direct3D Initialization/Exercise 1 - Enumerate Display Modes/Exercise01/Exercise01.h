#pragma once

#include "d3dApp.h"

class Exercise01 : public D3DApp
{
public:
	Exercise01(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~Exercise01();

	bool checkDeviceCaps();
};