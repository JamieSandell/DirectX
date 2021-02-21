#pragma once

#include "d3dApp.h"

class Exercise02 : public D3DApp
{
public:
	Exercise02(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~Exercise02();

	bool checkDeviceCaps();

	void IsMultisampleAvailable(std::ofstream &stream, _D3DFORMAT format, _D3DMULTISAMPLE_TYPE msampleType, bool windowed);
};