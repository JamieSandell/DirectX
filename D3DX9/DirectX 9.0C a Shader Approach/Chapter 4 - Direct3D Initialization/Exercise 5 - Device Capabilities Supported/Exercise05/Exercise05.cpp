//////////////////////////////////////////////////////////////////////////
//Look up ID3DXLine in the SDK documentation and see if you can figure out
//how to draw some lines to the window. (Hint: Particularly concern
//yourself with the function D3DXCreateLine and the methods
//ID3DXLine::Begin,ID3DXLine::End,ID3DXLine::OnLostDevice,
//ID3DXLine::OnResetDevice, and ID3DXLine::Draw. Once you have basic line
//drawing working, you can experiment with the other methods that allow
//you to change the attributes of the line, such as its width and
//pattern.)
//////////////////////////////////////////////////////////////////////////

#include "Exercise05.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	//Enable run-time memory check for debug builds
#if defined(DEBUG) | defined (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise05 app(hInstance, "Exercise 05", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gd3dApp = &app;
	return gd3dApp->run();
}

Exercise05::Exercise05(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	HR(D3DXCreateLine(gd3dDevice, &mpline));
}

Exercise05::~Exercise05()
{
	ReleaseCOM(mpline);
}

void Exercise05::onLostDevice()
{
	HR(mpline->OnLostDevice());
}

void Exercise05::onResetDevice()
{
	HR(mpline->OnResetDevice());
}

void Exercise05::drawScene()
{
	D3DXVECTOR2 line[2];
	line[0].x = 22.0f;
	line[0].y = 22.0f;
	line[1].x = 555.0f;
	line[1].y = 555.0f;

	HR(mpline->SetAntialias(true));
	HR(mpline->SetWidth(5.0f));
	HR(mpline->SetPattern(1));
	HR(mpline->SetPatternScale(1.0f));

	HR(gd3dDevice->BeginScene());
		HR(mpline->Begin());
			HR(mpline->Draw(line, 2, D3DCOLOR_RGBA(255, 0, 0, 255)));
		HR(mpline->End());
	HR(gd3dDevice->EndScene());

	HR(gd3dDevice->Present(0,0,0,0));
}