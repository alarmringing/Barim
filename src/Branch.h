#pragma once
#include "ofxBox2d.h"

class Branch {

public:
	Branch(b2World *boxWorld, float x, float y, int nodeNum);
	void draw(ofColor branchColor);
	void updateWind(ofTexture &flowVelocityTexture, ofFloatPixels &flowVelocityPixels);
	~Branch();

	ofxBox2dCircle anchor;
	vector<shared_ptr<ofxBox2dCircle>> nodes; //default box2d circles
	vector<shared_ptr<ofxBox2dJoint>> joints; //joints
	float jointLength = 25;
};

