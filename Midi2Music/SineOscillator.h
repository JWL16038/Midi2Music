#pragma once


class SineOscillator {
	float frequency, amplitude, angle = 0.0f, offset = 0.0f;

public:
	SineOscillator(float freq, float amp);
	float process();
};