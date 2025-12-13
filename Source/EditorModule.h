#pragma once
#include <Module.h>
#include <ImGuiPass.h>
#include "Application.h"
#include "Logger.h"
#include <DebugDrawPass.h>
#include <ConfigurationView.h>
#include "DescriptorHeap.h"
#include "Window.h"

class ImGuiPass;
class Logger;



class EditorModule: public Module
{
public:
	EditorModule();
	~EditorModule();
	bool postInit();
	void preRender() override;
	void render() override;
	void postRender() override;

	ImGuiPass* GetImGuiPass() { return _gui; }
	void RenderSceneEditorWindow();
private:
	
	ImGuiPass* _gui = nullptr;

	ConfigurationView* _configurationView = nullptr;
	DebugDrawPass* _debugDrawPass = nullptr;
    bool _showMainDockspace = true;
    bool _showHierarchy = true;
    bool _showInspector = true;


    bool _firstFrame = true;

	// Scene editor state
	bool _showSceneEditor = true;
	ImVec2 _sceneViewSize = { 1520, 825 };
	bool _isViewportHovered = false;
	bool _isViewportFocused = false;
};

