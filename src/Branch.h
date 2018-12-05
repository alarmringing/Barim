#pragma once
#include "ofxBox2d.h"
#include "chuck.h"

class Branch {

public:
	Branch(b2World *boxWorld, float x, float y, int nodeNum);
	void linkNodes();
	void draw(ofColor branchColor, ofImage &leafImage);
	~Branch();

	b2World *boxWorld;
	ofxBox2dCircle anchor;
	vector<shared_ptr<ofxBox2dCircle>> nodes; //default box2d circles
	vector<shared_ptr<ofxBox2dJoint>> branchJoints; //between nodes
	float jointLength = 8;
	float leafSize = 18;
};

