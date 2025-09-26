#include "symmetry.h"

Symmetry::Symmetry(char type) {
	switch (type) {
	case 'X':
		value = X;
		break;
	case 'T':
		value = T;
		break;
	case 'I':
		value = I;
		break;
	case 'L':
		value = L;
		break;
	case '\\':
		value = backslash;
		break;
	case 'F':
	default:
		value = F;
		break;
	}
}

uint32_t Symmetry::orientations() const {
	switch (value) {
	case X:
		return 1;
	case I:
	case backslash:
		return 2;
	case T:
	case L:
		return 4;
	case F:
	default:
		return 8;
	}
}

vector<uint32_t> Symmetry::rotationMap() const {
	switch (value) {
	case X:
		return {0};
	case I:
	case backslash:
		return {1, 0};
	case T:
	case L:
		return {1, 2, 3, 0};
	case F:
	default:
		return {1, 2, 3, 0, 5, 6, 7, 4};
	}
}

vector<uint32_t> Symmetry::reflectionMap() const {
	switch (value) {
	case X:
		return {0};
	case I:
		return {0, 1};
	case backslash:
		return {1, 0};
	case T:
		return {0, 3, 2, 1};
	case L:
		return {1, 0, 3, 2};
	case F:
	default:
		return {4, 7, 6, 5, 0, 3, 2, 1};
	}
}

vector<Image> Symmetry::generateOrientations(const Image& input) const {
	Image image = input;
	vector<Image> oriented;
	oriented.reserve(8);
	oriented.push_back(image);

	switch (value) {
	case X:
		break;
	case I:
	case backslash:
		oriented.push_back(image.rotate());
		break;
	case T:
	case L:
		oriented.push_back(image = image.rotate());
		oriented.push_back(image = image.rotate());
		oriented.push_back(image = image.rotate());
		break;
	case F:
	default:
		oriented.push_back(image = image.rotate());
		oriented.push_back(image = image.rotate());
		oriented.push_back(image = image.rotate());
		oriented.push_back(image = image.rotate().mirror());
		oriented.push_back(image = image.rotate());
		oriented.push_back(image = image.rotate());
		oriented.push_back(image = image.rotate());
		break;
	}
	return oriented;
}
