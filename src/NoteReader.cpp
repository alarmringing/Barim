#include "NoteReader.h"


NoteReader::NoteReader() {
	oscReceiver.setup(OSCPORT);
	// No fingers are on yet.
	for (int i = 0; i < sizeof(fingerStatus); i++) {
		fingerStatus[i] = 0;
	}
}

void NoteReader::update() {
	while (oscReceiver.hasWaitingMessages()) {
		// get the next message
		ofxOscMessage m;
		oscReceiver.getNextMessage(m);

		if (m.getAddress() == "/finger/ind"
			|| m.getAddress() == "/finger/mid"
			|| m.getAddress() == "/finger/ring"
			|| m.getAddress() == "/finger/pinky") {

			string fingerId = m.getAddress();
			fingerId.erase(0, 8);
			bool onOff = m.getArgAsBool(0);
			changeNote(fingerId, onOff);
		}
	}
}

void NoteReader::changeNote(string fingerId, bool onOff) {
	int noteId = 0;
	if (fingerId == "ind") noteId = 0;
	else if (fingerId == "mid") noteId = 1;
	else if (fingerId == "ring") noteId = 2;
	else if (fingerId == "pinky") noteId = 3;

	fingerStatus[noteId] = onOff;

	string noteString = to_string(fingerStatus[0]) + to_string(fingerStatus[1]) + to_string(fingerStatus[2]) + to_string(fingerStatus[3]);

	if (noteString == "0000") noteOn = false;
	else {
		if (noteString == "1110") currentNote = 66;
		if (noteString == "1100") currentNote = 68;
		if (noteString == "1000") currentNote = 70;
		if (noteString == "0100") currentNote = 71;
		if (noteString == "1111") currentNote = 73;
		if (noteString == "1101") currentNote = 75;
		if (noteString == "1001") currentNote = 78;
		noteOn = true;
	}
}

NoteReader::~NoteReader() {
}
