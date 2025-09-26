#ifndef SYMMETRY_H
#define SYMMETRY_H

#include <stdint.h>
#include <vector>

#include "image.h"

using namespace std;

class Symmetry {
public:
	enum Value : char { X, T, I, L, backslash, F };
	Symmetry(char type);
	uint32_t orientations() const;
	vector<uint32_t> rotationMap() const;
	vector<uint32_t> reflectionMap() const;
	vector<Image> generateOrientations(const Image& input) const;
private:
	Value value;
};

#endif
