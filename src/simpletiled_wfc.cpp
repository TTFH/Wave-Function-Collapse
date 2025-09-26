#include <stdexcept>

#include "simpletiled_wfc.h"

Tile::Tile(const Image& image, const Symmetry& symmetry, double weight)
	: images(symmetry.generateOrientations(image)), symmetry(symmetry), weight(weight) {}

Tile::Tile(const vector<Image>& images, const Symmetry& symmetry, double weight)
	: images(images), symmetry(symmetry), weight(weight) {}

static Array2D<uint32_t> generateActionMap(const Symmetry& symmetry) {
	uint32_t orientations = symmetry.orientations();
	vector<uint32_t> rotation_map = symmetry.rotationMap();
	vector<uint32_t> reflection_map = symmetry.reflectionMap();

	Array2D<uint32_t> action_map(8, orientations);
	for (uint32_t i = 0; i < 8; i++)
		for (uint32_t j = 0; j < orientations; j++)
			if (i == 0)
				action_map(0, j) = j;
			else if (i == 4)
				action_map(4, j) = reflection_map[action_map(0, j)];
			else
				action_map(i, j) = rotation_map[action_map(i - 1, j)];

	return action_map;
}

vector<PatternIndex> SimpletiledWFC::generatePatterns() {
	vector<PatternIndex> patterns;
	patterns.reserve(8 * tiles.size());
	for (uint32_t i = 0; i < tiles.size(); i++) {
		uint32_t orientations = tiles[i].images.size();
		for (uint32_t j = 0; j < orientations; j++)
			patterns.push_back({i, j});
	}
	return patterns;
}

vector<vector<uint32_t>> SimpletiledWFC::generatePatternIndices() {
	vector<vector<uint32_t>> pattern_indices;
	pattern_indices.resize(tiles.size());
	uint32_t linear_index = 0;
	for (uint32_t i = 0; i < tiles.size(); i++) {
		uint32_t orientations = tiles[i].images.size();
		for (uint32_t j = 0; j < orientations; j++) {
			pattern_indices[i].push_back(linear_index);
			linear_index++;
		}
	}
	return pattern_indices;
}

vector<double> SimpletiledWFC::computeWeights() const {
	vector<double> weights;
	weights.reserve(8 * tiles.size());
	for (uint32_t i = 0; i < tiles.size(); i++) {
		uint32_t orientations = tiles[i].images.size();
		for (uint32_t j = 0; j < orientations; j++)
			weights.push_back(tiles[i].weight / orientations);
	}
	return weights;
}

Image SimpletiledWFC::toImage(const Array2D<uint32_t>& output_patterns) const {
	uint32_t height = output_patterns.getSize(0);
	uint32_t width = output_patterns.getSize(1);
	uint32_t size = tiles[0].images[0].getHeight();
	Image output(height * size, width * size);
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < width; j++) {
			const PatternIndex& pattern = patterns[output_patterns(i, j)];
			for (uint32_t dy = 0; dy < size; dy++)
				for (uint32_t dx = 0; dx < size; dx++)
					output(i * size + dy, j * size + dx) =
						tiles[pattern.tile_index].images[pattern.image_index](dy, dx);
		}
	}
	return output;
}

PropagatorState SimpletiledWFC::generatePropagator(const vector<NeighborIndex>& neighbors) const {
	uint32_t pattern_count = patterns.size();
	Array3D<uint8_t> dense_propagator(Propagator::DIRECTIONS, pattern_count, pattern_count);
	dense_propagator.fill(false);

	for (vector<NeighborIndex>::const_iterator it = neighbors.begin(); it != neighbors.end(); it++) {
		Array2D<uint32_t> action_map1 = generateActionMap(tiles[it->left_index].symmetry);
		Array2D<uint32_t> action_map2 = generateActionMap(tiles[it->right_index].symmetry);

		auto add = [&](uint32_t action, uint32_t dir) {
			uint32_t orientation1 = action_map1(action, it->left_orientation);
			uint32_t orientation2 = action_map2(action, it->right_orientation);
			uint32_t tile_index1 = pattern_indices[it->left_index][orientation1];
			uint32_t tile_index2 = pattern_indices[it->right_index][orientation2];
			dense_propagator(dir, tile_index1, tile_index2) = true;
			dense_propagator(Propagator::Opposite[dir], tile_index2, tile_index1) = true;
		};

		add(0, 2);
		add(1, 0);
		add(2, 1);
		add(3, 3);
		add(4, 1);
		add(5, 3);
		add(6, 2);
		add(7, 0);
	}

	PropagatorState propagator(Propagator::DIRECTIONS, pattern_count);
	for (uint32_t dir = 0; dir < Propagator::DIRECTIONS; dir++)
		for (uint32_t i = 0; i < pattern_count; i++)
			for (uint32_t j = 0; j < pattern_count; j++)
				if (dense_propagator(dir, i, j))
					propagator(dir, i).push_back(j);

	return propagator;
}

SimpletiledWFC::SimpletiledWFC(vec2 size, const vector<Tile>& tiles, const vector<NeighborIndex>& neighbors,
							   const WFCOptions& options)
	: tiles(tiles), options(options), patterns(generatePatterns()), pattern_indices(generatePatternIndices()),
	  wfc(size, generatePropagator(neighbors), computeWeights(), options.periodic_output) {}

void SimpletiledWFC::setTile(vec2 index, uint32_t pattern, uint32_t orientation) {
	if (pattern >= pattern_indices.size() || orientation >= pattern_indices[pattern].size())
		throw out_of_range("Tile index or orientation out of range");
	uint32_t pattern_index = pattern_indices[pattern][orientation];
	for (uint32_t p = 0; p < patterns.size(); p++)
		if (p != pattern_index)
			wfc.collapse(index, pattern);
}

optional<Image> SimpletiledWFC::execute(int seed) {
	wfc.init();
	optional<Array2D<uint32_t>> result = wfc.execute(seed);
	if (result.has_value())
		return toImage(result.value());
	return nullopt;
}
