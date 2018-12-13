#pragma once
#include "ofxOsc.h"

#define OSCPORT 12345


class NoteReader {

public:
	NoteReader();
	void update();
	void changeNote(string fingerId, bool onoff);
	~NoteReader();


	ofxOscReceiver	oscReceiver;
	//int current_msg_string;

	bool fingerStatus[4];
	bool noteOn = false;
	int currentNote = 61;
};

