#include <stdexcept>
#include <unordered_map>

#include "overlapping_wfc.h"

// If non periodic, don't access the last out_height % pattern_size
vec2 OverlappingWFCOptions::getWaveSize() const {
	return periodic_output ? out_size : out_size - vec2(pattern_size - 1, pattern_size - 1);
}

// Checks if the intersection of two images at distance (dy, dx) is the same
static bool agrees(const Image& pattern1, const Image& pattern2, const vec2& offset) {
	uint32_t y_min = offset.i < 0 ? 0 : offset.i;
	uint32_t y_max = offset.i < 0 ? offset.i + pattern2.getHeight() : pattern1.getHeight();
	uint32_t x_min = offset.j < 0 ? 0 : offset.j;
	uint32_t x_max = offset.j < 0 ? offset.j + pattern2.getWidth() : pattern1.getWidth();

	for (uint32_t i = y_min; i < y_max; i++)
		for (uint32_t j = x_min; j < x_max; j++)
			if (pattern1(i, j) != pattern2(i - offset.i, j - offset.j))
				return false;
	return true;
}

PropagatorState OverlappingWFC::generatePropagator() const {
	PropagatorState state = PropagatorState(Propagator::DIRECTIONS, patterns.size());
	for (uint32_t dir = 0; dir < Propagator::DIRECTIONS; dir++)
		for (uint32_t p1 = 0; p1 < patterns.size(); p1++)
			for (uint32_t p2 = 0; p2 < patterns.size(); p2++)
				if (agrees(patterns[p1], patterns[p2], Propagator::DIRECTION[dir]))
					state(dir, p1).push_back(p2);
	return state;
}

static vector<Image> GenerateSymmetries(const Image& image, uint32_t count) {
	vector<Image> symmetries(8, Image(image.getHeight(), image.getWidth()));
	symmetries[0] = image;
	if (count > 1)
		symmetries[1] = symmetries[0].mirror();
	if (count > 2)
		symmetries[2] = symmetries[0].rotate();
	if (count > 3)
		symmetries[3] = symmetries[2].mirror();
	if (count > 4)
		symmetries[4] = symmetries[2].rotate();
	if (count > 5)
		symmetries[5] = symmetries[4].mirror();
	if (count > 6)
		symmetries[6] = symmetries[4].rotate();
	if (count > 7)
		symmetries[7] = symmetries[6].mirror();
	return symmetries;
}

typedef unordered_map<Image, uint32_t, ImageHash> ImageMap;

static pair<vector<Image>, vector<double>> generatePatternsAndWeights(const Image& input,
																	  const OverlappingWFCOptions& options) {
	vector<Image> patterns;
	vector<double> weights;
	ImageMap unique_patterns;
	uint32_t max_i = options.periodic_input ? input.getHeight() : input.getHeight() - options.pattern_size + 1;
	uint32_t max_j = options.periodic_input ? input.getWidth() : input.getWidth() - options.pattern_size + 1;

	for (uint32_t i = 0; i < max_i; i++) {
		for (uint32_t j = 0; j < max_j; j++) {
			Image pattern = input.subImage(i, j, options.pattern_size, options.pattern_size);
			vector<Image> symmetries = GenerateSymmetries(pattern, options.symmetry);

			for (uint32_t k = 0; k < options.symmetry; k++) {
				pair<ImageMap::const_iterator, bool> res =
					unique_patterns.insert(make_pair(symmetries[k], patterns.size()));
				if (res.second) {
					patterns.push_back(symmetries[k]);
					weights.push_back(1);
				} else // duplicated image
					weights[res.first->second] += 1;
			}
		}
	}
	return make_pair(patterns, weights);
}

void OverlappingWFC::initGround() {
	optional<uint32_t> ground_index = nullopt;
	Image ground_pattern =
		input.subImage(input.getHeight() - 1, input.getWidth() / 2, options.pattern_size, options.pattern_size);

	for (uint32_t p = 0; p < patterns.size(); p++)
		if (patterns[p] == ground_pattern) {
			ground_index = p;
			break;
		}

	if (!ground_index.has_value())
		throw logic_error("Ground pattern not found in input image");

	// Mark bottom row as only ground
	for (uint32_t j = 0; j < options.getWaveSize().width(); j++)
		for (uint32_t p = 0; p < patterns.size(); p++)
			if (ground_index != p)
				wfc.collapse(vec2(options.getWaveSize().height() - 1, j), p);

	// Mark the remaining rows as not ground
	for (uint32_t i = 0; i < options.getWaveSize().height() - 1; i++)
		for (uint32_t j = 0; j < options.getWaveSize().width(); j++)
			wfc.collapse(vec2(i, j), ground_index.value());

	wfc.propagate();
}

Image OverlappingWFC::toImage(const Array2D<uint32_t>& output_patterns) const {
	Image output = Image(options.out_size.height(), options.out_size.width());
	for (uint32_t i = 0; i < options.getWaveSize().height(); i++)
		for (uint32_t j = 0; j < options.getWaveSize().width(); j++)
			output(i, j) = patterns[output_patterns(i, j)](0, 0);

	if (!options.periodic_output) {
		// The edges are not computed by the wave when it's non-periodic
		// Those are completed from the last tile
		uint32_t right_index = options.getWaveSize().width() - 1;
		uint32_t top_index = options.getWaveSize().height() - 1;

		// Set the right column of the image
		for (uint32_t i = 0; i < options.getWaveSize().height(); i++) {
			const Image& pattern = patterns[output_patterns(i, right_index)];
			for (uint32_t dx = 1; dx < options.pattern_size; dx++)
				output(i, right_index + dx) = pattern(0, dx);
		}
		// Set the bottom row of the image
		for (uint32_t j = 0; j < options.getWaveSize().width(); j++) {
			const Image& pattern = patterns[output_patterns(top_index, j)];
			for (uint32_t dy = 1; dy < options.pattern_size; dy++)
				output(top_index + dy, j) = pattern(dy, 0);
		}
		// Set the bottom-right corner of the image
		const Image& pattern = patterns[output_patterns(top_index, right_index)];
		for (uint32_t dy = 1; dy < options.pattern_size; dy++)
			for (uint32_t dx = 1; dx < options.pattern_size; dx++)
				output(top_index + dy, right_index + dx) = pattern(dy, dx);
	}
	return output;
}

OverlappingWFC::OverlappingWFC(const Image& input, const OverlappingWFCOptions& options,
							   const pair<vector<Image>, vector<double>>& patterns_weights)
	: input(input), patterns(patterns_weights.first), options(options),
	  wfc(options.getWaveSize(), generatePropagator(), patterns_weights.second, options.periodic_output) {}

OverlappingWFC::OverlappingWFC(const Image& input, const OverlappingWFCOptions& options)
	: OverlappingWFC(input, options, generatePatternsAndWeights(input, options)) {}

optional<Image> OverlappingWFC::execute(int seed) {
	wfc.init();
	if (options.ground)
		initGround();
	optional<Array2D<uint32_t>> result = wfc.execute(seed);
	if (result.has_value())
		return toImage(result.value());
	return nullopt;
}
