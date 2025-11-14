#pragma once
#include <Module.h>
#include <ImGuiPass.h>
#include "Application.h"
#include "Logger.h"

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
private:
	ImGuiPass* _gui = nullptr;
	Logger* _logger = nullptr;

    bool _showMainDockspace = true;
    bool _showHierarchy = true;
    bool _showInspector = true;

    bool _firstFrame = true;
};

