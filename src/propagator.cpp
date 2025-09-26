#include "propagator.h"

void Propagator::init() {
	compatible.fill(0);
	propagating = stack<Position>();
	uint32_t patterns_size = state.getSize(1);
	for (uint32_t dir = 0; dir < DIRECTIONS; dir++)
		for (uint32_t p = 0; p < patterns_size; p++) {
			uint32_t count = state(Opposite[dir], p).size();
			if (count != 0) {
				for (uint32_t i = 0; i < size.height(); i++)
					for (uint32_t j = 0; j < size.width(); j++)
						compatible(dir, p, i, j) = count;
			}
		}
}

Propagator::Propagator(vec2 size, const PropagatorState& state, bool periodic_output)
	: size(size), periodic_output(periodic_output), state(state),
	  compatible(DIRECTIONS, state.getSize(1), size.height(), size.width()) {}

void Propagator::pushPattern(vec2 index, uint32_t pattern) {
	for (uint32_t dir = 0; dir < DIRECTIONS; dir++)
		compatible(dir, pattern, index.i, index.j) = 0;
	propagating.emplace(Position(index, pattern));
}

void Propagator::propagate(Wave& wave) {
	while (!propagating.empty()) {
		Position input = propagating.top();
		propagating.pop();

		for (uint32_t dir = 0; dir < DIRECTIONS; dir++) {
			vec2 index;
			vec2 delta = DIRECTION[dir];
			if (periodic_output)
				index = (input.index + delta + wave.size) % wave.size;
			else {
				index = input.index + delta;
				if (!index.inRange(wave.size))
					continue;
			}
			const vector<uint32_t>& patterns = state(dir, input.pattern);
			for (vector<uint32_t>::const_iterator it = patterns.begin(); it != patterns.end(); it++) {
				uint32_t& value = compatible(dir, *it, index.i, index.j);
				value--;
				if (value == 0) {
					pushPattern(index, *it);
					wave.set(index, *it, false);
				}
			}
		}
	}
}
