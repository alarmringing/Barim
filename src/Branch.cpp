#include "Branch.h"

Branch::Branch(b2World *boxWorldRef, float x, float y, int nodeNum) {
	boxWorld = boxWorldRef;
	anchor.setup(boxWorld, x, y, 4);

	// initialize the nodes
	for (int i = 0; i < nodeNum; i++) {
		shared_ptr<ofxBox2dCircle> circle = shared_ptr<ofxBox2dCircle>(new ofxBox2dCircle);
		circle.get()->setPhysics(10, 0.0, 10);
		float xOffset = (ofRandomf() - 0.5) * 10;
		circle.get()->setup(boxWorld, anchor.getPosition().x + xOffset, anchor.getPosition().y + (i + 1) * jointLength, 0.01);
		nodes.push_back(circle);
	}

	linkNodes();
}

void Branch::linkNodes() {
	// now connect each node with a joint
	for (int i = 0; i< nodes.size(); i++) {

		shared_ptr<ofxBox2dJoint> joint = shared_ptr<ofxBox2dJoint>(new ofxBox2dJoint);
		joint->jointType = e_ropeJoint;

		// if this is the first point connect to the top anchor.
		if (i == 0) {
			joint.get()->setup(boxWorld, anchor.body, nodes[i].get()->body);
		}
		else {
			joint.get()->setup(boxWorld, nodes[i - 1].get()->body, nodes[i].get()->body);
		}
		joint.get()->setLength(jointLength);
		branchJoints.push_back(joint);
	}
}

void Branch::draw(ofColor branchColor, ofImage &leafImage) {
	ofSetColor(branchColor);
	anchor.draw();
	for (int i = 0; i < nodes.size() - 1; i++) {
		ofFill();
		ofSetColor(branchColor);
	
		ofPushMatrix();
		ofVec2f bodyA = nodes[i].get()->getPosition();
		ofVec2f bodyB = nodes[i+1].get()->getPosition();
		ofVec2f vector = bodyB - bodyA;
		float theta = atan2(vector.x, vector.y);
		ofTranslate(bodyA.middle(bodyB).x, bodyA.middle(bodyB).y, 0);
		ofRotate(PI - ofRadToDeg(theta));
		leafImage.setAnchorPercent(0.5, 0.5);
		leafImage.draw(0, 0, leafSize, leafSize);
		ofPopMatrix();
		
	}

	/*
	for (int i = 0; i< leaves.size(); i++) {
		ofxBox2dRect *leaf = leaves[i].get();
		leafImage.rotate90(leaf->getRotation());
		leafImage.draw(leaf->getPosition().x - leaf->getWidth() / 2, leaf->getPosition().y - leaf->getHeight() / 2, leaf->getWidth(), leaf->getHeight());
	}*/

	for (int i = 0; i< branchJoints.size(); i++) {
		ofSetColor(branchColor);
		ofxBox2dJoint *branch = branchJoints[i].get();
		branch->draw();
	}
}

Branch::~Branch() {
}
