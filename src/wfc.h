#ifndef WFC_H
#define WFC_H

#include <optional>
#include <random>
#include <stdint.h>

#include "image.h"
#include "multi_array.h"
#include "propagator.h"
#include "wave.h"

using namespace std;

struct WFCOptions {
	bool periodic_output;
};

class WFC {
private:
	const vector<double> patterns; // Normalized before wave initialization
	Wave wave;
	Propagator propagator;
	minstd_rand generator;
	ObserveStatus observe();
	Array2D<uint32_t> toOutput() const;
public:
	WFC(vec2 size, const PropagatorState& state, const vector<double>& patterns, bool periodic_output);
	optional<Array2D<uint32_t>> execute(int seed);
	void propagate();
	void collapse(vec2 index, uint32_t pattern);
	void init();
};

#endif
