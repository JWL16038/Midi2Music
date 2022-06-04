#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include "MidiFile.cpp"


int main() {

	File::MidiFile midi;
	midi.ParseFile("bach.mid");



	return 0;
}