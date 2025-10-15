#pragma once

#include "Module.h"

namespace DirectX { class Keyboard; class Mouse; class GamePad;  }

class InputModule : public Module
{
public:

    InputModule(HWND hWnd);

private:
    std::unique_ptr<Keyboard> keyboard;
    std::unique_ptr<Mouse> mouse;
    std::unique_ptr<GamePad> gamePad;
};
