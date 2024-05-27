#ifndef __IMAGE_HPP__
#define __IMAGE_HPP__

#include "cvError.hpp"

namespace shun {

    class Image {
        public:
            // construct/destruct
            Image();
            Image(int width, int height, int channel);
            Image(const Image &rhs);
            Image(Image &&rhs);
            Image& operator=(const Image &rhs);
            Image& operator=(Image &&rhs);
            virtual ~Image();

            // data access
            CVError Allocate(int width, int height, int channel, int freeMemory = 1);
            void Release();
            int GetWidth() const { return mWidth; }
            int GetHeight() const { return mHeight; }
            int GetChannel() const { return mChannel; }
            int GetSize() const { return mSize; }
            float* GetData() { return mData; }
            float GetPixel(int x, int y, int channel) const;
            void SetPixel(int x, int y, int channel, float value);
            void SetDebug(int value) { mDebug = value; }
            int IsEmpty() const { return (mData == nullptr) ? 1 : 0; }

            // use libjpeg to read/write a jpeg image
            CVError ReadJpegImage(const char *pName);
            CVError WriteJpegImage(const char *pName, int quality = 80) const;

            // image transform
            CVError RGB2Gray(Image &image) const;
            CVError Normalize(Image &image, float lowerBoundary, float upperBoundary) const;

            // image filter
            CVError Sobel(Image &dX, Image &dY) const;
            CVError GaussianBlur(Image &g, float sigma) const;

            // draw
            void DrawPoint(int x, int y, float r, float g, float b, int size);
            void DrawLine(int x1, int y1, int x2, int y2, float r, float g, float b);
        
        protected:
            void Init();

            int mWidth;
            int mHeight;
            int mChannel;
            int mSize;
            int mDebug;
            float *mData;
    };
}

#endif  // __IMAGE_HPP__
