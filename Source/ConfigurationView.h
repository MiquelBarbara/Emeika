#pragma once
#include <vector>

// We need to show the frames per second of the application
// Options to show/hide the grid and the axis gizmo (DebugDrawPass.h)

using namespace std;

class ConfigurationView
{
public:
	ConfigurationView() = default;
	~ConfigurationView() = default;

	void Render();
	void Update();
private:
	vector<float> fps_log ;
	vector<float> ms_log;
};

