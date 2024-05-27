#include "image.hpp"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <limits>
#include <jpeglib.h>
#include <jerror.h>

namespace shun {

using namespace std;

Image::Image()
{
    Init();
}

Image::Image(int width, int height, int channel)
{
    Init();
    Allocate(width, height, channel, 0);
}

Image::Image(const Image &rhs)
{
    Init();
    if (CVError::NOERROR == Allocate(rhs.mWidth, rhs.mHeight, rhs.mChannel, 0)) {
        mDebug = rhs.mDebug;
        memcpy(mData, rhs.mData, mSize*sizeof(float));
    }
}

Image::Image(Image &&rhs)
{
    mChannel = rhs.mChannel;
    mData = rhs.mData;
    mDebug = rhs.mDebug;
    mHeight = rhs.mHeight;
    mSize = rhs.mSize;
    mWidth = rhs.mWidth;
    rhs.mData = nullptr;
}

Image::~Image()
{
    Release();
}

Image& Image::operator=(const Image &rhs)
{
    if (this != &rhs) {
        if (CVError::NOERROR == Allocate(rhs.mWidth, rhs.mHeight, rhs.mChannel)) {
            mDebug = rhs.mDebug;
            memcpy(mData, rhs.mData, mSize*sizeof(float));
        }
    }

    return *this;
}

Image& Image::operator=(Image &&rhs)
{
    mChannel = rhs.mChannel;
    mData = rhs.mData;
    mDebug = rhs.mDebug;
    mHeight = rhs.mHeight;
    mSize = rhs.mSize;
    mWidth = rhs.mWidth;
    rhs.mData = nullptr;

    return *this;
}

void Image::Init()
{
    mWidth = 0;
    mHeight = 0;
    mChannel = 0;
    mSize = 0;
    mDebug = 0;
    mData = nullptr;
}

CVError Image::Allocate(int width, int height, int channel, int freeMemory)
{
    CVError status = CVError::NOERROR;

    if (width <= 0 || height <= 0 || channel <= 0) {
        status = CVError::INPUT;
        SHOW_ERROR_AND_RETURN(status);
    }

    if (freeMemory)
        Release();

    mData = new float [width * height * channel];
    if (mData) {
        mWidth = width;
        mHeight = height;
        mChannel = channel;
        mSize = width * height * channel;
        mDebug = 0;
    } else {
        status = CVError::MEMORY;
        SHOW_ERROR_AND_RETURN(status);
    }

    return status;
}

void Image::Release()
{
    if (mData) {
        delete [] mData;
        mData = nullptr;
    }
    mWidth = mHeight = mChannel = mSize = mDebug = 0;
}

float Image::GetPixel(int x, int y, int channel) const
{
    if (x < 0)
        x = 0;
    else if (x >= mWidth)
        x = mWidth - 1;

    if (y < 0)
        y = 0;
    else if (y >= mHeight)
        y = mHeight - 1;

    if (channel < 0)
        channel = 0;
    else if (channel >= mChannel)
        channel = mChannel - 1;

    return mData[y * mWidth * mChannel + x * mChannel + channel];
}

void Image::SetPixel(int x, int y, int channel, float value)
{
    if (x < 0)
        x = 0;
    else if (x >= mWidth)
        x = mWidth - 1;

    if (y < 0)
        y = 0;
    else if (y >= mHeight)
        y = mHeight - 1;

    if (channel < 0)
        channel = 0;
    else if (channel >= mChannel)
        channel = mChannel - 1;

    mData[y * mWidth * mChannel + x * mChannel + channel] = value;
}

CVError Image::ReadJpegImage(const char *pName)
{
    CVError status = CVError::NOERROR;

    FILE *pFile;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *pTmp;
    
    pFile = fopen(pName, "rb");
    if (pFile == nullptr) {
        status = CVError::FILEACCESS;
        SHOW_ERROR_AND_RETURN(status);
    }    

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, pFile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);    

    status = Allocate(cinfo.output_width, cinfo.output_height, cinfo.output_components);
    if (CVError::NOERROR != status) {
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(pFile);
        SHOW_ERROR_AND_RETURN(status);
    }

    pTmp = new unsigned char [cinfo.output_width * cinfo.output_components];
    if (pTmp == nullptr) {
        status = CVError::MEMORY;
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(pFile);
        SHOW_ERROR_AND_RETURN(status);
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned int i = 0;
        jpeg_read_scanlines(&cinfo, &pTmp, 1);
        for (; i < cinfo.output_width*cinfo.output_components; ++i)
            mData[(cinfo.output_scanline-1)*cinfo.output_width*cinfo.output_components+i] = (float)pTmp[i];
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    delete [] pTmp;
    fclose(pFile);

    return status;
}

CVError Image::WriteJpegImage(const char *pName, int quality) const
{
    CVError status = CVError::NOERROR;

    if (IsEmpty()) {
        status = CVError::INPUT;
        SHOW_ERROR_AND_RETURN(status);
    }

    FILE *pFile;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *pTmp;

    pFile = fopen(pName, "wb");
    if (pFile == nullptr) {
        status = CVError::FILEACCESS;
        SHOW_ERROR_AND_RETURN(status);
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, pFile);
    cinfo.image_width = mWidth;
    cinfo.image_height = mHeight;
    cinfo.input_components = mChannel;
    cinfo.in_color_space = (mChannel == 1) ? JCS_GRAYSCALE : JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    pTmp = new unsigned char [cinfo.image_width * cinfo.input_components];

    if (pTmp == nullptr) {
        status = CVError::MEMORY;
        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        fclose(pFile);
        SHOW_ERROR_AND_RETURN(status);
    }

    while ((int)cinfo.next_scanline < mHeight) {
        unsigned int i = 0;
        for (; i < cinfo.image_width*cinfo.input_components; ++i)
            pTmp[i] = (unsigned char)(mData[cinfo.next_scanline*cinfo.image_width*cinfo.input_components + i] + 0.5f);
        jpeg_write_scanlines(&cinfo, &pTmp, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    delete [] pTmp;
    fclose(pFile);

    return status;
}

CVError Image::RGB2Gray(Image &image) const
{
    CVError status = CVError::NOERROR;

    if (mChannel == 3) {
        status = image.Allocate(mWidth, mHeight, 1);
        SHOW_ERROR_AND_RETURN(status);

        for (int i = 0; i < mHeight; ++i) {
            for (int j = 0; j < mWidth; ++j) {
                image.mData[i*mWidth+j] = 
                    0.144f * mData[i*mWidth*mChannel+mChannel*j+0] +
                    0.587f * mData[i*mWidth*mChannel+mChannel*j+1] +
                    0.299f * mData[i*mWidth*mChannel+mChannel*j+2];
            }
        }
    } else if (mChannel == 1) {
        image = *this;
        if (image.IsEmpty()) {
            status = CVError::MEMORY;
            SHOW_ERROR_AND_RETURN(status);
        }
    } else {
        cerr << "mChannel must be 1 or 3" << endl;
        status = CVError::INPUT;
        SHOW_ERROR_AND_RETURN(status);
    }

    return status;
}

CVError Image::Normalize(Image &image, float lowerBoundary, float upperBoundary) const
{
    CVError status = CVError::NOERROR;
    float max[mChannel];
    float min[mChannel];
    float value;
    
    status = image.Allocate(mWidth, mHeight, mChannel);
    SHOW_ERROR_AND_RETURN(status);

    for (int c = 0; c < mChannel; ++c) {
        max[c] = numeric_limits<float>::lowest();
        min[c] = numeric_limits<float>::max();
        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                value = GetPixel(x, y, c);
                if (max[c] < value)
                    max[c] = value;
                if (min[c] > value)
                    min[c] = value;
            }
        }
    }

    for (int c = 0; c < mChannel; ++c) {
        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                value = GetPixel(x, y, c);
                value = (value - min[c]) / (max[c] - min[c]) * (upperBoundary - lowerBoundary) + lowerBoundary;
                image.SetPixel(x, y, c, value);
            }
        }
    }

    return status;
}

CVError Image::Sobel(Image &dX, Image &dY) const
{
    CVError status = CVError::NOERROR;

    if (IsEmpty()) {
        status = CVError::INPUT;
        SHOW_ERROR_AND_RETURN(status);
    }

    Image filterX; // horizontal
    Image filterY; // vertical
    static float kernel1[3] = {1.0f, 2.0f, 1.0f};
    static float kernel2[3] = {-1.0f, 0.0f, 1.0f};
    float sum1;
    float sum2;
    float value;

    status = filterX.Allocate(mWidth, mHeight, mChannel);
    SHOW_ERROR_AND_RETURN(status);
    status = filterY.Allocate(mWidth, mHeight, mChannel);
    SHOW_ERROR_AND_RETURN(status);
    status = dX.Allocate(mWidth, mHeight, mChannel);
    SHOW_ERROR_AND_RETURN(status);
    status = dY.Allocate(mWidth, mHeight, mChannel);
    SHOW_ERROR_AND_RETURN(status);

    for (int c = 0; c < mChannel; ++c) {
        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                sum1 = 0.0f;
                sum2 = 0.0f;
                for (int k = -1; k < 2; ++k) {
                    value = GetPixel(x, y+k, c);
                    sum1 += value * kernel1[1+k];
                    sum2 += value * kernel2[1+k];
                }
                filterX.SetPixel(x, y, c, sum1);
                filterY.SetPixel(x, y, c, sum2);
            }
        }
    }

    for (int c = 0; c < mChannel; ++c) {
        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                sum1 = 0.0f;
                sum2 = 0.0f;
                for (int k = -1; k < 2; ++k) {
                    sum1 += filterX.GetPixel(x+k, y, c) * kernel2[1+k];
                    sum2 += filterY.GetPixel(x+k, y, c) * kernel1[1+k];
                }
                dX.SetPixel(x, y, c, sum1);
                dY.SetPixel(x, y, c, sum2);
            }
        }
    }

    return status;
}

static CVError GaussianKernel(Image &kernel, int &size, int &center, float sigma)
{
    CVError status = CVError::NOERROR;

    if (sigma > 0.0f) {
        float tmp, sum;
        size = (int)ceil(6 * sigma);
        if (size % 2 == 0)
            ++size;
        center = size / 2;
        status = kernel.Allocate(size, 1, 1);
        SHOW_ERROR_AND_RETURN(status);

        sum = 0.0f;
        for (int i = -size/2; i <= size/2; ++i) {
            tmp = exp(-(i*i) / (2*sigma*sigma));    // e^(-x^2/(2*sigma^2))
            sum += tmp;
            kernel.SetPixel(center+i, 0, 0, tmp);
        }
        // normalization
        for (int i = 0; i < size; ++i) {
            tmp = kernel.GetPixel(i, 0, 0);
            kernel.SetPixel(i, 0, 0, tmp/sum);
        }

        return status;
    } else {
        status = CVError::INPUT;
        SHOW_ERROR_AND_RETURN(status);
    }

    return status;
}

CVError Image::GaussianBlur(Image &g, float sigma) const
{
    CVError status = CVError::NOERROR;

    if (IsEmpty()) {
        status = CVError::INPUT;
        SHOW_ERROR_AND_RETURN(status);
    }

    status = g.Allocate(mWidth, mHeight, mChannel);
    SHOW_ERROR_AND_RETURN(status);

    Image filter1D;
    status = filter1D.Allocate(mWidth, mHeight, mChannel);
    SHOW_ERROR_AND_RETURN(status);

    Image kernel;    
    int size, center;
    status = GaussianKernel(kernel, size, center, sigma);
    SHOW_ERROR_AND_RETURN(status);

    if (mDebug) {
        cout << "gaussian blur kernel size: " << size << endl;
        for (int i = 0; i < kernel.GetSize(); ++i)
            cout << kernel.GetPixel(i, 0, 0) << " ";
        cout << endl;
    }

    float sum;

    // convolve horizontal       
    for (int c = 0; c < mChannel; ++c) {
        for (int i = 0; i < mHeight; ++i) {
            for (int j = 0; j < mWidth; ++j) {
                sum = 0.0f;
                for (int k = 0; k < size; ++k) {
                    sum += GetPixel(j+k-center, i, c) * kernel.GetPixel(k, 0, 0);
                }
                filter1D.SetPixel(j, i, c, sum);
            }
        }
    }           

    // convolve vertical
    for (int c = 0; c < mChannel; ++c) {
        for (int i = 0; i < mHeight; ++i) {
            for (int j = 0; j < mWidth; ++j) {
                sum = 0.0f;
                for (int k = 0; k < size; ++k) {
                    sum += filter1D.GetPixel(j, i+k-center, c) * kernel.GetPixel(k, 0, 0);
                }
                g.SetPixel(j, i, c, sum);
            }
        }
    }

    return status;
}

void Image::DrawPoint(int x, int y, float r, float g, float b, int size)
{
    for (int i = x-size/2; i <= x+size/2; i++) {
        for (int j = y-size/2; j <= y+size/2; j++) {
            if (i < 0 || i >= mWidth) continue;
            if (j < 0 || j >= mHeight) continue;
            if (abs(i-x) + abs(j-y) > size/2) continue;
            if (mChannel == 3) {
                SetPixel(i, j, 0, r);
                SetPixel(i, j, 1, g);
                SetPixel(i, j, 2, b);
            } else if (mChannel == 1) {
                SetPixel(i, j, 0, r);
            }
        }
    }
}

void Image::DrawLine(int x1, int y1, int x2, int y2,  float r, float g, float b)
{
    if (x2 < x1) {
        swap(x1, x2);
        swap(y1, y2);
    }
    int dx = x2 - x1, dy = y2 - y1;
    for (int x = x1; x < x2; x++) {
        int y = y1 + dy*(x-x1)/dx;
        if (mChannel == 3) {
            SetPixel(x, y, 0, r);
            SetPixel(x, y, 1, g);
            SetPixel(x, y, 2, b);
        } else {
            SetPixel(x, y, 0, r);
        }
    }
}

}