#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <string>

#include "multi_array.h"

using namespace std;

struct RGB {
	uint8_t r, g, b;
	bool operator==(const RGB& c) const;
	bool operator!=(const RGB& c) const;
};

class Image : public Array2D<RGB> {
private:
	size_t height() const;
	size_t width() const;
public:
	Image(size_t height, size_t width);
	size_t getHeight() const;
	size_t getWidth() const;
	const RGB* rawData() const;
	Image rotate() const;
	Image mirror() const;
	Image subImage(size_t i, size_t j, size_t height, size_t width) const;
	bool operator==(const Image& image) const;
};

struct RGBHash {
	size_t operator()(const RGB& color) const;
};

struct ImageHash {
	size_t operator()(const Image& image) const;
};

Image LoadImage(const string& path);
void SaveImagePNG(const string& path, const Image& image);

#endif
