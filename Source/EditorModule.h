#pragma once
#include <Module.h>
#include <ImGuiPass.h>
#include "Application.h"

class ImGuiPass;

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
};

