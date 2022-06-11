#pragma once
#include <iostream>
#include <vector>

	
class WavGenerator {
	#define SAMPLE_RATE 44100
	#define BIT_DEPTH 16

public:
	WavGenerator(int duration);
private:
	void writeToFile(std::ofstream& file, int value, int size);
};
