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

    void Resize(ImVec2 contentRegion);

private:
    CameraModule m_Camera;

};

