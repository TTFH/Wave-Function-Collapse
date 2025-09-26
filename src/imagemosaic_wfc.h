#ifndef IMAGEMOSAICWFC_H
#define IMAGEMOSAICWFC_H

#include <optional>
#include <stdint.h>
#include <vector>

#include "image.h"
#include "multi_array.h"
#include "simpletiled_wfc.h"
#include "wfc.h"

using namespace std;

struct ImageWeight {
	Image image;
	double weight;
	ImageWeight(const Image& image, double weight);
};

class ImagemosaicWFC {
private:
	const WFCOptions options;
	const vector<ImageWeight> tiles;
	const vector<uint32_t> patterns;
	WFC wfc;
	vector<uint32_t> generatePatterns();
	vector<double> computeWeights() const;
	Image toImage(const Array2D<uint32_t>& output_patterns) const;
	PropagatorState generatePropagator(const Array3D<uint8_t>& neighbors) const;
public:
	ImagemosaicWFC(vec2 size, const vector<ImageWeight>& tiles, const Array3D<uint8_t>& neighbors,
				   const WFCOptions& options);
	void setTile(vec2 index, uint32_t pattern);
	optional<Image> execute(int seed);
};

#endif
