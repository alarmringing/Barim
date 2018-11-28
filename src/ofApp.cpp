#include "ofApp.h"

#include <sstream>

using namespace std;
using namespace ofxKinectForWindows2;

//--------------------------------------------------------------
void ofApp::setup(){
	kinect.open();
	kinect.initDepthSource();
	kinect.initBodySource();
	kinect.initBodyIndexSource();

	gui.setup();
	gui.add(fps.setup("fps", ofToString(ofGetFrameRate())));

	backgroundShader.load("shaders/background");
	backgroundFbo.allocate(ofGetWidth(), ofGetHeight());
}

void ofApp::sporkNewChuckFile(string pathName) {
	string args = "";
	myChuck->compileFile(pathName, args);
	return;
}

bool ofApp::isJointTrackingStable(JointType jointType) {
	if (currentBody.joints[jointType].getTrackingState() == TrackingState_Tracked
		&& previousBody.joints[jointType].getTrackingState() == TrackingState_Tracked) return true;
	return false;
}

void ofApp::checkHeadGong() {
	Data::Joint previousHeadJoint = previousBody.joints[JointType_Head];
	Data::Joint currentHeadJoint = currentBody.joints[JointType_Head];
	float headYVelocity = (currentHeadJoint.getPositionInWorld().y - previousHeadJoint.getPositionInWorld().y) / ofGetLastFrameTime();
	// Prevent noise
	if (headYVelocity < gongTriggerVelocity &&
			ofGetElapsedTimef() - lastGongTime > gongMinInterval &&
			currentHeadJoint.getTrackingState() == TrackingState_Tracked) {
		sporkNewChuckFile(GONGPATH);
		lastGongTime = ofGetElapsedTimef();
	}
}

void ofApp::checkBothHandsOpen() {
	// Because humans are slower than kinect's fps, account for duration by first storing when each hand was last opened.
	if ((previousBody.rightHandState == HandState_Closed || previousBody.rightHandState == HandState_Unknown)
			&& currentBody.rightHandState == HandState_Open) {
		lastRightHandOpenTime = ofGetElapsedTimef();
	}
	if ((previousBody.leftHandState == HandState_Closed || previousBody.leftHandState == HandState_Unknown)
			&& currentBody.leftHandState == HandState_Open) {
		lastLeftHandOpenTime = ofGetElapsedTimef();
	}
	// Both hands are open now, and both hands were opened within x seconds.
	if ((currentBody.rightHandState == HandState_Open && currentBody.leftHandState == HandState_Open)
		&& ofGetElapsedTimef() - lastRightHandOpenTime < handOpenDelayLimit 
		&& ofGetElapsedTimef() - lastLeftHandOpenTime < handOpenDelayLimit) {
		
		int randomInt;
		do {
			randomInt = ofRandom(size(noteOptions));
		} while (randomInt == currentNoteIndex);
		myChuck->setGlobalInt("note", noteOptions[randomInt]); // Change note.
		currentNoteIndex = randomInt;

		lastRightHandOpenTime = 0;
		lastLeftHandOpenTime = 0;
	}
}

void ofApp::checkMaxHandHeight() {
	float currentMaxHandHeight = numeric_limits<float>().lowest();
	if (isJointTrackingStable(JointType_HandRight) 
			&& currentBody.joints[JointType_HandRight].getPosition().y > currentMaxHandHeight) {
		currentMaxHandHeight = currentBody.joints[JointType_HandRight].getPosition().y;
	}
	if (isJointTrackingStable(JointType_HandLeft)
			&& currentBody.joints[JointType_HandLeft].getPosition().y > currentMaxHandHeight) {
		currentMaxHandHeight = currentBody.joints[JointType_HandLeft].getPosition().y;
	}
	if (currentMaxHandHeight > numeric_limits<float>::lowest()) maxHandHeight = currentMaxHandHeight;
	
	float fluteGainLerped = ofLerp(0.15, 1, ofClamp(currentMaxHandHeight, 0, 1));
	myChuck->setGlobalFloat("finalGain", fluteGainLerped);
}

void ofApp::lerpBtwFluteParamsAndWrite(Flute firstFlute, Flute secondFlute, float amt) {
	myChuck->setGlobalFloat("jetDelay", ofLerp(firstFlute.jetDelay, secondFlute.jetDelay, amt));
	myChuck->setGlobalFloat("jetReflection", ofLerp(firstFlute.jetReflection, secondFlute.jetReflection, amt));
	myChuck->setGlobalFloat("endReflection", ofLerp(firstFlute.endReflection, secondFlute.endReflection, amt));
	myChuck->setGlobalFloat("noiseGain", ofLerp(firstFlute.noiseGain, secondFlute.noiseGain, amt));
	myChuck->setGlobalFloat("pressure", ofLerp(firstFlute.pressure, secondFlute.pressure, amt));
	myChuck->setGlobalFloat("vibratoFreq", ofLerp(firstFlute.vibratoFreq, secondFlute.vibratoFreq, amt));
	myChuck->setGlobalFloat("vibratoGain", ofLerp(firstFlute.vibratoGain, secondFlute.vibratoGain, amt));
}


void ofApp::checkHandSpeed() {
	float speedSum = 0;
	if (isJointTrackingStable(JointType_HandRight)) {
		float currentSpeed =
			(currentBody.joints[JointType_HandRight].getPosition() 
				- previousBody.joints[JointType_HandRight].getPosition()).length();
		speedSum += currentSpeed;
		rightHandPreviousSpeed = currentSpeed;
	}
	else {
		speedSum += rightHandPreviousSpeed;
	}
	if (isJointTrackingStable(JointType_HandLeft)) {
		float currentSpeed =
			(currentBody.joints[JointType_HandLeft].getPosition()
				- previousBody.joints[JointType_HandLeft].getPosition()).length();
		speedSum += currentSpeed;
		leftHandPreviousSpeed = currentSpeed;
	}
	else {
		speedSum += leftHandPreviousSpeed;
	}
	speedSum = speedSum / 2;
	float breathLerpAmount = pow(ofClamp(speedSum, 0, 0.1), 0.2);
	
	//lerpBtwFluteParamsAndWrite(noisyFlute, notNoisyFlute, breathLerpAmount);
}

void ofApp::updateKinectData() {
	kinect.update();

	//Getting joint positions (skeleton tracking)
	vector<ofxKinectForWindows2::Data::Body> bodies = kinect.getBodySource()->getBodies();
	if (bodies.size() < 1) return; // Return if no human in scene.

	for (Data::Body body : bodies) {
		if (body.tracked) {
			if (!hasKinectStarted) {
				hasKinectStarted = true;
				sporkNewChuckFile(FLUTEPATH);
			}
			previousBody = currentBody;
			currentBody = body;
			break; // Just take first if there are multiple tracked bodies.
		}
	}

	checkHeadGong();
	checkBothHandsOpen();
	checkMaxHandHeight();
	checkHandSpeed();
	//lerpBtwFluteParamsAndWrite(straightFlute, normalSalientFlute, sin(ofGetElapsedTimef()));

	/*
	for (auto body : bodies) {
		for (auto joint : body.joints) {
			//now do something with the joints
		}
	}
	*/

	/*
	//Getting bones (connected joints)
	// Note that for this we need a reference of which joints are connected to each other.
	// We call this the 'boneAtlas', and you can ask for a reference to this atlas whenever you like
	auto boneAtlas = ofxKinectForWindows2::Data::Body::getBonesAtlas();

	for (auto body : bodies) {
		for (auto bone : boneAtlas) {
			auto firstJointInBone = body.joints[bone.first];
			auto secondJointInBone = body.joints[bone.second];
			//now do something with the joints
		}
	}
	*/
}

//--------------------------------------------------------------
void ofApp::update() {
	updateKinectData();
}

//--------------------------------------------------------------
void ofApp::drawBackground() {
	backgroundFbo.begin();
	backgroundShader.begin();

	backgroundShader.end();
	backgroundFbo.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
	kinect.getBodyIndexSource()->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	//kinect.getColorSource()->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	//backgroundFbo.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	kinect.getBodySource()->drawProjected(0, 0, ofGetWindowWidth(), ofGetWindowHeight(), ofxKFW2::ProjectionCoordinates::DepthCamera);
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
