#include "Globals.h"
#include "Application.h"
#include "InputModule.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "GamePad.h"

InputModule::InputModule(HWND hWnd)
{
    keyboard = std::make_unique<Keyboard>();
    mouse = std::make_unique<Mouse>();
    gamePad = std::make_unique<GamePad>();

    mouse->SetWindow(hWnd);
}


