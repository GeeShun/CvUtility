#ifndef __FEATUREDETECT_HPP__
#define __FEATUREDETECT_HPP__

#include <string>
#include "image.hpp"

namespace shun {

    class FeatureDetect {
        public:
            FeatureDetect(): mDebug{0}, mDebugPath{"./"} {}
            virtual ~FeatureDetect() {}

            int mDebug;
            std::string mDebugPath;
    };

}

#endif // __FEATUREDETECT_HPP__