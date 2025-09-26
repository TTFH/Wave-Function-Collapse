#include <limits>
#include <stdexcept>

#include "wfc.h"

static vector<double> normalize(const vector<double>& distribution) {
	double sum_weights = 0.0;
	for (uint32_t i = 0; i < distribution.size(); i++)
		sum_weights += distribution[i];

	if (sum_weights == 0)
		throw runtime_error("Can't normalize vector of all zeroes");

	double inv_sum_weights = 1.0 / sum_weights;
	vector<double> normalized(distribution.size());
	for (uint32_t i = 0; i < distribution.size(); i++)
		normalized[i] = distribution[i] * inv_sum_weights;
	return normalized;
}

ObserveStatus WFC::observe() {
	vec2 argmin;
	ObserveStatus status = wave.getMinEntropy(generator, argmin);
	if (status != ObserveStatus::CONTINUE)
		return status;

	double sum = 0;
	for (uint32_t p = 0; p < patterns.size(); p++)
		sum += wave.get(argmin, p) ? patterns[p] : 0;

	uniform_real_distribution<double> distribution(0, sum);
	double random_value = distribution(generator);
	uint32_t chosen_value = patterns.size() - 1;

	for (uint32_t p = 0; p < patterns.size(); p++) {
		random_value -= wave.get(argmin, p) ? patterns[p] : 0;
		if (random_value <= 0) {
			chosen_value = p;
			break;
		}
	}

	for (uint32_t p = 0; p < patterns.size(); p++) {
		if (wave.get(argmin, p) ^ (p == chosen_value)) {
			propagator.pushPattern(argmin, p);
			wave.set(argmin, p, false);
		}
	}
	return ObserveStatus::CONTINUE;
}

Array2D<uint32_t> WFC::toOutput() const {
	Array2D<uint32_t> output(wave.size.height(), wave.size.width());
	for (uint32_t i = 0; i < wave.size.height(); i++)
		for (uint32_t j = 0; j < wave.size.width(); j++)
			for (uint32_t p = 0; p < patterns.size(); p++)
				if (wave.get(vec2(i, j), p))
					output(i, j) = p;
	return output;
}

WFC::WFC(vec2 size, const PropagatorState& state, const vector<double>& patterns, bool periodic_output)
	: patterns(normalize(patterns)), wave(size, this->patterns), propagator(size, state, periodic_output) {}

optional<Array2D<uint32_t>> WFC::execute(int seed) {
	generator = minstd_rand(seed);
	while (true) {
		ObserveStatus result = observe();
		if (result == ObserveStatus::SUCCESS)
			return toOutput();
		if (result == ObserveStatus::FAILURE)
			return nullopt;
		propagator.propagate(wave);
	}
}

void WFC::propagate() {
	propagator.propagate(wave);
}

void WFC::collapse(vec2 index, uint32_t pattern) {
	if (wave.get(index, pattern)) {
		wave.set(index, pattern, false);
		propagator.pushPattern(index, pattern);
	}
}

void WFC::init() {
	// Finish initialization, reset values for next execution
	wave.init();
	propagator.init();
}
