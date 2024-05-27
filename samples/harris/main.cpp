#include "harrisDetect.hpp"

using namespace shun;
using namespace std;

int main()
{
    Image image;
    HarrisParam param;
    HarrisResult result;
    HarrisDetect harris;

    image.ReadJpegImage("../../images/chessboard.jpg");
    param.thd = 150;
    harris.mDebug = 1;
    harris.FindFeature(image, param, result);

    for (auto keypoint : result.mCoord) {
        image.DrawPoint(keypoint.first, keypoint.second, 255.0f, 0.0f, 0.0f, 5);
    }
    image.WriteJpegImage("result.jpg");

    return 0;
}