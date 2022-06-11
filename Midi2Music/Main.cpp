#define _USE_MATH_DEFINES 

#include <iostream>
#include <fstream>
#include "MidiFile.h"
#include "WavGenerator.h"
#include "SineOscillator.h"

int main() {
	midi::MidiFile midiFile;
	midiFile.ParseFile("bach.mid");

	std::cout << "===============================" << std::endl;
	std::cout << "Number of tracks: " << midiFile.vecTracks.size() << std::endl;
	std::cout << "===============================" << std::endl;



	for (auto& track : midiFile.vecTracks) {
		if (!track.vecNotes.empty()) {
			uint32_t nNoteRange = track.nMaxNote - track.nMinNote;
			for (auto& note : track.vecNotes) {
				
				auto maxAmplitude = pow(2, BIT_DEPTH - 1) - 1;
				for (int i = 0; i < SAMPLE_RATE * duration; i++) {
					SineOscillator sineOscillator(440, 0.5);
					auto sample = sineOscillator.process();
					int intSample = static_cast<int> (sample * maxAmplitude);
					//writeToFile(audioFile, intSample, 2);
				}
			}

		}
	}

	//WavGenerator wav(2);
	return 0;
}

