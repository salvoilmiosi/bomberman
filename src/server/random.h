#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <vector>
#include <algorithm>
#include <random>

extern std::default_random_engine rng;

void init_rng();

int rand_num(int max);

template<typename T>
void rand_shuffle(T &v) {
	std::shuffle(v.begin(), v.end(), rng);
}

#endif // __RANDOM_H__
