#pragma once

#include <vector>

namespace neo {
    namespace util {
        struct FrameCounter {
            FrameCounter() = default;
            ~FrameCounter() = default;
            FrameCounter(const FrameCounter&) = delete;
            FrameCounter& operator=(const FrameCounter&) = delete;

            void update(double);
            void imGuiEditor() const;

        public:
            std::vector<int> mFPSList;
            int mFPS = 0;                    /* Frames per second */
            double mTimeStep = 0.0;         /* Delta time */
            int mTotalFrames = 0;           /* Total frames since start up */
            int mMaxFPS = 0;           
        private:
            double mLastFPSTime = 0.0;      /* Time at which last FPS was calculated */
            int mFramesInCount = 0;         /* Number of frames in current second */
        };
    }
}
