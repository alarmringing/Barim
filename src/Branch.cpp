#include "Branch.h"

Branch::Branch(b2World *boxWorld, float x, float y, int nodeNum) {

	anchor.setup(boxWorld, x, y, 4);

	// initialize the nodes
	for (int i = 0; i < nodeNum; i++) {
		shared_ptr<ofxBox2dCircle> circle = shared_ptr<ofxBox2dCircle>(new ofxBox2dCircle);
		circle.get()->setPhysics(30.0, 0.0, 0.1);
		float xOffset = (ofRandomf() - 0.5) * 10;
		circle.get()->setup(boxWorld, anchor.getPosition().x + xOffset, anchor.getPosition().y + (i + 1) * jointLength, 4);
		nodes.push_back(circle);
	}

	// now connect each circle with a joint
	for (int i = 0; i< nodes.size(); i++) {

		shared_ptr<ofxBox2dJoint> joint = shared_ptr<ofxBox2dJoint>(new ofxBox2dJoint);

		// if this is the first point connect to the top anchor.
		if (i == 0) {
			joint.get()->setup(boxWorld, anchor.body, nodes[i].get()->body);
		}
		else {
			joint.get()->setup(boxWorld, nodes[i - 1].get()->body, nodes[i].get()->body);
		}

		joint.get()->setLength(jointLength);
		joints.push_back(joint);
	}
}

void Branch::draw(ofColor branchColor) {
	ofSetColor(branchColor);
	anchor.draw();
	for (int i = 0; i < nodes.size(); i++) {
		ofFill();
		ofSetColor(branchColor);
		nodes[i].get()->draw();
	}

	for (int i = 0; i<joints.size(); i++) {
		ofSetColor(ofColor(1, 0, 0, 1));
		joints[i].get()->draw();
	}
}

Branch::~Branch() {
}
