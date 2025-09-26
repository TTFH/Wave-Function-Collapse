#ifndef WAVE_H
#define WAVE_H

#include <random>
#include <stdint.h>
#include <vector>

#include "multi_array.h"
#include "propagator.h"

using namespace std;

struct Probability {
	double sum;
	double sum_log;
	double sum_plogp;
	double entropy;
	uint32_t remaining;
};

enum class ObserveStatus { FAILURE, CONTINUE, SUCCESS };

class Wave {
private:
	bool is_impossible;
	Array3D<uint8_t> data;
	const vector<double> patterns;
	const vector<double> plogp_patterns;
	const double min_abs_half_plogp;
	Array2D<Probability> probabilities;
public:
	const vec2 size;
	Wave(vec2 size, const vector<double>& patterns);
	bool get(vec2 index, uint32_t pattern) const;
	void set(vec2 index, uint32_t pattern, bool value);
	ObserveStatus getMinEntropy(minstd_rand& generator, vec2& argmin) const;
	void init();
};

#endif
