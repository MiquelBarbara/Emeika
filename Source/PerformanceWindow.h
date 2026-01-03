#pragma once
#include "EditorWindow.h"
#include <deque>
using std::deque;

class PerformanceWindow: public EditorWindow
{
	public:
	const char* GetWindowName() const override { return "Performance"; }
	void Update() override;
	void Render() override;

private:
	deque<float> fps_log;
	deque<float> ms_log;
};

