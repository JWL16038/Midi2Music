#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "Synthesizer.h"

// General purpose oscillator
#define OSC_SINE 0
#define OSC_SQUARE 1
#define OSC_TRIANGLE 2
#define OSC_SAW_ANA 3
#define OSC_SAW_DIG 4

// Global synthesizer variables
synthesizer::sEnvelopeADSR envelope;							// amplitude modulation of output to give texture, i.e. the timbre
atomic<double> frequency = 0.0;
double dOctaveBaseFrequency = 110.0; // A2		// frequency of octave represented by keyboard
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatve


double synthesizer::w(double hertz) {
	return hertz * 2.0 * PI;
}

double synthesizer::osc(double dHertz, double dTime, int nType) {
	switch (nType)
	{
	case OSC_SINE: // Sine wave bewteen -1 and +1
		return sin(w(dHertz) * dTime);

	case OSC_SQUARE: // Square wave between -1 and +1
		return sin(w(dHertz) * dTime) > 0 ? 1.0 : -1.0;

	case OSC_TRIANGLE: // Triangle wave between -1 and +1
		return asin(sin(w(dHertz) * dTime)) * (2.0 / PI);

	case OSC_SAW_ANA: // Saw wave (analogue / warm / slow)
	{
		double dOutput = 0.0;

		for (double n = 1.0; n < 40.0; n++)
			dOutput += (sin(n * w(dHertz) * dTime)) / n;

		return dOutput * (2.0 / PI);
	}
	case OSC_SAW_DIG: // Saw Wave (optimised / harsh / fast)
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));

	default:
		return 0.0;
	}
}

double synthesizer::MakeNoise(double dTime) {
	// Mix together a little sine and square waves
	double dOutput = envelope.GetAmplitude(dTime) *
		(+1.0 * osc(frequency * 0.5, dTime, OSC_SINE) + 1.0 * osc(frequency, dTime, OSC_SAW_ANA));

	return dOutput * 0.4; // Master Volume
}