#include "Branch.h"

Branch::Branch(b2World *boxWorld, float x, float y, int nodeNum) {

	anchor.setup(boxWorld, x, y, 4);

	// initialize the nodes
	for (int i = 0; i < nodeNum; i++) {
		shared_ptr<ofxBox2dCircle> circle = shared_ptr<ofxBox2dCircle>(new ofxBox2dCircle);
		circle.get()->setPhysics(0.001, 0.0, 10);
		float xOffset = (ofRandomf() - 0.5) * 10;
		circle.get()->setup(boxWorld, anchor.getPosition().x + xOffset, anchor.getPosition().y + (i + 1) * jointLength, 0.01);
		nodes.push_back(circle);
	}

	// now connect each circle with a joint
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
		joints.push_back(joint);
	}
}

void Branch::updateWind(ofTexture &flowVelocityTexture, ofFloatPixels &flowVelocityPixels) {
	for (int i = 0; i < nodes.size(); i++) {
		ofVec2f coordInTex = flowVelocityTexture.getCoordFromPoint(nodes[i].get()->getPosition.x, nodes[i].get()->getPosition.y);
		ofColor texVal = flowVelocityPixels.getColor(coordInTex.x, coordInTex.y);
		ofVec2f windVelocity = ofVec2f(texVal.r, texVal.g);
		cout << "windVelocity looking like " << windVelocity.x << ", " << windVelocity.y << endl;
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
		ofSetColor(branchColor);
		joints[i].get()->draw();
	}
}

Branch::~Branch() {
}
