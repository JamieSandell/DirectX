//////////////////////////////////////////////////////////////////////////
//Write a program that checks if your primary adapter supports the following device capabilities:
//
//D3DPRESENT_INTERVAL_IMMEDIATE 
//D3DPTADDRESSCAPS_CLAMP
//D3DFVFCAPS_PSIZE 
//D3DCAPS2_CANAUTOGENMIPMAP 
//D3DPRASTERCAPS_DITHER
//Moreover, determine the MaxPointSize,MaxPrimitiveCount,MaxActiveLights,MaxUserClipP1anes,MaxVertexIndex,andMaxVertexShaderConst
//of the primary display adapter. (Hint: All this information can be found by looking at the D3DCAPS9 structure
//— you will need to look up "D3DCAPS9" in the SDK documentation for help on this exercise.)
//////////////////////////////////////////////////////////////////////////
#include <fstream>
#include <iostream>
#include "Exercise04.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	//Enable run-time memory check for debug builds
#if defined(DEBUG) | defined (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise04 app(hInstance, "Exericse 04", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;
	return gd3dApp->run();
}

Exercise04::Exercise04(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	D3DCAPS9 caps;
	HR(md3dObject->GetDeviceCaps(D3DADAPTER_DEFAULT, mDevType, &caps));
	std::ofstream outFile(".\\Disp.txt", std::ios_base::out);

	if (caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
	{
		outFile << "D3DPRESENT_INTERVAL_IMMEDIATE supported." << std::endl;
	}
	else
	{
		outFile << "D3DPRESENT_INTERVAL_IMMEDIATE not supported." << std::endl;
	}
	if (caps.TextureAddressCaps & D3DPTADDRESSCAPS_CLAMP)
	{
		outFile << "D3DPTADDRESSCAPS_CLAMP supported." << std::endl;
	}
	else
	{
		outFile << "D3DPTADDRESSCAPS_CLAMP not supported." << std::endl;
	}
	if (caps.FVFCaps & D3DFVFCAPS_PSIZE)
	{
		outFile << "D3DFVFCAPS_PSIZE supported." << std::endl;
	} 
	else
	{
		outFile << "D3DFVFCAPS_PSIZE not supported." << std::endl;
	}
	if (caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
	{
		outFile << "D3DCAPS2_CANAUTOGENMIPMAP supported." << std::endl;
	} 
	else
	{
		outFile << "D3DCAPS2_CANAUTOGENMIPMAP not supported." << std::endl;
	}
	if (caps.RasterCaps & D3DPRASTERCAPS_DITHER)
	{
		outFile << "D3DPRASTERCAPS_DITHER supported." << std::endl;
	} 
	else
	{
		outFile << "D3DPRASTERCAPS_DITHER not supported." << std::endl;
	}
	outFile << "Max Point Size:\t" << caps.MaxPointSize << std::endl;
	outFile << "Max Primitive Count:\t" << caps.MaxPrimitiveCount << std::endl;
	outFile << "Max Active Lights:\t" << caps.MaxActiveLights << std::endl;
	outFile << "Max User Clip Planes:\t" << caps.MaxUserClipPlanes << std::endl;
	outFile << "Max Vertex Index:\t" << caps.MaxVertexIndex << std::endl;
	outFile << "Max Vertex Shader Const:\t" << caps.MaxVertexShaderConst << std::endl;

	outFile.close();
}

Exercise04::~Exercise04()
{

}