//////////////////////////////////////////////////////////////////////////
// For the primary display adapter, check to see which of the following
// multisample types the adapter supports in both windowed mode and
// full-screen mode: D3DMULTISAMPLE_2_SAMPLES,...,D3DMULTISAMPLE_16_SAMPLES.
// Use a HAL device, and try it with formats D3DFMT_X8R8G8B8 and
// D3DFMT_R5G6B5. Save your results to a text file in a readable descriptive
// format. Check your results by comparing it to the results given in the
// DirectX Caps Viewer.
//////////////////////////////////////////////////////////////////////////
#include <fstream>
#include <iostream>
#include "Exercise02.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//Enable run-time memory check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise02 app(hInstance, "Exercise 02", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;

	return gd3dApp->run();
}

Exercise02::Exercise02(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	checkDeviceCaps();
}

Exercise02::~Exercise02()
{

}

bool Exercise02::checkDeviceCaps()
{
	std::ofstream outFile(".\\disp.txt", std::ios_base::out);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONMASKABLE, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_2_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_3_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_4_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_5_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_6_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_7_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_8_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_9_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_10_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_11_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_12_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_13_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_14_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_15_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_16_SAMPLES, true);
	//
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONMASKABLE, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_2_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_3_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_4_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_5_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_6_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_7_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_8_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_9_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_10_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_11_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_12_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_13_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_14_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_15_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_16_SAMPLES, false);
	//
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_NONMASKABLE, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_2_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_3_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_4_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_5_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_6_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_7_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_8_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_9_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_10_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_11_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_12_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_13_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_14_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_15_SAMPLES, true);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_16_SAMPLES, true);
	//
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_NONMASKABLE, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_2_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_3_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_4_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_5_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_6_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_7_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_8_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_9_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_10_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_11_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_12_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_13_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_14_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_15_SAMPLES, false);
	IsMultisampleAvailable(outFile, D3DFMT_R5G6B5, D3DMULTISAMPLE_16_SAMPLES, false);
	outFile.close();
	return true;
}

void Exercise02::IsMultisampleAvailable(std::ofstream &stream, _D3DFORMAT format, _D3DMULTISAMPLE_TYPE msampleType, bool windowed)
{
	DWORD qualityLevels;
	if (SUCCEEDED(md3dObject->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, format, windowed, msampleType,
		&qualityLevels)))
	{
		stream << "Multisampling technique:\t" << msampleType << " on format:\t" << format;
		if (windowed)
			stream << " in windowed mode";
		else
			stream << " in full-screen mode";
		stream << " available with quality levels:\t";
		for (DWORD i = 0; i <= qualityLevels; i++)
		{
			stream << i;
			if (i == qualityLevels)
				stream << "." << std::endl;
			else
				stream << ", ";
		}
	}
	else
	{
		stream << "Multisampling technique:\t" << msampleType << " on format:\t" << format;
		if (windowed)
			stream << " in windowed mode";
		else
			stream << " in full-screen mode";
		stream << " not available." << std::endl;
	}
}