#include <assert.h>
#include <stdexcept>

#include "imagemosaic_wfc.h"

ImageWeight::ImageWeight(const Image& image, double weight) : image(image), weight(weight) {}

vector<uint32_t> ImagemosaicWFC::generatePatterns() {
	vector<uint32_t> patterns;
	patterns.resize(tiles.size());
	for (uint32_t i = 0; i < tiles.size(); i++)
		patterns[i] = i;
	return patterns;
}

vector<double> ImagemosaicWFC::computeWeights() const {
	vector<double> weights(tiles.size());
	for (uint32_t i = 0; i < tiles.size(); i++)
		weights[i] = tiles[i].weight;
	return weights;
}

Image ImagemosaicWFC::toImage(const Array2D<uint32_t>& output_patterns) const {
	uint32_t height = output_patterns.getSize(0);
	uint32_t width = output_patterns.getSize(1);
	uint32_t size = tiles[0].image.getHeight();
	Image output(height * size, width * size);
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < width; j++) {
			uint32_t pattern = patterns[output_patterns(i, j)];
			for (uint32_t dy = 0; dy < size; dy++)
				for (uint32_t dx = 0; dx < size; dx++)
					output(i * size + dy, j * size + dx) = tiles[pattern].image(dy, dx);
		}
	}
	return output;
}

PropagatorState ImagemosaicWFC::generatePropagator(const Array3D<uint8_t>& neighbors) const {
	uint32_t tile_count = tiles.size();
	PropagatorState state(Propagator::DIRECTIONS, tile_count);
	for (uint32_t dir = 0; dir < Propagator::DIRECTIONS; dir++)
		for (uint32_t i = 0; i < tile_count; i++)
			for (uint32_t j = 0; j < tile_count; j++)
				if (neighbors(dir, i, j) || neighbors(Propagator::Opposite[dir], j, i)) {
					if (!neighbors(dir, i, j))
						printf("[Warning] Missing neighbor %d-%d (x=%d, y=%d)\n", i, j, Propagator::DIRECTION[dir].j,
							   -Propagator::DIRECTION[dir].i);
					state(dir, i).push_back(j);
				}
	return state;
}

ImagemosaicWFC::ImagemosaicWFC(vec2 size, const vector<ImageWeight>& tiles, const Array3D<uint8_t>& neighbors,
							   const WFCOptions& options)
	: options(options), tiles(tiles), patterns(generatePatterns()),
	  wfc(size, generatePropagator(neighbors), computeWeights(), options.periodic_output) {}

void ImagemosaicWFC::setTile(vec2 index, uint32_t pattern) {
	for (uint32_t p = 0; p < patterns.size(); p++)
		if (pattern != p)
			wfc.collapse(index, p);
}

optional<Image> ImagemosaicWFC::execute(int seed) {
	wfc.init();
	optional<Array2D<uint32_t>> result = wfc.execute(seed);
	if (result.has_value())
		return toImage(result.value());
	return nullopt;
}
