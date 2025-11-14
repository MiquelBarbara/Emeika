#pragma once

class EngineController {
public: 
	void Play();
	void Pause();
	void Tick();

	void ControlSpeed(float amount);
};
