#include <iostream>
#include <fstream>
#include "WavGenerator.h"
#include "SineOscillator.h"


void WavGenerator::writeToFile(std::ofstream& file, int value, int size)
{
	file.write(reinterpret_cast<const char*> (&value), size);
}

WavGenerator::WavGenerator(int duration){
	std::ofstream audioFile;
	audioFile.open("output.wav", std::ios::binary);
	SineOscillator sineOscillator(440, 0.5);

	//Header chunk
	audioFile << "RIFF";
	audioFile << "----";
	audioFile << "WAVE";

	//Format chunk
	audioFile << "fmt ";
	writeToFile(audioFile, 16, 4);//size
	writeToFile(audioFile, 1, 2);//compression code
	writeToFile(audioFile, 1, 2);//Number of channels;
	writeToFile(audioFile, SAMPLE_RATE, 4);//sample rate
	writeToFile(audioFile, SAMPLE_RATE * BIT_DEPTH / 8, 4);//Byte rate
	writeToFile(audioFile, BIT_DEPTH / 8, 2);//Block align
	writeToFile(audioFile, BIT_DEPTH, 2);//Bit depth

	//Data chunk
	audioFile << "data";
	audioFile << "----";

	int preAudioPosition = audioFile.tellp();

	/*auto maxAmplitude = pow(2, BIT_DEPTH - 1) - 1;
	for (int i = 0; i < SAMPLE_RATE * duration; i++) {
		auto sample = sineOscillator.process();
		int intSample = static_cast<int> (sample * maxAmplitude);
		writeToFile(audioFile, intSample, 2);
	}*/

	int postAudioPosition = audioFile.tellp();

	audioFile.seekp(preAudioPosition - 4);
	writeToFile(audioFile, postAudioPosition - preAudioPosition, 4);

	audioFile.seekp(4, std::ios::beg);
	writeToFile(audioFile, postAudioPosition - 8, 4);

	audioFile.close();
}
