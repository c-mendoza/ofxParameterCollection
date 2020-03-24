#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
	
	// The mainGroup is where all of the ofParameters that will be serialized or deserialized need to be.
	mainGroup.setName("ofxPC Example");
	mainGroup.add(radius.set("Circle Radius", 10, 1, 500),
				  color.set("Circle Color", ofColor::orange));
	
	auto min = glm::vec2(0, 0);
	auto max = glm::vec2(ofGetWidth(), ofGetHeight());
	positionsCollection.setup("Position ", "Circle Positions", mainGroup, min, max);
	// Listen for button presses:
	addCircleButtton.addListener(this, &ofApp::addCircleButtonPressed);
	addCircleButtton.setup("Add Circle");
	settingsFilename = "settings.xml";
	setupGui();
}

// We are setting up the gui in a separate method to rebuild it when we deserialize or when we add a circle
void ofApp::setupGui() {
	gui.setup();
	gui.add(&addCircleButtton);
	gui.add(mainGroup);	
}

//--------------------------------------------------------------
void ofApp::update(){
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(ofColor::gray, ofColor::blueSteel);
	gui.draw();
	
	// We iterate over the collection to get all of the ofParameters
	for (auto& pos : positionsCollection) {
		ofFill();
		ofSetColor(color);
		// pos is a shared_ptr<ofParameter<glm::vec2>>, so we need to dereference it:
		ofDrawEllipse(*pos, radius, radius);
		
		// An alternative would be to use pos->get() (get() in this case is a method of
		// ofParameter that gets us the parameter's value:
		// ofDrawEllipse(pos->get(), radius, radius);
	}
	
	ofSetColor(255);
	ofDrawBitmapString("Press 'l' to load, 's' to save", 100, ofGetHeight() - 50);
}

void ofApp::exit() {
	// Cleanup:
	addCircleButtton.removeListener(this, &ofApp::addCircleButtonPressed);
}

void ofApp::addCircleButtonPressed() {
	
	// Add an item to the collection in a random location:
	positionsCollection.addItem(glm::vec2(ofRandom(0, ofGetWidth()), ofRandom(0, ofGetHeight())));
	
	// Since our data has changed, we need to rebuild the gui:
	setupGui();
}

// Standard serialization (it is in fact copied from ofxGui!)
void ofApp::serialize() {
	ofXml xml;
	if(ofFile(settingsFilename, ofFile::Reference).exists()){
		xml.load(settingsFilename);
	}
	ofSerialize(xml, mainGroup);
	xml.save(settingsFilename);
}

// Equally standard deserialization, with the exception of the
// positionsCollection.preDeserialize(xml) call.
void ofApp::deserialize() {
	ofXml xml;
	xml.load(settingsFilename);
	
	// Pre-deserialize the collection so that ofDeserialize can find the collection's parameters:
	positionsCollection.preDeserialize(xml);
	ofDeserialize(xml, mainGroup);
	
	// Our data changed so we rebuild our gui:
	setupGui();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key == 's') {
		serialize();
	}
	if(key == 'l') {
		deserialize();
	}
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
