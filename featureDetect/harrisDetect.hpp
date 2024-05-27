#ifndef __HARRISDETECT_HPP__
#define __HARRISDETECT_HPP__

#include "featureDetect.hpp"
#include <vector>
#include <utility>

namespace shun {

    struct HarrisParam {
            HarrisParam(): sigma{2.0f}, k{0.04f}, thd{200} {}

            float sigma; // a variance for Gaussion blur
            float k;     // a const for Harris's response function [0.04 ~ 0.06]
            int thd;     // a threshold for normalized Harris's response function [0 - 255]
    };

    struct HarrisResult {
            std::vector<std::pair<int, int>> mCoord;
    };

    class HarrisDetect : public FeatureDetect {
        public:
            HarrisDetect(): FeatureDetect() {}
            virtual ~HarrisDetect() {}
            CVError FindFeature(const Image &img, const HarrisParam &param, HarrisResult &result) const;
    };

}

#endif  // __HARRISIMPLEMENT_HPP__