#pragma once
#include "Resources.h"
#include "CameraModule.h"
#include "EditorWindow.h"

class SceneEditor: public EditorWindow
{
public:
    SceneEditor(RenderTexture* renderTexture);
    const char* GetWindowName() const override { return "Scene Editor"; }
    void Render() override;
private:
    RenderTexture* m_RenderTexture;

    bool _isViewportHovered = false;
    bool _isViewportFocused = false;
};

