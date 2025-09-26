#ifndef SIMPLETILED_WFC_H
#define SIMPLETILED_WFC_H

#include <optional>
#include <stdint.h>
#include <vector>

#include "image.h"
#include "multi_array.h"
#include "symmetry.h"
#include "wfc.h"

using namespace std;

struct Tile {
	vector<Image> images;
	Symmetry symmetry;
	double weight;
	Tile(const Image& image, const Symmetry& symmetry, double weight);
	Tile(const vector<Image>& images, const Symmetry& symmetry, double weight);
};

struct PatternIndex {
	uint32_t tile_index;
	uint32_t image_index;
};

struct NeighborIndex {
	uint32_t left_index;
	uint32_t left_orientation;
	uint32_t right_index;
	uint32_t right_orientation;
};

class SimpletiledWFC {
private:
	const vector<Tile> tiles;
	const WFCOptions options;
	const vector<PatternIndex> patterns;
	const vector<vector<uint32_t>> pattern_indices;
	WFC wfc;
	vector<PatternIndex> generatePatterns();
	vector<vector<uint32_t>> generatePatternIndices();
	vector<double> computeWeights() const;
	Image toImage(const Array2D<uint32_t>& output_patterns) const;
	PropagatorState generatePropagator(const vector<NeighborIndex>& neighbors) const;
public:
	SimpletiledWFC(vec2 size, const vector<Tile>& tiles, const vector<NeighborIndex>& neighbors,
				   const WFCOptions& options);
	void setTile(vec2 index, uint32_t pattern, uint32_t orientation);
	optional<Image> execute(int seed);
};

#endif
