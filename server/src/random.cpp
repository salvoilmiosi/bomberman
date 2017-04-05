#include "random.h"

#include <chrono>

std::default_random_engine rng;

void init_rng() {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	rng.seed(seed);
}

int rand_num(int max) {
	return rng() % max;
}