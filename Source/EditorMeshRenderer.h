#pragma once
#include "EditorComponent.h"

class Model;

class EditorMeshRenderer: EditorComponent<Model>
{
public:
	const char* GetName() const override { return "Mesh Renderer"; }
	void Render() override;
private:
};

