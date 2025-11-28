#include "Globals.h"
#include "Timer.h"
#include "Time.h"
#include <algorithm>

namespace {

    struct State
    {
        Timer realTimer;
        float scale = 1.0f;

        uint64_t lastRealMs = 0;
        float lastRealDelta = 0.0f;

        uint32_t frames = 0;
    } g;

    inline float msToSec(uint64_t ms) { return ms * 0.001f; }

}

namespace Time {

    void update()
    {
        uint64_t nowRealMs = g.realTimer.Read();
        uint64_t deltaMs = nowRealMs - g.lastRealMs;

        g.lastRealDelta = msToSec(deltaMs);
        g.lastRealMs = nowRealMs;

        g.frames++;
    }

    float unscaledDeltaTime() { return g.lastRealDelta; }
    float deltaTime() { return g.scale * g.lastRealDelta; }
    float time() { return msToSec(g.realTimer.Read()) * g.scale; }
    float timeScale() { return g.scale; }
    void setTimeScale(float s) { g.scale = std::max(0.0f, s); }
    uint32_t frameCount() { return g.frames; }
    float realtimeSinceStartup() { return msToSec(g.realTimer.Read()); }

}
