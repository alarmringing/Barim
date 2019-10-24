#include "ofApp.h"

#include <sstream>

using namespace std;
using namespace ofxKinectForWindows2;

//--------------------------------------------------------------
void ofApp::setup(){
	// Kinect Setup
	kinect.open();
	kinect.initDepthSource();
	kinect.initBodySource();
	kinect.initBodyIndexSource();

	backgroundShader.load("shaders/background");
	backgroundFbo.allocate(ofGetWidth(), ofGetHeight());
	leafShader.load("shaders/leaf");
	leafFbo.allocate(ofGetWidth(), ofGetHeight());
	
	// physics
	box2d.init();
	box2d.setGravity(0, 10);
	box2d.setFPS(60.0);
	box2d.registerGrabbing();
	boxLeftHand = shared_ptr<ofxBox2dCircle>(new ofxBox2dCircle);
	boxLeftHand.get()->setPhysics(0.001, 0.0, 5);
	boxLeftHand.get()->setup(box2d.world, 0, 0, handSize);
	boxRightHand = shared_ptr<ofxBox2dCircle>(new ofxBox2dCircle);
	boxRightHand.get()->setPhysics(0.001, 0.0, 5);
	boxRightHand.get()->setup(box2d.world, 0, 0, handSize);

	// branches
	Branch::scaleFactor = 1.7;
	generateBranches();
	leafImage.load(LEAF2DPATH);

	// Flow tools
	flowWidth = ofGetWidth() / 4;
	flowHeight = ofGetHeight() / 4;
	flowInputFbo.allocate(ofGetWidth(), ofGetHeight());
	flowInputFbo.black();
	opticalFlow.setup(flowWidth, flowHeight);
	velocityMask.setup(ofGetWidth(), ofGetHeight());
	fluidSimulation.setup(flowWidth, flowHeight, ofGetWidth(), ofGetHeight());
	particleFlow.setup(flowWidth, flowHeight, ofGetWidth(), ofGetHeight());

	// Other aesthetics
	sunImage.load(SUNPATH);

	// GUI Setup
	gui.setup("particleSettings", PARTICLESETTINGPATH);
	gui.add(fps.setup("fps", ofToString(ofGetFrameRate())));
	gui.add(opticalFlow.parameters);
	gui.add(velocityMask.parameters);
	gui.add(fluidSimulation.parameters);
	gui.add(particleFlow.parameters);
	// if the settings file is not present the parameters will not be set during this setup
	if (!ofFile(PARTICLESETTINGPATH)) gui.saveToFile(PARTICLESETTINGPATH);
	gui.loadFromFile(PARTICLESETTINGPATH);

	// Note Control
	noteReader = NoteReader();
}

void ofApp::sporkNewChuckFile(string pathName) {
	string args = "";
	myChuck->compileFile(pathName, args);
	return;
}

void ofApp::generateBranches() {
	// Generate positions of branch groups.
	vector<float> groupPositions;
	float maxPos = 0.0f;
	float minPos = ofGetWidth() + 10;
	for (int i = 0; i < numBranchGroups; i++) {
		float newPos = ofRandom(ofGetWidth());
		if (maxPos < newPos) maxPos = newPos;
		if (minPos > newPos) minPos = newPos;
		groupPositions.push_back(newPos);
	}
	// Normalize the group positions so that they are reasonably spaced across screen.
	for (int i = 0; i < numBranchGroups; i++) {
		groupPositions[i] = groupPositions[i] - minPos + 10;
		groupPositions[i] *= (ofGetWidth()*0.8) / (maxPos - minPos);
	}
	

	// Generate each branch and add it to branches.
	for (int i = 0; i < numBranches; i++) {
		int branchGroupMembership = ofRandom(numBranchGroups);
		float deeviationFromGroup = 0;
		if (branchGroupMembership == 0 || branchGroupMembership == numBranchGroups - 1) {
			deeviationFromGroup = ofRandom(branchGroupDeviationDegree) 
					- ((branchGroupMembership - numBranchGroups / 2)>0) * branchGroupDeviationDegree;
		}
		else {
			deeviationFromGroup = ofRandom(branchGroupDeviationDegree) - branchGroupDeviationDegree / 2;
		}
		float xPos = groupPositions[branchGroupMembership] + deeviationFromGroup;
		int nodeNum = 6 + ofRandom(27);
		shared_ptr<Branch> branch = shared_ptr<Branch>(new Branch(box2d.getWorld(), xPos, -20 - 10 * ofRandomf(), nodeNum, myChuck));
		branches.push_back(branch);
	}
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
		fogAmount = 0.6;
	}
}

Flute ofApp::lerpNewFlute(Flute firstFlute, Flute secondFlute, float amt) {
	Flute newFlute = Flute();
	newFlute.jetDelay = ofLerp(firstFlute.jetDelay, secondFlute.jetDelay, amt);
	newFlute.jetReflection = ofLerp(firstFlute.jetReflection, secondFlute.jetReflection, amt);
	newFlute.endReflection = ofLerp(firstFlute.endReflection, secondFlute.endReflection, amt);
	newFlute.noiseGain = ofLerp(firstFlute.noiseGain, secondFlute.noiseGain, amt);
	newFlute.pressure = ofLerp(firstFlute.pressure, secondFlute.pressure, amt);
	newFlute.vibratoFreq = ofLerp(firstFlute.vibratoFreq, secondFlute.vibratoFreq, amt);
	newFlute.vibratoGain = ofLerp(firstFlute.vibratoGain, secondFlute.vibratoGain, amt);
	newFlute.finalGain = ofLerp(firstFlute.finalGain, secondFlute.finalGain, amt);
	return newFlute;
}

void ofApp::updateFluteInChuck(Flute flute) {
	myChuck->setGlobalFloat("jetDelay", flute.jetDelay);
	myChuck->setGlobalFloat("jetReflection", flute.jetReflection);
	myChuck->setGlobalFloat("endReflection", flute.endReflection);
	myChuck->setGlobalFloat("noiseGain", flute.noiseGain);
	myChuck->setGlobalFloat("pressure", flute.pressure);
	myChuck->setGlobalFloat("vibratoFreq", flute.vibratoFreq);
	myChuck->setGlobalFloat("vibratoGain", flute.vibratoGain);
	myChuck->setGlobalFloat("finalGain", flute.finalGain);
}

float ofApp::getMaxHandHeight() {
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
	
	return ofLerp(0.05, 1, ofClamp(currentMaxHandHeight, 0, 0.4));
}

float ofApp::getMaxHandFront() {
	float currentMaxHandFront = numeric_limits<float>().lowest();
	float handDistFromElbow;
	handDistFromElbow = currentBody.joints[JointType_ElbowRight].getPosition().z - currentBody.joints[JointType_HandRight].getPosition().z;
	if (isJointTrackingStable(JointType_HandRight)
		&& handDistFromElbow > currentMaxHandFront) {
		currentMaxHandFront = handDistFromElbow;
	}
	handDistFromElbow = currentBody.joints[JointType_ElbowLeft].getPosition().z - currentBody.joints[JointType_HandLeft].getPosition().z;
	if (isJointTrackingStable(JointType_HandLeft)
		&& handDistFromElbow > currentMaxHandFront) {
		currentMaxHandFront = handDistFromElbow;
	}
	if (currentMaxHandFront > numeric_limits<float>::lowest()) maxHandFront = currentMaxHandFront;
	// z is typically 1 at closest, 0.5 at furthest. we want it to be 0 at closest and 1 at furthest.
	float lerpAmt = ofClamp(currentMaxHandFront, 0, 0.3) * (1 / 0.3);

	return lerpAmt;
}

float ofApp::getHandSpeed() {
	float leftSpeed = 0;
	float rightSpeed = 0;
	if (isJointTrackingStable(JointType_HandRight)) {
		ofVec2f projectedPos = currentBody.joints[JointType_HandRight].getPositionInDepthMap();
		projectedPos.x = projectedPos.x / kinect.getDepthSource().get()->getWidth() * ofGetWidth();
		projectedPos.y = projectedPos.y / kinect.getDepthSource().get()->getHeight() * ofGetHeight();
		boxRightHand.get()->setPosition(projectedPos);

		float currentSpeed =
			(currentBody.joints[JointType_HandRight].getPosition() 
				- previousBody.joints[JointType_HandRight].getPosition()).length();
		rightSpeed += currentSpeed / ofGetLastFrameTime();
		rightHandPreviousSpeed = currentSpeed;
	}
	else {
		rightSpeed += rightHandPreviousSpeed / ofGetLastFrameTime();
	}
	if (isJointTrackingStable(JointType_HandLeft)) {
		ofVec2f projectedPos = currentBody.joints[JointType_HandLeft].getPositionInDepthMap();
		projectedPos.x = projectedPos.x / kinect.getDepthSource().get()->getWidth() * ofGetWidth();
		projectedPos.y = projectedPos.y / kinect.getDepthSource().get()->getHeight() * ofGetHeight();
		boxLeftHand.get()->setPosition(projectedPos);

		float currentSpeed =
			(currentBody.joints[JointType_HandLeft].getPosition()
				- previousBody.joints[JointType_HandLeft].getPosition()).length();
		leftSpeed += currentSpeed / ofGetLastFrameTime();
		leftHandPreviousSpeed = currentSpeed;
	}
	else {
		leftSpeed += leftHandPreviousSpeed / ofGetLastFrameTime();
	}
	float maxSpeed = max(leftSpeed, rightSpeed);
	return ofClamp(maxSpeed, 0, 7) / 7; // 1 is faster
}

float ofApp::getHandDistance() {
	float handDistance = 0;
	if (isJointTrackingStable(JointType_HandLeft) && isJointTrackingStable(JointType_HandRight)) {
		ofVec3f leftPos = currentBody.joints[JointType_HandLeft].getPosition();
		ofVec3f rightPos = currentBody.joints[JointType_HandRight].getPosition();
		handDistance = leftPos.distance(rightPos);
	}
	else handDistance = previousHandDistance;
	previousHandDistance = handDistance;
	return ofClamp(handDistance, 0, 1.5) / 1.5;
}

void ofApp::controlFlute() {
	float handSpeedLerpAmt = getHandSpeed();
	myFlute = lerpNewFlute(straightFlute, strongFlute, pow(handSpeedLerpAmt, 0.15));
	//myChuck->setGlobalFloat("filterRate", ofLerp(1, strongFluteFilterRate, handSpeedLerpAmt));

	
	float handHeightLerpAmt = pow(getMaxHandHeight(), 0.52);
	myFlute = lerpNewFlute(breathyFlute, myFlute, handHeightLerpAmt);

	//float handFrontLerpAmt = getMaxHandFront();

	float handDistanceLerpAmt = pow(getHandDistance(), 2);
	myFlute.jetDelay = ofLerp(myFlute.jetDelay + pitchPushRange / 2, myFlute.jetDelay - pitchPushRange / 2, handDistanceLerpAmt);

	updateFluteInChuck(myFlute);

	// Inputs from phone.
	myChuck->setGlobalInt("note", noteReader.currentNote); 
	myChuck->setGlobalInt("noteOn", noteReader.noteOn);
}

void ofApp::updateFlow() {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	flowInputFbo.begin();
	kinect.getBodyIndexSource()->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	flowInputFbo.end();
	ofPopStyle();

	opticalFlow.setSource(flowInputFbo.getTexture());
	opticalFlow.update();

	velocityMask.setDensity(flowInputFbo.getTexture());
	velocityMask.setVelocity(opticalFlow.getOpticalFlow());
	velocityMask.update();

	fluidSimulation.addVelocity(opticalFlow.getOpticalFlowDecay());
	fluidSimulation.addDensity(velocityMask.getColorMask());
	fluidSimulation.addTemperature(velocityMask.getLuminanceMask());
	fluidSimulation.update();

	// ParticleFlow is dependent on: opticalFlow, fluidSimulation (Dependent on opticalFlow and velocitymask)
	particleFlow.setSpeed(fluidSimulation.getSpeed());
	particleFlow.setCellSize(fluidSimulation.getCellSize());
	particleFlow.addFlowVelocity(opticalFlow.getOpticalFlow());
	particleFlow.addFluidVelocity(fluidSimulation.getVelocity());
	particleFlow.setObstacle(fluidSimulation.getObstacle());
	particleFlow.update();
}


void ofApp::updateKinectData() {
	kinect.update(); // Update early to not feed into ofxFlowTools when there's no body recognition.
	vector<ofxKinectForWindows2::Data::Body> bodies = kinect.getBodySource()->getBodies();
	if (!kinect.getBodySource()->isFrameNew()) return; // Return if no human in scene or kinect is not working.

	updateFlow();

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
}

//--------------------------------------------------------------
void ofApp::update() {
	updateKinectData();
	checkHeadGong();
	controlFlute();
	box2d.update();
	noteReader.update();
}

//--------------------------------------------------------------
void ofApp::drawBackground() {
	//backgroundFbo.begin();
	ofBackground(20, 20, 20, 255);
	backgroundShader.begin();
	backgroundShader.setUniform2f("u_resolution", backgroundFbo.getWidth(), backgroundFbo.getHeight());
	backgroundShader.setUniform1f("u_time", ofGetElapsedTimef());
	backgroundShader.setUniform1f("u_deltaTime", ofGetLastFrameTime());
	if (fogAmount > minFogAmount) fogAmount = fogAmount - fogFadeRate * ofGetLastFrameTime();
	backgroundShader.setUniform1f("u_fogAmount", fogAmount);
	ofSetColor(200, 200, 200, 255);
	ofDrawRectangle(0, 0, backgroundFbo.getWidth(), backgroundFbo.getHeight());
	backgroundShader.end();
	//backgroundFbo.end();
}

//--------------------------------------------------------------
void ofApp::draw(){

	leafFbo.begin();
	ofPushMatrix();
	//ofScale(-1, 1, 1);
	//ofTranslate(-ofGetWidth(), 0, 0);
	//ofBackground(20, 20, 20, 255);
	//kinect.getBodyIndexSource()->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	//kinect.getColorSource()->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	//backgroundFbo.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	//kinect.getBodySource()->drawProjected(0, 0, ofGetWindowWidth(), ofGetWindowHeight(), ofxKFW2::ProjectionCoordinates::DepthCamera);

	drawBackground();
	//backgroundFbo.draw(0, 0, ofGetWidth(), ofGetHeight());	

	ofPushStyle();
	ofSetColor(ofColor(200, 200, 200, 255));
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	if (particleFlow.isActive())
		particleFlow.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	ofPopStyle();

	ofPushStyle();
	//draw willow
	for (int i = 0; i < branches.size(); i++) {
		branches[i]->draw(ofColor(200, 200, 200, 255), leafImage);
	}
	ofPopStyle();

	/*
	leafShader.begin();
	leafShader.setUniform2f("u_resolution", backgroundFbo.getWidth(), backgroundFbo.getHeight());
	leafShader.setUniform1f("u_time", ofGetElapsedTimef());
	leafShader.setUniformTexture("leafTex", leafFbo, 0);
	ofDrawRectangle(0, 0, leafFbo.getWidth(), leafFbo.getHeight());

	leafShader.end();
	*/

	// Presentation (body tracking)
	float w = 270;
	float h = 180;
	kinect.getBodyIndexSource()->draw(ofGetWindowWidth() - w, ofGetWindowHeight() - h, w, h);

	ofPopMatrix();
	leafFbo.end();
	leafFbo.draw(0,0,ofGetWidth(), ofGetHeight());
	//gui.draw();
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
