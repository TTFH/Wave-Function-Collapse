#ifndef OVERLAPPING_WFC_H
#define OVERLAPPING_WFC_H

#include <optional>
#include <stdint.h>
#include <vector>

#include "image.h"
#include "wfc.h"

using namespace std;

struct OverlappingWFCOptions : WFCOptions {
	bool ground;
	bool periodic_input;
	vec2 out_size;
	uint32_t symmetry;
	uint32_t pattern_size;
	vec2 getWaveSize() const;
};

class OverlappingWFC {
private:
	const Image input;
	const vector<Image> patterns;
	const OverlappingWFCOptions options;
	WFC wfc;
	void initGround();
	PropagatorState generatePropagator() const;
	Image toImage(const Array2D<uint32_t>& output_patterns) const;
	OverlappingWFC(const Image& input, const OverlappingWFCOptions& options,
				   const pair<vector<Image>, vector<double>>& patterns_weights);
public:
	OverlappingWFC(const Image& input, const OverlappingWFCOptions& options);
	optional<Image> execute(int seed);
};

#endif
