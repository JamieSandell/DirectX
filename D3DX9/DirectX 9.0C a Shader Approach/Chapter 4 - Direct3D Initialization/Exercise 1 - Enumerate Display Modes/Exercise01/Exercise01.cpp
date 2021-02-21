//////////////////////////////////////////////////////////////////////////
// For each adapter on your system, enumerate the display modes for the
// formats D3DFMT_X8R8G8B8 and D3DFMT_R5G6B5
//////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include "Exercise01.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise01 app(hInstance, "Exercise 01", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	return gd3dApp->run();
}

Exercise01::Exercise01(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	checkDeviceCaps();
}

Exercise01::~Exercise01()
{

}

bool Exercise01::checkDeviceCaps()
{
	D3DDISPLAYMODE mode;
	std::ofstream outFile(".\\disp.txt", std::ios_base::out);
	for (UINT i = 0; i < md3dObject->GetAdapterCount(); ++i)
	{
		UINT modeCnt = md3dObject->GetAdapterModeCount(i, D3DFMT_X8R8G8B8);

		for (UINT j = 0; j < modeCnt; ++j)
		{
			HR(md3dObject->EnumAdapterModes(i, D3DFMT_X8R8G8B8, j, &mode));
			outFile << "Width = " << mode.Width << "\t";
			outFile << "Height = " << mode.Height << "\t";
			outFile << "Format = " << mode.Format << "\t";
			outFile << "Refresh Rate = " << mode.RefreshRate << std::endl;
		}

		modeCnt = md3dObject->GetAdapterModeCount(i, D3DFMT_R5G6B5);

		for (UINT j = 0; j < modeCnt; ++j)
		{
			md3dObject->EnumAdapterModes(i, D3DFMT_R5G6B5, j, &mode);
			outFile << "Width = " << mode.Width << "\t";
			outFile << "Height = " << mode.Height << "\t";
			outFile << "Format = " << mode.Format << "\t";
			outFile << "Refresh Rate = " << mode.RefreshRate << std::endl;
		}
	}
	outFile.close();

	return true;
}