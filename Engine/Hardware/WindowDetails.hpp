#pragma once

namespace neo {
    struct WindowDetails {
        glm::uvec2 mSize = {1920, 1080};
        glm::uvec2 mPos = { 0,0 };
        bool mFullscreen = false;
        bool mVSyncEnabled = true;
        int mRefreshRate = 60;
        float mDPIScale = 1.f;
    };
}
