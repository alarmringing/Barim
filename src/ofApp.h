#pragma once
#include "RtAudio/RtAudio.h"
#include "chuck.h"

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxKinectForWindows2.h"
#include "ofxBox2d.h"
#include "ofxAssimpModelLoader.h"
#include "ofxFlowTools.h"

#include "Branch.h"

#define SAMPLE float
#define MY_FORMAT RTAUDIO_FLOAT32
#define MY_SRATE 44100
#define MY_CHANNELS 2
#define PI 3.14159265358979
#define SND_BUFFER_SIZE 1024

#define GONGPATH "ckfiles/gongPlayer.ck"
#define FLUTEPATH "ckfiles/bambooFlutePlayer.ck"
#define SUNPATH "assets/sun.png"
#define LEAF2DPATH "assets/strokeTransparent.png"
#define PARTICLESETTINGPATH "particleSettings.xml"

using namespace flowTools;

struct Flute {
	double jetDelay;
	double jetReflection;
	double endReflection;
	double noiseGain;
	double pressure;
	double vibratoFreq;
	double vibratoGain;
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void sporkNewChuckFile(string fileName);
	void addBranch(float xPos);
	bool isJointTrackingStable(JointType jointType);
	void checkHeadGong();
	void checkBothHandsOpen();
	void checkMaxHandHeight();
	void checkHandSpeed();
	void updateFlow();
	void updateBranches();
	void generateBranches();
	void lerpBtwFluteParamsAndWrite(Flute firstFlute, Flute secondFlute, float amt);
	void updateKinectData();
	void update();
	void drawBackground();
	void drawWillow();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ChucK *myChuck;
	string chuckPath;
	SAMPLE *currentAudioBuffer = NULL;
	SAMPLE *chuckOutputBuffer = NULL;
	bool useChuck = true;
	float averageVolume;

	// Kinect setup
	ofxKFW2::Device kinect;
	bool hasKinectStarted = false;
	ofxKinectForWindows2::Data::Body previousBody;
	ofxKinectForWindows2::Data::Body currentBody;
	shared_ptr<ofxBox2dCircle> boxLeftHand;
	shared_ptr<ofxBox2dCircle> boxRightHand;
	float handSize = 25;

	// Head gong 
	float lastGongTime = 0;
	float gongMinInterval = 0.5;
	float gongTriggerVelocity = -0.7;
	// sun
	float sunFadeRate = 30;
	float sunAlpha = 0.0f;
	float sunSize = 200;
	ofImage sunImage;

	// Hand open check
	int noteOptions[5] = { 61, 63, 66, 68, 70 };
	int currentNoteIndex = 0;
	float lastLeftHandOpenTime;
	float lastRightHandOpenTime;
	float handOpenDelayLimit = 0.5;

	// Hand height
	float maxHandHeight;

	// Hand velocity
	float leftHandPreviousSpeed = 0;
	float rightHandPreviousSpeed = 0;
	Flute weirdSalientFlute = {
		0.751648, //jetDelay 
		0.295592, //jetReflection
		0.834268, //endReflection
		0.753903, //noiseGain
		0.835219, //pressure 
		1, //vibratoFreq
		0 // vibratoGain
	};

	Flute straightFlute = {
		0.751648, //jetDelay 
		0.395552, //jetReflection
		0.604268, //endReflection
		0.353903, //noiseGain
		0.735219, //pressure 
		1, //vibratoFreq
		0 // vibratoGain
	};
	Flute strongFlute = {
		0.751648, //jetDelay 
		0.395552, //jetReflection
		0.604268, //endReflection
		0.353903, //noiseGain
		0.735219, //pressure 
		1, //vibratoFreq
		0 // vibratoGain
	};
	Flute normalSalientFlute = {
		0.751648, //jetDelay 
		0.682338, //jetReflection
		0.675746, //endReflection
		0.739889, //noiseGain
		0.598381, //pressure 
		1, //vibratoFreq
		0 // vibratoGain
	};

	ofxPanel gui;
	ofxLabel fps;
	ofFbo backgroundFbo;
	ofShader backgroundShader;
	float dayProgressionSpeed;

	// Willow world
	ofxBox2d box2d;
	ofxBox2d willowWorld;
	int numBranches = 30;
	vector<shared_ptr<Branch>> branches;

	// Leaf
	//ofxAssimpModelLoader leafModel;
	ofImage leafImage;

	// FlowTools
	int	flowWidth;
	int	flowHeight;

	ftFbo flowInputFbo;
	ftOpticalFlow opticalFlow;
	ftVelocityMask velocityMask;
	ftFluidSimulation fluidSimulation;
	ftParticleFlow particleFlow;
};
