
#include "Util.hpp"

namespace neo {
	int Util::mFPS = 0;
	int Util::mFramesInCount = 0;
	int Util::mTotalFrames = 0;
	double Util::mTimeStep = 0.0;
	double Util::mLastFPSTime = 0.0;
	double Util::mLastFrameTime = 0.0;
	std::vector<int> Util::mFPSList;
	std::vector<float> Util::mTimeStepList;
	const float Util::PI = glm::pi<float>();
}
