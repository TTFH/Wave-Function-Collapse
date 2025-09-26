#include <stdexcept>

#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"

bool RGB::operator==(const RGB& c) const {
	return r == c.r && g == c.g && b == c.b;
}

bool RGB::operator!=(const RGB& c) const {
	return !(c == *this);
}

Image::Image(size_t height, size_t width) : Array2D<RGB>(height, width) {}

size_t Image::height() const {
	return size[0];
}

size_t Image::width() const {
	return size[1];
}

size_t Image::getHeight() const {
	return height();
}

size_t Image::getWidth() const {
	return width();
}

const RGB* Image::rawData() const {
	return data.data();
}

Image Image::rotate() const {
	if (height() != width())
		throw logic_error("Image to rotate is not square");
	Image result(height(), width());
	for (size_t i = 0; i < height(); i++)
		for (size_t j = 0; j < width(); j++)
			result(j, i) = (*this)(i, width() - 1 - j);
	return result;
}

Image Image::mirror() const {
	Image result(height(), width());
	if (height() != width())
		throw logic_error("Image to mirror is not square");
	for (size_t i = 0; i < height(); i++)
		for (size_t j = 0; j < width(); j++)
			result(i, j) = (*this)(i, width() - 1 - j);
	return result;
}

Image Image::subImage(size_t i0, size_t j0, size_t height, size_t width) const {
	if (height != width)
		throw logic_error("Sub image size is not square");
	Image result(height, width);
	for (size_t i = 0; i < height; i++) {
		for (size_t j = 0; j < width; j++) {
			size_t src_i = (i + i0) % this->height();
			size_t src_j = (j + j0) % this->width();
			result(i, j) = (*this)(src_i, src_j);
		}
	}
	return result;
}

bool Image::operator==(const Image& image) const {
	if (height() != image.height() || width() != image.width())
		return false;
	for (size_t i = 0; i < data.size(); i++)
		if (image.data[i] != data[i])
			return false;
	return true;
}

size_t RGBHash::operator()(const RGB& color) const {
	return static_cast<size_t>(color.r) << 16 | static_cast<size_t>(color.g) << 8 | static_cast<size_t>(color.b);
}

size_t ImageHash::operator()(const Image& image) const {
	size_t hash = 0;
	const RGB* data = image.rawData();
	size_t length = image.getHeight() * image.getWidth();
	for (size_t i = 0; i < length; i++) {
		const RGB& pixel = data[i];
		hash ^= RGBHash()(pixel) + 0x9E3779B9 + (hash << 6) + (hash >> 2);
	}
	return hash;
}

Image LoadImage(const string& path) {
	int width, height, num_components;
	stbi_set_flip_vertically_on_load(false);
	uint8_t* data = stbi_load(path.c_str(), &width, &height, &num_components, STBI_rgb);
	if (data == nullptr)
		throw runtime_error("Failed to load image: " + path);
	Image image = Image(height, width);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			uint32_t index = STBI_rgb * (i * width + j);
			RGB pixel = {data[index], data[index + 1], data[index + 2]};
			image(i, j) = pixel;
		}
	}
	stbi_image_free(data);
	return image;
}

void SaveImagePNG(const string& path, const Image& image) {
	stbi_flip_vertically_on_write(false);
	stbi_write_png(path.c_str(), image.getWidth(), image.getHeight(), STBI_rgb,
				   reinterpret_cast<const uint8_t*>(image.rawData()), 0);
}
