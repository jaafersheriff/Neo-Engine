#pragma once

namespace tracy {
    class View;
}

namespace neo {
    namespace util {
        class Profiler {
        public:
            Profiler();
            ~Profiler();
            Profiler(const Profiler&) = delete;
            Profiler& operator=(const Profiler&) = delete;

            void update(double);
            void imGuiEditor() const;

            std::unique_ptr<tracy::View> view;
            // std::vector<int> mFPSList;
            int mFPS = 0;                    /* Frames per second */
            double mTimeStep = 0.0;         /* Delta time */
            int mTotalFrames = 0;           /* Total frames since start up */
            int mMaxFPS = 0;           
        private:
            double mLastFPSTime = 0.0;      /* Time at which last FPS was calculated */
            int mFramesInCount = 0;         /* Number of frames in current second */
            double mLastFrameTime = 0.0;    /* Time at which last frame was rendered */
        };
    }
}
