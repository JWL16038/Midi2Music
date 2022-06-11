#define  _USE_MATH_DEFINES
#include <cmath>
#include "SineOscillator.h"
#include "WavGenerator.h"


SineOscillator::SineOscillator(float freq, float amp) : frequency(freq), amplitude(amp) {
	offset = 2 * M_PI * frequency / SAMPLE_RATE;
}


float SineOscillator::process() {
	auto sample =  amplitude * sin(angle);
	angle += offset;
	return sample;
}

