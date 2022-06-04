#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>


struct MidiNote {
	uint8_t nKey = 0;
	uint8_t nVelocity = 0;
	uint32_t nStartTime = 0;
	uint32_t nDuration = 0;
};

struct MidiEvent {
	enum class Type {
		NoteOff,
		NoteOn,
		Other
	} event;
	uint8_t nKey = 0;
	uint8_t nVelocity = 0;
	uint32_t nWallTick = 0;
	uint32_t nDeltaTick = 0;
};




struct MidiTrack {
	std::string sName;
	std::string sInstrument;
	std::vector<MidiEvent> vecEvents;
	std::vector<MidiNote> vecNotes;
	uint8_t nMaxNote = 64;
	uint8_t nMinNote = 64;

};



class MidiFile {

	enum EventName : uint8_t {
		VoiceNoteOff = 0x80,
		VoiceNoteOn = 0x90,
		VoiceAftertouch = 0xA0,
		VoiceControlChange = 0xB0,
		VoiceProgramChange = 0xC0,
		VoiceChannelPressure = 0xD0,
		VoicePitchBend = 0xE0,
		SystemExclusive = 0xF0,
	};

	enum MetaEventName : uint8_t {
		MetaSequence = 0x00,
		MetaText = 0x01,
		MetaCopyright = 0x02,
		MetaTrackName = 0x03,
		MetaInstrumentName = 0x04,
		MetaLyrics = 0x05,
		MetaMarker = 0x06,
		MetaCuePoint = 0x07,
		MetaChannelPrefix = 0x20,
		MetaEndOfTrack = 0x2F,
		MetaSetTempo = 0x51,
		MetaSMPTEOffset = 0x54,
		MetaTimeSignature = 0x58,
		MetaKeySignature = 0x59,
		MetaSequencerSpecific = 0x7F,
	};


	public:
		MidiFile() {}
		MidiFile(const std::string& sFileName) {
			ParseFile(sFileName);
		}

		bool ParseFile(const std::string& sFileName) {
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
		}

		public:
			std::vector<MidiTrack> vecTracks;
			uint32_t m_nTempo = 0;
			uint32_t m_nBPM = 0;

};



int main() {

	MidiFile midi;
	midi.ParseFile("bach.mid");

	for (auto& track : midi.vecTracks) {
		if (!track.vecNotes.empty()) {

		}
	}


	return 0;
}