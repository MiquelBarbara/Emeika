#pragma once
#include "EditorWindow.h"
#include "Model.h"
#include "ImGuizmo.h"

class SceneEditor; // Forward declaration

class EditorTransform : public EditorWindow
{
public:
	EditorTransform(Emeika::Model* model, SceneEditor* sceneEditor);
	const char* GetWindowName() const override { return "Transform"; }

	void Render() override;

private:
	Emeika::Model* model;
	SceneEditor* m_SceneEditor;
};