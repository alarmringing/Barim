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
}

void ofApp::sporkNewChuckFile(string pathName) {
	string args = "";
	myChuck->compileFile(pathName, args);
	return;
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

void ofApp::updateKinectData() {
	kinect.update();

	//Getting joint positions (skeleton tracking)
	vector<ofxKinectForWindows2::Data::Body> bodies = kinect.getBodySource()->getBodies();
	if (bodies.size() < 1) return; // Return if no human in scene.

	for (Data::Body body : bodies) {
		if (body.tracked) {
			previousBody = currentBody;
			currentBody = body;
			break; // Just take first if there are multiple tracked bodies.
		}
	}

//	if (currentBody == NULL || previousBody == NULL) return;

	checkHeadGong();
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
void ofApp::draw(){
	kinect.getBodyIndexSource()->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
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
