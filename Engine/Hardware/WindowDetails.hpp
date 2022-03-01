#pragma once

#include "glm/glm.hpp"

namespace neo {
    struct WindowDetails {
        glm::ivec2 mFrameSize = {};
        glm::ivec2 mFullscreenSize = {};
        glm::ivec2 mWindowSize = { 1920, 1080 };
        glm::ivec2 mWindowPos = { 0,0 };
        bool mFullscreen = false;
        bool mVSyncEnabled = true;

        glm::ivec2 getSize() const { return mFullscreen ? mFullscreenSize : mFrameSize; }
    };
}
