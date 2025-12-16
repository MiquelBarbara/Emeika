#pragma once
#include <Module.h>
#include <ImGuiPass.h>
#include "Application.h"
#include "Logger.h"
#include <DebugDrawPass.h>
#include <ConfigurationView.h>
#include "DescriptorHeap.h"
#include "EditorWindow.h"
#include <vector>

class ImGuiPass;
class Logger;
class HardwareWindow;
class PerformanceWindow;
class SceneEditor;


class EditorModule: public Module
{
public:
	EditorModule();
	~EditorModule();

	bool postInit();
	void update() override;
	void preRender() override;
	void render() override;
	void postRender() override;
	bool cleanUp() override;


	ImGuiPass* GetImGuiPass() { return _gui; }
private:
	void SetupDockLayout();
	std::vector<EditorWindow*> _editorWindows;
	Logger* _logger = nullptr;
	ConfigurationView* _configurationView = nullptr;
	HardwareWindow* _hardwareWindow = nullptr;
	PerformanceWindow* _performanceWindow = nullptr;
	SceneEditor* _sceneView = nullptr;

	ImGuiPass* _gui = nullptr;

	DebugDrawPass* _debugDrawPass = nullptr;
    bool _showMainDockspace = true;

    bool _firstFrame = true;

};

