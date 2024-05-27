#include "harrisDetect.hpp"
#include <iostream>
#include <cmath>

using namespace std;

namespace shun {

CVError HarrisDetect::FindFeature(const Image &img, const HarrisParam &param, HarrisResult &result) const
{
    CVError status = CVError::NOERROR;
    result.mCoord.clear();

    if (img.IsEmpty()) {
        status = CVError::INPUT;
        SHOW_ERROR_AND_RETURN(status);
    }

    Image grayImg;
    status = img.RGB2Gray(grayImg);
    SHOW_ERROR_AND_RETURN(status);

    Image sobelX, sobelY;
    status = grayImg.Sobel(sobelX, sobelY);
    SHOW_ERROR_AND_RETURN(status);

    float dx, dy;
    Image cov;
    status = cov.Allocate(grayImg.GetWidth(), grayImg.GetHeight(), 3);
    SHOW_ERROR_AND_RETURN(status);
    for (int y = 0; y < grayImg.GetHeight(); ++y) {
        for (int x = 0; x < grayImg.GetWidth(); ++x) {
            dx = sobelX.GetPixel(x, y, 0);
            dy = sobelY.GetPixel(x, y, 0);
            cov.SetPixel(x, y, 0, dx*dx);
            cov.SetPixel(x, y, 1, dx*dy);
            cov.SetPixel(x, y, 2, dy*dy);
        }
    }

    Image gaussian;
    status = cov.GaussianBlur(gaussian, param.sigma);
    SHOW_ERROR_AND_RETURN(status);    

    float h11, h12, h22, trace, det, R;
    Image response;
    status = response.Allocate(grayImg.GetWidth(), grayImg.GetHeight(), 1);
    SHOW_ERROR_AND_RETURN(status);
    for (int y = 0; y < response.GetHeight(); ++y) {
        for (int x = 0; x < response.GetWidth(); ++x) {
            // harris's response function
            h11 = gaussian.GetPixel(x, y, 0);
            h12 = gaussian.GetPixel(x, y, 1);
            h22 = gaussian.GetPixel(x, y, 2);
            det = h11 * h22 - h12 * h12;
            trace = h11 + h22;
            R = det - param.k * trace * trace;
            response.SetPixel(x, y, 0, R);
        }
    }

    Image normalResp;
    status = response.Normalize(normalResp, 0.0f, 255.0f);
    SHOW_ERROR_AND_RETURN(status);
    for (int y = 0; y < normalResp.GetHeight(); ++y) {
        for (int x = 0; x < normalResp.GetWidth(); ++x) {
            R = normalResp.GetPixel(x, y, 0);
            if (R > param.thd) {
                result.mCoord.push_back(make_pair(x, y));
            }
        }
    }

    if (mDebug)
    {
        // choose a corner coordinate to see how to work out
        #if (0)
        {
            int x = 90;
            int y = 90;

            cout << "at the coordinate (" << x << ", " << y << "):" << endl;
            cout << "gray: ";
            cout << grayImg.GetPixel(x-1, y-1, 0) << ", ";
            cout << grayImg.GetPixel(x-0, y-1, 0) << ", ";
            cout << grayImg.GetPixel(x+1, y-1, 0) << ", ";
            cout << grayImg.GetPixel(x-1, y+0, 0) << ", ";
            cout << grayImg.GetPixel(x-0, y+0, 0) << ", ";
            cout << grayImg.GetPixel(x+1, y+0, 0) << ", ";
            cout << grayImg.GetPixel(x-1, y+1, 0) << ", ";
            cout << grayImg.GetPixel(x-0, y+1, 0) << ", ";
            cout << grayImg.GetPixel(x+1, y+1, 0) << endl;
            cout << "sobel: ";
            cout << sobelX.GetPixel(x, y, 0) << ", ";
            cout << sobelY.GetPixel(x, y, 0) << endl;
            cout << "cov: ";        
            cout << cov.GetPixel(x, y, 0) << ", ";
            cout << cov.GetPixel(x, y, 1) << ", ";
            cout << cov.GetPixel(x, y, 2) << endl;
            cout << "blur: ";        
            cout << gaussian.GetPixel(x, y, 0) << ", ";
            cout << gaussian.GetPixel(x, y, 1) << ", ";
            cout << gaussian.GetPixel(x, y, 2) << endl;
            cout << "response: ";
            cout << response.GetPixel(x, y, 0) << endl;
            cout << "normal response: ";
            cout << normalResp.GetPixel(x, y, 0) << endl;
        }
        #endif

        // save the gray image
        string fileName = mDebugPath;
        fileName += "harris_gray.jpg";
        grayImg.WriteJpegImage(fileName.c_str());

        // save sobel images
        Image normalSX, normalSY;
        for (int y = 0; y < sobelX.GetHeight(); ++y) {
            for (int x = 0; x < sobelX.GetWidth(); ++x) {
                dx = sobelX.GetPixel(x, y, 0);
                dy = sobelY.GetPixel(x, y, 0);
                sobelX.SetPixel(x, y, 0, fabs(dx));
                sobelY.SetPixel(x, y, 0, fabs(dy));
            }
        }
        status = sobelX.Normalize(normalSX, 0, 255);
        SHOW_ERROR_AND_RETURN(status);
        status = sobelY.Normalize(normalSY, 0, 255);
        SHOW_ERROR_AND_RETURN(status);
        fileName = mDebugPath;
        fileName += "harris_sobelX.jpg";
        normalSX.WriteJpegImage(fileName.c_str());
        fileName = mDebugPath;
        fileName += "harris_sobelY.jpg";
        normalSY.WriteJpegImage(fileName.c_str());

        // save the response image
        fileName = mDebugPath;
        fileName += "harris_response.jpg";
        normalResp.WriteJpegImage(fileName.c_str());
    }

    return CVError::NOERROR;
}

}