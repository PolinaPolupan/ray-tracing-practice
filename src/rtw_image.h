#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

#include <iostream>

class rtw_image {
  public:
    rtw_image() = default;

    explicit rtw_image(const char* image_filename);

    ~rtw_image();

    bool load(const std::string& filename);

    [[nodiscard]] int width()  const { return (fdata == nullptr) ? 0 : image_width; }
    [[nodiscard]] int height() const { return (fdata == nullptr) ? 0 : image_height; }

    [[nodiscard]] const unsigned char* pixelData(int x, int y) const;

  private:
    const int      bytes_per_pixel = 3;
    float         *fdata = nullptr;         // Linear floating point pixel data
    unsigned char *bdata = nullptr;         // Linear 8-bit pixel data
    int            image_width = 0;         // Loaded image width
    int            image_height = 0;        // Loaded image height
    int            bytes_per_scanline = 0;

    static int clamp(const int x, const int low, const int high) {
        // Return the value clamped to the range [low, high).
        if (x < low) return low;
        if (x < high) return x;
        return high - 1;
    }

    static unsigned char float_to_byte(const float value) {
        if (value <= 0.0)
            return 0;
        if (1.0 <= value)
            return 255;
        return static_cast<unsigned char>(256.0 * value);
    }

    void convert_to_bytes();
};

#endif