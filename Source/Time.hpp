#pragma once
#include <cstdint>

namespace Time
{
    float deltaTime();          // last frame (scaled)
    float time();               // seconds since start (scaled)
    float timeScale();
    void  setTimeScale(float s);
    uint32_t frameCount();
    float realtimeSinceStartup();
    float unscaledDeltaTime();  // last frame (real)

    // Called once per frame by the engine
    void update();
}