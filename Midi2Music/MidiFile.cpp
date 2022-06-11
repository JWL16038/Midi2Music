#include <iostream>
#include "MidiFile.h"

using namespace midi;

bool MidiFile::ParseFile(const std::string& sFileName) {
	std::ifstream inFile;
	inFile.open(sFileName, std::fstream::in | std::ios::binary);

	if (!inFile.is_open()) {
		std::cout << "Cannot find midi" << std::endl;
		return false;
	}
	std::cout << "Midi file open" << std::endl;

	auto Swap32 = [](uint32_t n) {
		return (((n >> 24) && 0xff | ((n << 8) & 0xff0000) | ((n >> 8) & 0xff00) | (n << 24) & 0xff000000));
	};
	auto Swap16 = [](uint16_t n) {
		return ((n >> 8) | (n << 8));
	};

	auto ReadString = [&inFile](uint32_t nLength) {
		std::string s;
		for (uint32_t i = 0; i < nLength; i++) {
			s += inFile.get();
		}
		return s;
	};

	auto ReadValue = [&inFile]() {
		uint32_t nValue = 0;
		uint8_t nByte = 0;

		nValue = inFile.get();
		if (nValue & 0x80) {
			nValue &= 0x7F;

			do {
				nByte = inFile.get();
				nValue = (nValue << 7 | nByte & 0x7F);
			} while (nByte & 0x80);
		}
		return nValue;
	};


	//Parse midi file

	uint32_t n32 = 0;
	uint16_t n16 = 0;

	inFile.read((char*)&n32, sizeof(uint32_t));
	uint32_t nFileID = Swap32(n32);
	inFile.read((char*)&n32, sizeof(uint32_t));
	uint32_t nHeaderLength = Swap32(n32);
	inFile.read((char*)&n16, sizeof(uint16_t));
	uint16_t nFormat = Swap16(n16);
	inFile.read((char*)&n16, sizeof(uint16_t));
	uint16_t nTrackChunks = Swap16(n16);//number of midi tracks this file contains
	inFile.read((char*)&n16, sizeof(uint16_t));
	uint16_t nDivision = Swap16(n16);

	for (uint16_t nChunk = 0; nChunk < nTrackChunks; nChunk++) {
		std::cout << "New track" << std::endl;
		inFile.read((char*)&n16, sizeof(uint16_t));
		uint32_t nTrackID = Swap32(n32);
		inFile.read((char*)&n16, sizeof(uint16_t));
		uint32_t nTrackLength = Swap32(n32);

		bool bEndOfTrack = false;
		int8_t nPreviousStatus = 0;

		vecTracks.push_back(MidiTrack());



		while (!inFile.eof() && !bEndOfTrack) {
			uint32_t nStatusTimeDelta = 0;
			uint8_t nStatus = 0;

			nStatusTimeDelta = ReadValue();
			nStatus = inFile.get();

			if (nStatus < 0x80) {
				nStatus = nPreviousStatus;
				inFile.seekg(-1, std::ios_base::cur);
			}

			if ((nStatus & 0xF0) == EventName::VoiceNoteOff) {
				nPreviousStatus = nStatus;
				uint8_t nChannel = nStatus & 0x0F;
				uint8_t nNoteID = inFile.get();
				uint8_t nNoteVelocity = inFile.get();
				vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::NoteOff,nNoteID,nNoteVelocity,nStatusTimeDelta });
			}
			else if ((nStatus & 0xF0) == EventName::VoiceNoteOn) {
				nPreviousStatus = nStatus;
				uint8_t nChannel = nStatus & 0x0F;
				uint8_t nNoteID = inFile.get();
				uint8_t nNoteVelocity = inFile.get();
				if (nNoteVelocity == 0) {
					vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::NoteOff,nNoteID,nNoteVelocity,nStatusTimeDelta });
				}
				else {
					vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::NoteOn,nNoteID,nNoteVelocity,nStatusTimeDelta });
				}
			}
			else if ((nStatus & 0xF0) == EventName::VoiceAftertouch) {
				nPreviousStatus = nStatus;
				uint8_t nChannel = nStatus & 0x0F;
				uint8_t nNoteID = inFile.get();
				uint8_t nNoteVelocity = inFile.get();
				vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::Other });
			}
			else if ((nStatus & 0xF0) == EventName::VoiceControlChange) {
				nPreviousStatus = nStatus;
				uint8_t nChannel = nStatus & 0x0F;
				uint8_t nNoteID = inFile.get();
				uint8_t nNoteVelocity = inFile.get();
				vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::Other });
			}
			else if ((nStatus & 0xF0) == EventName::VoiceProgramChange) {
				nPreviousStatus = nStatus;
				uint8_t nChannel = nStatus & 0x0F;
				uint8_t nProgramID = inFile.get();
				vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::Other });
			}
			else if ((nStatus & 0xF0) == EventName::VoiceChannelPressure) {
				nPreviousStatus = nStatus;
				uint8_t nChannel = nStatus & 0x0F;
				uint8_t nProgramID = inFile.get();
				vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::Other });
			}
			else if ((nStatus & 0xF0) == EventName::VoicePitchBend) {
				nPreviousStatus = nStatus;
				uint8_t nChannel = nStatus & 0x0F;
				uint8_t nLS7B = inFile.get();
				uint8_t nMS7B = inFile.get();
				vecTracks[nChunk].vecEvents.push_back({ MidiEvent::Type::Other });
			}
			else if ((nStatus & 0xF0) == EventName::SystemExclusive) {//making the file format flexible enough
				if (nStatus == 0xF0) {
					std::cout << "System exclusive begin" << std::endl;
				}
				if (nStatus == 0xF7) {
					std::cout << "System exclusive end" << std::endl;
				}
				if (nStatus == 0xFF) {
					uint8_t nType = inFile.get();
					uint8_t nLength = ReadValue();

					switch (nType)
					{
					case MetaSequence:
						std::cout << "Sequence Number: " << inFile.get() << inFile.get() << std::endl;
						break;
					case MetaText:
						std::cout << "Text: " << ReadString(nLength) << std::endl;
						break;
					case MetaCopyright:
						std::cout << "Copyright: " << ReadString(nLength) << std::endl;
						break;
					case MetaTrackName:
						vecTracks[nChunk].sName = ReadString(nLength);
						std::cout << "Track Name: " << vecTracks[nChunk].sName << std::endl;
						break;
					case MetaInstrumentName:
						vecTracks[nChunk].sInstrument = ReadString(nLength);
						std::cout << "Instrument Name: " << vecTracks[nChunk].sInstrument << std::endl;
						break;
					case MetaLyrics:
						std::cout << "Lyrics: " << ReadString(nLength) << std::endl;
						break;
					case MetaMarker:
						std::cout << "Marker: " << ReadString(nLength) << std::endl;
						break;
					case MetaCuePoint:
						std::cout << "Cue: " << ReadString(nLength) << std::endl;
						break;
					case MetaChannelPrefix:
						std::cout << "Prefix: " << inFile.get() << std::endl;
						break;
					case MetaEndOfTrack:
						bEndOfTrack = true;
						break;
					case MetaSetTempo:
						// Tempo is in microseconds per quarter note	
						if (m_nTempo == 0)
						{
							(m_nTempo |= (inFile.get() << 16));
							(m_nTempo |= (inFile.get() << 8));
							(m_nTempo |= (inFile.get() << 0));
							m_nBPM = (60000000 / m_nTempo);
							std::cout << "Tempo: " << m_nTempo << " (" << m_nBPM << "bpm)" << std::endl;
						}
						break;
					case MetaSMPTEOffset:
						std::cout << "SMPTE: H:" << inFile.get() << " M:" << inFile.get() << " S:" << inFile.get() << " FR:" << inFile.get() << " FF:" << inFile.get() << std::endl;
						break;
					case MetaTimeSignature:
						std::cout << "Time Signature: " << inFile.get() << "/" << (2 << inFile.get()) << std::endl;
						std::cout << "ClocksPerTick: " << inFile.get() << std::endl;

						// A MIDI "Beat" is 24 ticks, so specify how many 32nd notes constitute a beat
						std::cout << "32per24Clocks: " << inFile.get() << std::endl;
						break;
					case MetaKeySignature:
						std::cout << "Key Signature: " << inFile.get() << std::endl;
						std::cout << "Minor Key: " << inFile.get() << std::endl;
						break;
					case MetaSequencerSpecific:
						std::cout << "Sequencer Specific: " << ReadString(nLength) << std::endl;
						break;
					default:
						std::cout << "Unrecognised MetaEvent: " << nType << std::endl;
					}
				}
			}
			else {
				std::cout << "Unrecognized system byte" << std::endl;
			}
		}

	}

	for (auto& track : vecTracks) {
		uint32_t nWallTime = 0;

		std::list<MidiNote> listNotesBeingProcessed;
		for (auto& event : track.vecEvents) {
			nWallTime += event.nDeltaTick;

			if (event.event == MidiEvent::Type::NoteOn) {
				listNotesBeingProcessed.push_back({ event.nKey,event.nVelocity,nWallTime,0 });
			}

			if (event.event == MidiEvent::Type::NoteOff) {
				auto note = std::find_if(listNotesBeingProcessed.begin(), listNotesBeingProcessed.end(), [&](const MidiNote& n) {return n.nKey == event.nKey; });
				if (note != listNotesBeingProcessed.end()) {
					note->nDuration = nWallTime - note->nStartTime;
					track.vecNotes.push_back(*note);
					track.nMinNote = std::min(track.nMinNote, note->nKey);
					track.nMaxNote = std::min(track.nMaxNote, note->nKey);
					listNotesBeingProcessed.erase(note);
				}
			}
		}

	}

	std::cout << "Closing file" << std::endl;
	inFile.close();
	return true;
}
