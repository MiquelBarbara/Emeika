#include "Globals.h"
#include "Timer.h"
#include "Time.h"
#include <algorithm>

namespace {

    struct State
    {
        Timer realTimer;   // measures real time
        Timer gameTimer;   // measures scaled time
        float scale = 1.0f;
        uint32_t frames = 0;
        float lastRealDelta = 0.0f;
    } g;

    float msToSec(uint64_t ms) { return ms * 0.001f; }

} // anon namespace

namespace Time {

    void update()
    {
        uint64_t nowRealMs = g.realTimer.Read();
        float nowRealSec = msToSec(nowRealMs);

        g.lastRealDelta = nowRealSec - msToSec(g.realTimer.Read() - nowRealMs);


        g.frames++;
    }

    float unscaledDeltaTime() { return g.lastRealDelta; }
    float deltaTime() { return g.scale == 0.0f ? 0.0f : unscaledDeltaTime() * g.scale; }
    float time() { return msToSec(g.gameTimer.Read()) * g.scale; }
    float timeScale() { return g.scale; }
    void  setTimeScale(float s) { g.scale = std::max(0.0f, s); }
    uint32_t frameCount() { return g.frames; }
    float realtimeSinceStartup() { return msToSec(g.realTimer.Read()); }


} // namespace Time