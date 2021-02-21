//=============================================================================
// DirectInput.cpp by Frank Luna (C) 2005 All Rights Reserved.
//=============================================================================

#include "d3dUtil.h"
#include "DirectInput.h"
#include "d3dApp.h"

DirectInput* gDInput = 0;

DirectInput::DirectInput(DWORD keyboardCoopFlags, DWORD mouseCoopFlags)
{
	ZeroMemory(mKeyboardState, sizeof(mKeyboardState));
	ZeroMemory(&mMouseStateCurr, sizeof(mMouseStateCurr));
	ZeroMemory(&mMouseStatePrev, sizeof(mMouseStatePrev));

	HR(DirectInput8Create(gd3dApp->getAppInst(), DIRECTINPUT_VERSION, 
		IID_IDirectInput8, (void**)&mDInput, 0));

	HR(mDInput->CreateDevice(GUID_SysKeyboard, &mKeyboard, 0));
	HR(mKeyboard->SetDataFormat(&c_dfDIKeyboard));
	HR(mKeyboard->SetCooperativeLevel(gd3dApp->getMainWnd(), keyboardCoopFlags));
	HR(mKeyboard->Acquire());

	HR(mDInput->CreateDevice(GUID_SysMouse, &mMouse, 0));
	HR(mMouse->SetDataFormat(&c_dfDIMouse2));
	HR(mMouse->SetCooperativeLevel(gd3dApp->getMainWnd(), mouseCoopFlags));
	HR(mMouse->Acquire());
}

DirectInput::~DirectInput()
{
	ReleaseCOM(mDInput);
	mKeyboard->Unacquire();
	mMouse->Unacquire();
	ReleaseCOM(mKeyboard);
	ReleaseCOM(mMouse);
}

void DirectInput::poll()
{
	// Poll keyboard.
	HRESULT hr = mKeyboard->GetDeviceState(sizeof(mKeyboardState), (void**)&mKeyboardState); 
	if( FAILED(hr) )
	{
		// Keyboard lost, zero out keyboard data structure.
		ZeroMemory(mKeyboardState, sizeof(mKeyboardState));

		 // Try to acquire for next time we poll.
		hr = mKeyboard->Acquire();
	}

	// Poll mouse.
	mMouseStatePrev = mMouseStateCurr;
	hr = mMouse->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&mMouseStateCurr); 
	if( FAILED(hr) )
	{
		// Mouse lost, zero out mouse data structure.
		ZeroMemory(&mMouseStateCurr, sizeof(mMouseStateCurr));
		ZeroMemory(&mMouseStateCurr, sizeof(mMouseStatePrev));

		// Try to acquire for next time we poll.
		hr = mMouse->Acquire(); 
	}
}

bool DirectInput::keyDown(char key)
{
	return (mKeyboardState[key] & 0x80) != 0;
}

bool DirectInput::mouseButtonDown(int button)
{
	return (mMouseStateCurr.rgbButtons[button] & 0x80) != 0;
}

bool DirectInput::mouseButtonPressed(int button)
{
	// If the previous mouse state had the button down and the current mouse state has the button up a mouse press has occurred.
	return ( ((mMouseStatePrev.rgbButtons[button] & 0x80) != 0) && ((mMouseStateCurr.rgbButtons[button] & 0x80) == 0) );
}

float DirectInput::mouseDX()
{
	return (float)mMouseStateCurr.lX;
}

float DirectInput::mouseDY()
{
	return (float)mMouseStateCurr.lY;
}

float DirectInput::mouseDZ()
{
	return (float)mMouseStateCurr.lZ;
}