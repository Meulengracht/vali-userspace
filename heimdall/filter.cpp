
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <algorithm>
#include <chrono>
#include <iostream>

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

class Image
{
public:
    Image(int width, int height);
    Image(const std::string& path);
    ~Image();

    void         SetColor(int pixelIndex, struct Color&);
    void         SetColor(int pixelIndex, unsigned char r, unsigned char g, unsigned char b);
    struct Color GetColor(int pixelIndex);

    void* GetPixels() const;
    int   Width() const;
    int   Height() const;

private:
    void* m_data;
    int   m_width;
    int   m_height;
};

Image::Image(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_data(new unsigned char[width * height * 4]) { }

Image::Image(const std::string& path)
    : m_width(0)
    , m_height(0)
    , m_data(nullptr)
{

}

Image::~Image()
{
    if (m_data != nullptr) {
        delete[] static_cast<char*>(m_data);
    }
}

void Image::SetColor(int pixelIndex, struct Color& color)
{
    SetColor(pixelIndex, color.r, color.g, color.b);
}

void Image::SetColor(int pixelIndex, unsigned char r, unsigned char g, unsigned char b)
{
    if (pixelIndex < 0 || pixelIndex >= (m_width * m_height)) {
        return;
    }
    
    ((uint32_t*)m_data)[pixelIndex] = (0xFF000000 | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b);
}

struct Color Image::GetColor(int pixelIndex)
{
    struct Color color = { 0 };
    if (pixelIndex < 0 || pixelIndex >= (m_width * m_height)) {
        return color;
    }

    unsigned int rawPixel = ((uint32_t*)m_data)[pixelIndex];
    color.a = (rawPixel >> 24) & 0xFF;
    color.r = (rawPixel >> 16) & 0xFF;
    color.g = (rawPixel >> 8) & 0xFF;
    color.b = rawPixel & 0xFF;
    return color;
}

void* Image::GetPixels() const
{
    return m_data;
}

int Image::Width() const
{
    return m_width;
}

int Image::Height() const
{
    return m_height;
}

/**
 * Guassian blur implemented using box-approximation. Not a perfect blur
 * but the performance is much better. Error-rate is low.
 * http://blog.ivank.net/fastest-gaussian-blur.html
 */
class GuassianFilter
{
public:
    Image* Apply(Image& image, double radius);

private:
    int* CreateBoxesForGuass(double sigma, int boxCount);
    void BoxBlur(Image& source, Image& output, int radius);
    void BoxBlurH(Image& source, Image& output, int radius);
    void BoxBlurT(Image& source, Image& output, int radius);
};

Image* GuassianFilter::Apply(Image& image, double radius)
{
    auto output = new Image(image.Width(), image.Height());
    auto boxes = CreateBoxesForGuass(radius, 3);
    BoxBlur(image, *output, (boxes[0] - 1) / 2);
    BoxBlur(*output, image, (boxes[1] - 1) / 2);
    BoxBlur(image, *output, (boxes[2] - 1) / 2);
    delete[] boxes;
    return output; 
}

int* GuassianFilter::CreateBoxesForGuass(double sigma, int boxCount)
{
    double wIdeal = sqrt((12 * sigma * sigma / boxCount) + 1);
    int    wl     = (int)floor(wIdeal);

    if (wl % 2 == 0) {
        wl--;
    }

    int    wu     = wl + 2;
    double mIdeal = (12 * sigma * sigma - boxCount * wl * wl - 4 * boxCount * wl - 3 * boxCount) / (-4 * wl - 4);
    int    m      = round(mIdeal);

    int* sizes = new int[boxCount];
    for (int i = 0; i < boxCount; i++) {
        sizes[i] = i < m ? wl : wu;
    }
    return sizes;
}

void GuassianFilter::BoxBlur(Image& source, Image& output, int radius)
{
    // assumption made that source.size == output.size
    memcpy(output.GetPixels(), source.GetPixels(), source.Width() * source.Height() * 4);
    BoxBlurH(output, source, radius);
    BoxBlurT(source, output, radius);
}

void GuassianFilter::BoxBlurH(Image& source, Image& output, int radius)
{
    auto   height = source.Height();
    auto   width = source.Width();
    double iarr = (double)1 / (radius + radius + 1);
    for (int i = 0; i < height; i++) {
        int ti = i * width;
        int li = ti;
        int ri = ti + radius;
        struct Color fv = source.GetColor(ti);
        struct Color lv = source.GetColor(ti + width - 1);

        unsigned int currentR = fv.r * (radius + 1);
        unsigned int currentG = fv.g * (radius + 1);
        unsigned int currentB = fv.b * (radius + 1);

        for (int j = 0; j < radius; j++) {
            struct Color pixel = source.GetColor(ti + j);
            currentR += pixel.r;
            currentG += pixel.g;
            currentB += pixel.b;
        }

        for (int j = 0; j <= radius; j++) {
            struct Color pixel = source.GetColor(ri++);
            currentR += (pixel.r - fv.r);
            currentG += (pixel.g - fv.g);
            currentB += (pixel.b - fv.b);

            output.SetColor(ti++,
                std::clamp((int)(currentR * iarr), 0, 255), 
                std::clamp((int)(currentG * iarr), 0, 255), 
                std::clamp((int)(currentB * iarr), 0, 255));
        }

        for (int j = radius + 1; j < width - radius; j++) {
            struct Color first_pixel = source.GetColor(ri++);
            struct Color second_pixle = source.GetColor(li++);

            currentR += (first_pixel.r - second_pixle.r);
            currentG += (first_pixel.g - second_pixle.g);
            currentB += (first_pixel.b - second_pixle.b);

            output.SetColor(ti++,
                std::clamp((int)(currentR * iarr), 0, 255), 
                std::clamp((int)(currentG * iarr), 0, 255), 
                std::clamp((int)(currentB * iarr), 0, 255));
        }

        for (int j = width - radius; j < width; j++) {
            struct Color pixel = source.GetColor(li++);

            currentR += (lv.r - pixel.r);
            currentG += (lv.g - pixel.g);
            currentB += (lv.b - pixel.b);

            output.SetColor(ti++,
                std::clamp((int)(currentR * iarr), 0, 255), 
                std::clamp((int)(currentG * iarr), 0, 255), 
                std::clamp((int)(currentB * iarr), 0, 255));
        }
    }
}

void GuassianFilter::BoxBlurT(Image& source, Image& output, int radius)
{
    auto   height = source.Height();
    auto   width = source.Width();
    double iarr = (double)1 / (radius + radius + 1);
    for (int i = 0; i < width; i++) {
        int ti = i;
        int li = ti;
        int ri = ti + radius * width;

        struct Color fv = source.GetColor(ti);
        struct Color lv = source.GetColor(ti + width * (height - 1));

        unsigned currentR = fv.r * (radius + 1);
        unsigned currentG = fv.g * (radius + 1);
        unsigned currentB = fv.b * (radius + 1);

        for (int j = 0; j < radius; j++) {
            struct Color pixel = source.GetColor(ti + j * width);
            currentR += pixel.r;
            currentG += pixel.g;
            currentB += pixel.b;
        }

        for (int j = 0; j <= radius; j++) {
            struct Color pixel = source.GetColor(ri);
            currentR += (pixel.r - fv.r);
            currentG += (pixel.g - fv.g);
            currentB += (pixel.b - fv.b);

            output.SetColor(ti,
                std::clamp((int)(currentR * iarr), 0, 255), 
                std::clamp((int)(currentG * iarr), 0, 255), 
                std::clamp((int)(currentB * iarr), 0, 255));

            ri += width;
            ti += width;
        }

        for (int j = radius + 1; j < height - radius; j++) {
            struct Color first_pixel = source.GetColor(ri);
            struct Color second_pixle = source.GetColor(li);

            currentR += (first_pixel.r - second_pixle.r);
            currentG += (first_pixel.g - second_pixle.g);
            currentB += (first_pixel.b - second_pixle.b);

            output.SetColor(ti,
                std::clamp((int)(currentR * iarr), 0, 255), 
                std::clamp((int)(currentG * iarr), 0, 255), 
                std::clamp((int)(currentB * iarr), 0, 255));

            li += width;
            ri += width;
            ti += width;
        }

        for (int j = height - radius; j < height; j++) {
            struct Color pixel = source.GetColor(li);

            currentR += (lv.r + pixel.r);
            currentG += (lv.g + pixel.g);
            currentB += (lv.b + pixel.b);

            output.SetColor(ti,
                std::clamp((int)(currentR * iarr), 0, 255), 
                std::clamp((int)(currentG * iarr), 0, 255), 
                std::clamp((int)(currentB * iarr), 0, 255));

            li += width;
            ti += width;
        }
    }
}

int main()
{
    Image image(1920, 1080);
    GuassianFilter filter;

    auto start = std::chrono::high_resolution_clock::now();
    filter.Apply(image, 3.0);
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed time: " << elapsed.count() << "ms";
    return 0;
}
