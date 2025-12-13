#pragma once
#include "Resources.h"
#include "CameraModule.h"

class SceneEditor
{
public:
    void Draw() {
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        // Display rendered scene
        ImGui::Image((ImTextureID)m_RenderTexture.SRV().gpu.ptr,
            viewportSize,
            ImVec2(0, 0), ImVec2(1, 1));
    }
private:
    RenderTexture m_RenderTexture;
    CameraModule* m_Camera;
    float m_ClearColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
};

