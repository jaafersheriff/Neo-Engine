#include "FrameCounter.hpp"

#include "microprofile.h"

namespace neo {
    namespace util {

        void FrameCounter::init(double runTime) {
            mLastFrameTime = runTime;
        }

        void FrameCounter::update(double _runTime) {
            MICROPROFILE_SCOPEI("Util", "Util::update", MP_AUTO);
            /* Update delta time and FPS */
            float runTime = static_cast<float>(_runTime);
            mTotalFrames++;
            mTimeStep = runTime - mLastFrameTime;
            mLastFrameTime = runTime;
            mFramesInCount++;
            if (runTime - mLastFPSTime >= 1.0) {
                mFPS = mFramesInCount;
                if (mFPSList.size() == 100) {
                    mFPSList.erase(mFPSList.begin());
                }
                mFPSList.push_back(mFPS);
                mFramesInCount = 0;
                mLastFPSTime = runTime;
            }

            if (mTimeStepList.size() == 1000) {
                mTimeStepList.erase(mTimeStepList.begin());
            }
            mTimeStepList.push_back(static_cast<float>(mTimeStep) * 1000.f);

        }
    }
}