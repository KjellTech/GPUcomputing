#pragma once
#include <Windows.h>
#include <string>
#include "stdlib.h"

class Stopwatch {
public:
	Stopwatch(const char *function) : function_(function) {
		QueryPerformanceCounter((LARGE_INTEGER*)&startCounter_);
	}
	~Stopwatch() {
		__int64        stopCounter;
		QueryPerformanceCounter((LARGE_INTEGER*)&stopCounter);
		__int64 frequency;
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
		__int64 elapsedCounts = stopCounter - startCounter_;
		__int64 ms = (1000 * elapsedCounts) / frequency;

		printf("Ms spent in %s: %d\n", function_.c_str(), ms);
	}
private:
	std::string function_;
	__int64       startCounter_;
};