#ifndef PROPAGATOR_H
#define PROPAGATOR_H

#include <stack>
#include <stdint.h>
#include <vector>

#include "multi_array.h"
#include "wave.h"

using namespace std;

class Wave;

typedef Array2D<vector<uint32_t>> PropagatorState;

class Propagator {
private:
	struct Position {
		vec2 index;
		uint32_t pattern;
		Position(vec2 index, uint32_t pattern) : index(index), pattern(pattern) {}
	};
	const vec2 size;
	const bool periodic_output;
	const PropagatorState state;
	Array4D<uint32_t> compatible;
	stack<Position> propagating;
public:
	inline static const uint32_t DIRECTIONS = 4;
	inline static const vec2 DIRECTION[] = {vec2(-1, 0), vec2(0, -1), vec2(0, 1), vec2(1, 0)};
	inline static const uint32_t Opposite[] = {3, 2, 1, 0};
	Propagator(vec2 size, const PropagatorState& state, bool periodic_output);
	void pushPattern(vec2 index, uint32_t pattern);
	void propagate(Wave& wave);
	void init();
};

#endif
