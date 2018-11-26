#include "RtAudio/RtAudio.h"
#include "chuck.h"
#include "ofMain.h"
#include "ofApp.h"

ofApp* mainOfApp;

//-----------------------------------------------------------------------------
// desc: audio callback
//-----------------------------------------------------------------------------
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int numFrames,
	double streamTime, RtAudioStreamStatus status, void *data) {
	SAMPLE * input = (SAMPLE *)inputBuffer;
	SAMPLE * output = (SAMPLE *)outputBuffer;

	if (mainOfApp->useChuck) {
		mainOfApp->myChuck->run(input, mainOfApp->chuckOutputBuffer, numFrames);
	}

	// fill the buffer.
	mainOfApp->averageVolume = 0;
	for (int i = 0; i < numFrames; i++)
	{
		mainOfApp->currentAudioBuffer[i] = 0;
		output[i * MY_CHANNELS] = 0;
		output[i * MY_CHANNELS + 1] = 0;

		if (!mainOfApp->useChuck) {
			// If not using chuck, write channel 0 of input to currentAudioBuffer.
			mainOfApp->currentAudioBuffer[i] = input[i*MY_CHANNELS];
		}
		else {
			// If using chuck, copy output from chuck to currentAudioBuffer and output buffer of computer.
			mainOfApp->currentAudioBuffer[i] = mainOfApp->chuckOutputBuffer[i];
			output[i*MY_CHANNELS] = mainOfApp->chuckOutputBuffer[i];
			output[i*MY_CHANNELS + 1] = mainOfApp->chuckOutputBuffer[i];
		}
		mainOfApp->averageVolume += ::fabs(mainOfApp->currentAudioBuffer[i]) / (float)numFrames;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// desc: Modified from VisualSine.cpp.
//-----------------------------------------------------------------------------
void setupAudio() {
	RtAudio audio;
	unsigned int bufferBytes = 0;
	unsigned int bufferFrames = SND_BUFFER_SIZE;

	// check for audio devices
	if (audio.getDeviceCount() < 1) {
		// nopes
		cout << "no audio devices found!" << endl;
		return;
	}
	audio.showWarnings(true); // let RtAudio print messages to stderr.

							  // set input and output parameters
	RtAudio::StreamParameters iParams, oParams;
	iParams.deviceId = audio.getDefaultInputDevice();
	iParams.nChannels = MY_CHANNELS;
	iParams.firstChannel = 0;
	oParams.deviceId = audio.getDefaultOutputDevice();
	oParams.nChannels = MY_CHANNELS;
	oParams.firstChannel = 0;

	// create stream options
	RtAudio::StreamOptions options;

	try {
		// open a stream, register callback.
		audio.openStream(&oParams, &iParams, MY_FORMAT, MY_SRATE, &bufferFrames, &audioCallback, (void *)&bufferBytes, &options);
	}
	catch (RtError& e) {
		cout << e.getMessage() << endl;
		return;
	}

	// fill in buffers.
	mainOfApp->currentAudioBuffer = new SAMPLE[bufferFrames];
	memset(mainOfApp->currentAudioBuffer, 0, sizeof(SAMPLE) * bufferFrames);
	mainOfApp->chuckOutputBuffer = new SAMPLE[bufferFrames];
	memset(mainOfApp->chuckOutputBuffer, 0, sizeof(SAMPLE) * bufferFrames);

	mainOfApp->myChuck = new ChucK();
	mainOfApp->myChuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 1);
	mainOfApp->myChuck->setParam(CHUCK_PARAM_SAMPLE_RATE, MY_SRATE);
	mainOfApp->myChuck->init();

	// Run updateDrivenByAudio between each stream update.
	try {
		audio.startStream();
		ofRunApp(mainOfApp);
		audio.stopStream();
	}
	catch (RtError& e) {
		cout << e.getMessage() << endl;
		goto cleanup;
	}

cleanup:
	// close if open
	if (audio.isStreamOpen())
		audio.closeStream();
	return;
}

//========================================================================
int main( ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	mainOfApp = new ofApp();
	setupAudio();
	
}