#ifndef MULTI_ARRAY_H
#define MULTI_ARRAY_H

#include <array>
#include <stdexcept>
#include <stdint.h>
#include <vector>

using namespace std;

template <typename T, size_t N>
class ArrayND {
protected:
	// It's turtles all the way down
	array<size_t, N> size;
	vector<T> data;

	size_t calculate_index(array<size_t, N> indices) const {
		for (size_t i = 0; i < N; i++)
			if (indices[i] >= size[i])
				throw out_of_range("Index out of bounds");

		size_t index = 0;
		size_t multiplier = 1;
		for (ssize_t i = N - 1; i >= 0; i--) {
			index += indices[i] * multiplier;
			multiplier *= size[i];
		}
		return index;
	}
public:
	template <typename... Sizes>
	ArrayND(Sizes... sizes) {
		static_assert(sizeof...(sizes) == N, "Number of sizes must match N");
		array<size_t, N> size_array = {static_cast<size_t>(sizes)...};
		size_t length = 1;
		for (size_t i = 0; i < N; i++) {
			size[i] = size_array[i];
			length *= size[i];
		}
		data.resize(length, T{});
	}

	void fill(const T& value) {
		for (size_t i = 0; i < data.size(); i++)
			data[i] = value;
	}

	size_t getSize(size_t i) const {
		if (i >= N)
			throw out_of_range("Index out of bounds");
		return size[i];
	}

	template <typename... Indices>
	T& operator()(Indices... indices) {
		static_assert(sizeof...(indices) == N, "Number of indices must match N");
		array<size_t, N> idx_array = {static_cast<size_t>(indices)...};
		return data[calculate_index(idx_array)];
	}

	template <typename... Indices>
	const T& operator()(Indices... indices) const {
		static_assert(sizeof...(indices) == N, "Number of indices must match N");
		array<size_t, N> idx_array = {static_cast<size_t>(indices)...};
		return data[calculate_index(idx_array)];
	}
};

struct vec2 {
	int i, j;
	vec2() : i(0), j(0) {}
	vec2(int i, int j) : i(i), j(j) {}
	uint32_t height() const {
		return static_cast<uint32_t>(i);
	}
	uint32_t width() const {
		return static_cast<uint32_t>(j);
	}
	vec2 operator+(const vec2& other) const {
		return vec2(i + other.i, j + other.j);
	}
	vec2 operator-(const vec2& other) const {
		return vec2(i - other.i, j - other.j);
	}
	vec2 operator%(const vec2& other) const {
		return vec2(i % other.i, j % other.j);
	}
	bool inRange(const vec2& size) const {
		return i >= 0 && j >= 0 && i < size.i && j < size.j;
	}
};

template <typename T>
using Array2D = ArrayND<T, 2>;

template <typename T>
using Array3D = ArrayND<T, 3>;

template <typename T>
using Array4D = ArrayND<T, 4>;

#endif
