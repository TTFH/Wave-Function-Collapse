#include "wave.h"

#include <limits>
#include <math.h>

static vector<double> calculate_plogp(const vector<double>& distribution) {
	vector<double> plogp;
	for (uint32_t i = 0; i < distribution.size(); i++) {
		double p = distribution[i];
		plogp.push_back(p * log(p));
	}
	return plogp;
}

static double calculate_min_abs_half(const vector<double>& distribution) {
	double min_abs_half = numeric_limits<double>::infinity();
	for (uint32_t i = 0; i < distribution.size(); i++)
		min_abs_half = min(min_abs_half, abs(distribution[i] / 2.0));
	return min_abs_half;
}

void Wave::init() {
	is_impossible = false;
	data.fill(true);

	double base_sum = 0;
	double base_entropy = 0;
	for (uint32_t i = 0; i < patterns.size(); i++) {
		base_sum += patterns[i];
		base_entropy += plogp_patterns[i];
	}
	double base_sum_log = log(base_sum);
	double entropy_base = base_sum_log - base_entropy / base_sum;

	Probability probability;
	probability.sum = base_sum;
	probability.sum_log = base_sum_log;
	probability.sum_plogp = base_entropy;
	probability.entropy = entropy_base;
	probability.remaining = patterns.size();
	probabilities.fill(probability);
}

Wave::Wave(vec2 size, const vector<double>& patterns)
	: data(size.height(), size.width(), patterns.size()), patterns(patterns), plogp_patterns(calculate_plogp(patterns)),
	  min_abs_half_plogp(calculate_min_abs_half(plogp_patterns)), probabilities(size.height(), size.width()),
	  size(size) {}

bool Wave::get(vec2 index, uint32_t pattern) const {
	return data(index.i, index.j, pattern);
}

void Wave::set(vec2 index, uint32_t pattern, bool value) {
	bool prev_value = data(index.i, index.j, pattern);
	if (prev_value == value)
		return;
	data(index.i, index.j, pattern) = value;

	Probability& probability = probabilities(index.i, index.j);
	probability.sum -= patterns[pattern];
	probability.sum_log = log(probability.sum);
	probability.sum_plogp -= plogp_patterns[pattern];
	probability.entropy = probability.sum_log - probability.sum_plogp / probability.sum;
	probability.remaining--;
	if (probability.remaining == 0)
		is_impossible = true;
}

ObserveStatus Wave::getMinEntropy(minstd_rand& generator, vec2& argmin) const {
	if (is_impossible)
		return ObserveStatus::FAILURE;

	bool all_collapsed = true;
	double min = numeric_limits<double>::infinity();
	uniform_real_distribution<double> distribution(0, min_abs_half_plogp);

	for (uint32_t i = 0; i < size.height(); i++) {
		for (uint32_t j = 0; j < size.width(); j++) {
			if (probabilities(i, j).remaining == 1)
				continue;

			double entropy = probabilities(i, j).entropy;
			if (entropy <= min) {
				double noise = distribution(generator);
				if (entropy + noise < min) {
					min = entropy + noise;
					all_collapsed = false;
					argmin = vec2(i, j);
				}
			}
		}
	}
	return all_collapsed ? ObserveStatus::SUCCESS : ObserveStatus::CONTINUE;
}
