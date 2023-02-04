#pragma once
#include <ctime>


inline int randint(const unsigned int start, const unsigned int end)
{
	return rand() % (end - start) + start;
}

inline float randfloat(const float start, const float end)
{
	return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (end - start)) + start;
}