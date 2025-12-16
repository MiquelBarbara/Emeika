#pragma once
#include "EditorWindow.h"
#include <vector>
using std::vector;

class PerformanceWindow: public EditorWindow
{
	public:
	const char* GetWindowName() const override { return "Performance"; }
	void Update() override;
	void Render() override;


private:
	vector<float> fps_log;
	vector<float> ms_log;
};

