#include "ofApp.h"
#include "MSAPhysics3D.h"
#include <iostream>
#include <string>

using namespace std;
using namespace msa::physics;

bool learning = false;
bool training = false;
bool running = false;
int selectedClass = 1;
int numInputs = 6;
int numOutputs = 12;

float inputVals[6];
float outputVals[12];

bool				mouseAttract	= false;
bool				doMouseXY		= false;		// pressing left mmouse button moves mouse in XY plane
bool				doMouseYZ		= false;		// pressing right mouse button moves mouse in YZ plane
bool				doRender		= true;
int					forceTimer		= false;

float				rotSpeed		= 0;
float				mouseMass		= 1;

int             	width;
int         		height;

//ofImage				ballImage;
World3D_ptr         world;
Particle3D_ptr      controllerNodeA;
Particle3D_ptr      controllerNodeB;
Particle3D_ptr      quarkA;
Particle3D_ptr      quarkB;




void ofApp::randomizeVals() {
    for(int i = 0; i < numOutputs; i++) {
        targetVector[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        outputVals[i] = targetVector[i];
    }
}

// Physics Defs
#define UNITS(s)                (s * ofGetWidth() / 1280.0) // scale to uniform units
#define	SPRING_MIN_STRENGTH		0.005
#define SPRING_MAX_STRENGTH		0.1

#define	SPRING_MIN_WIDTH		UNITS(1)
#define SPRING_MAX_WIDTH		UNITS(3)

#define NODE_MIN_RADIUS			UNITS(5)
#define NODE_MAX_RADIUS			UNITS(15)

#define MIN_MASS				1
#define MAX_MASS				3

#define MIN_BOUNCE				0.2
#define MAX_BOUNCE				0.9
#define	FIX_PROBABILITY			10		// % probability of a particle being fixed on creation

#define FORCE_AMOUNT			UNITS(10)

#define EDGE_DRAG				0.98

#define	GRAVITY					1

#define MAX_ATTRACTION			10
#define MIN_ATTRACTION			3

#define SECTOR_COUNT			1		// currently there is a bug at sector borders, so setting this to 1


//--------------------------------------------------------------
void ofApp::setup(){
    midiOut.listPorts();
    midiOut.openPort(0);
    
    
    // PHYSICS -------------------------------------------------
    width = ofGetWidth();
    height = ofGetHeight();
    
    // initialize our physics world
    world = World3D::create();
    
    world->setGravity(ofVec3f(0, GRAVITY, 0));
    
    // set world dimensions, not essential, but speeds up collision
    world->setWorldSize(ofVec3f(-width/2, -height, -width/2), ofVec3f(width/2, height, width/2));
    world->setSectorCount(SECTOR_COUNT);
    world->setDrag(0.97f);
    world->setDrag(1);		// FIXTHIS
    world->enableCollision();
    
    controllerNodeA = Particle3D::create();
    controllerNodeB = Particle3D::create();
    quarkA = Particle3D::create();
    quarkB = Particle3D::create();
    world->addParticle(controllerNodeA);
    world->addParticle(controllerNodeB);
    world->addParticle(quarkA);
    world->addParticle(quarkB);
    
    world->makeSpring(controllerNodeA, quarkA, ofRandom(SPRING_MIN_STRENGTH, SPRING_MAX_STRENGTH), ofRandom(10, width/2));
    world->makeSpring(controllerNodeB, quarkB, ofRandom(SPRING_MIN_STRENGTH, SPRING_MAX_STRENGTH), ofRandom(10, width/2));
    
//    
//    
//    ofBackground(255, 255, 255);
//    ofSetVerticalSync(true);
//    ofSetFrameRate(60);
//    ballImage.load("ball.png");
//    
//    //       initScene();
//    windowResized(ofGetWidth(), ofGetHeight());
//    
//    setupLighting();
    
    
    
    
    
    
    
    
    
    
    // setting random seed
    srand(static_cast <unsigned> (time(0)));
    
    //
    targetVector.resize(numOutputs);
    trainingData.setInputAndTargetDimensions(numInputs,numOutputs);
    
    MLP mlp;
 
    unsigned int numInputNeurons = trainingData.getNumInputDimensions();
    unsigned int numHiddenNeurons = 12;
    unsigned int numOutputNeurons = 1; //1 as we are using multidimensional regression
    
    //Initialize the MLP
    mlp.init(numInputNeurons, numHiddenNeurons, numOutputNeurons, Neuron::LINEAR, Neuron::SIGMOID, Neuron::SIGMOID );
    
    //Set the training settings
    mlp.setMaxNumEpochs( 1000 ); //This sets the maximum number of epochs (1 epoch is 1 complete iteration of the training data) that are allowed
    mlp.setMinChange(1.0e-10); //This sets the minimum change allowed in training error between any two epochs
    mlp.setLearningRate(0.003); //This sets the rate at which the learning algorithm updates the weights of the neural network
    mlp.setNumRandomTrainingIterations(3); //This sets the number of times the MLP will be trained, each training iteration starts with new random values
    mlp.setUseValidationSet( true ); //This sets aside a small portiion of the training data to be used as a validation set to mitigate overfitting
    mlp.setValidationSetSize( 20 ); //Use 20% of the training data for validation during the training phase
    mlp.setRandomiseTrainingOrder( true ); //Randomize the order of the training data so that the training algorithm does not bias the training
    
    //The MLP generally works much better if the training and prediction data is first scaled to a common range (i.e. [0.0 1.0])
    mlp.enableScaling( true );
    
    pipeline << MultidimensionalRegression(mlp,true);
    
    
    
    
    // OSC: listen on the given port
    cout << "listening for osc messages on port " << PORT << "\n";
    receiver.setup( PORT );
}

//--------------------------------------------------------------
void ofApp::update(){
//    width = ofGetWidth();
//    height = ofGetHeight();
//    
//    // update the world!
//    world->update();
    
    while(receiver.hasWaitingMessages())
    {
        VectorFloat inputVector(numInputs);
        VectorFloat quarkPos(numInputs);
        
        VectorFloat inputVecA(3);
        VectorFloat inputVecB(3);
        
        ofVec3f inputVecA3f(3);
        ofVec3f inputVecB3f(3);
        
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        string vals = "inputVals:";
        
        
        for(int i = 0; i < numInputs; i++) {
            inputVals[i] = m.getArgAsFloat(i);
            inputVector[i] = inputVals[i];
            vals += " " + std::to_string(inputVals[i]);
            
            if (i < 3) {
                inputVecA.push_back(inputVals[i] + 10000);
            } else {
                inputVecB.push_back(inputVals[i] + 10000);
            }
            
            string invalA;
            for(int i = 0; i < numInputs; i++) {
                invalA += "\n" + std::to_string(inputVecA[i]);
            }
            
            cout << invalA;
            
        }

        
        if (m.getArgAsInt(numInputs) == 1 || learning==true){
            trainingData.addSample(inputVector, targetVector );
        }
        
        if(training){
            if( pipeline.train( trainingData ) ){
                cout << "Pipeline Trained";
                training = false;
                running = true;
            }else cout << "WARNING: Failed to train pipeline";
        }
        
        if (pipeline.getTrained() && training){
            training = false;
            running = true;
        }
        
        if( pipeline.getTrained() && running ){
            for(int i = 0; i < 3; i++) {
                inputVecA3f[i] = inputVecA[i];
                inputVecB3f[i] = inputVecB[i];
            }
            
            controllerNodeA->moveBy(inputVecA3f);
            controllerNodeB->moveBy(inputVecB3f);
            
            ofVec3f qA = quarkA->getPosition();
            ofVec3f qB = quarkB->getPosition();

            for(int i = 0; i < 3; i++) {
                quarkPos[i] = qA[i];
                quarkPos[3+i] = qB[i];

            }
            
            pipeline.predict(quarkPos);
            
            VectorFloat regressionData;
            regressionData = pipeline.getRegressionData();
            
            for(int i = 0; i < numOutputs; i++) {
                outputVals[i] = GRT::Util::limit(regressionData[i],0.0,1.0);
            }
        }

        

        
    }
    
    // QMI: 1v per octave
    for(int i = 0; i < 4; i++) {
        midiOut << NoteOn(i+1, 27+floor(outputVals[i]*60) ,  127);
    }
    // QMI: CV1 = Aftertouch
    for(int i = 0; i < 4; i++) {
        midiOut << Aftertouch(i+1, 27+floor(outputVals[i+4]*60));
    }
    // QMI: CV2 = Modwheel CC#1
    for(int i = 0; i < 4; i++) {
        midiOut << ControlChange(i+1,1, 27+floor(outputVals[i+8]*60));
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // HUD
    
    // INPUT
    ofDrawBitmapStringHighlight("Input", 40, 20);
    
    ofDrawRectangle(40, 40, 180, 600);
    
    string txt = "Class: " + std::to_string(selectedClass);
    ofDrawBitmapStringHighlight(txt , 60, 70);
    
    string learningYesNo = learning ? "yes" : "no";
    ofDrawBitmapStringHighlight("(a) Learning? " + learningYesNo , 60, 90);
    
    string trainingYesNo = training ? "yes" : "no";
    ofDrawBitmapStringHighlight("(s) Training? " + trainingYesNo , 60, 110);
    
    string runningYesNo = running ? "yes" : "no";
    ofDrawBitmapStringHighlight("(d) Running? " + runningYesNo , 60, 130);
    
    string invals;
    for(int i = 0; i < numInputs; i++) {
        invals += "\n" + std::to_string(inputVals[i]);
    }
    
    ofDrawBitmapStringHighlight("inputVals:" +  invals, 60, 160);
    
    
    // OUTPUT
    ofDrawBitmapStringHighlight("Output", 260, 20);
    
    ofDrawRectangle(260, 40, 180, 600);
    
    ofDrawBitmapStringHighlight("(q) Randomize", 280, 70);
    
    string vals;
    for(int i = 0; i < numOutputs; i++) {
        vals += "\n" + std::to_string(outputVals[i]);
    }
    
    ofDrawBitmapStringHighlight("outputVals:" +  vals, 280, 90);
    
    stringstream text;
    text << "connected to port " << midiOut.getPort()
    << " \"" << midiOut.getName() << "\"" << endl;
    ofDrawBitmapStringHighlight(text.str(), 280, 300);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    std::cout << "key value: " << key << endl;
    
    if (key == 97){
        learning = true;
        running = false;
        training = false;
        
    }
    if (key == 115){
        training = true;
        running = false;
        learning = false;
    }
    if (key == 100){
        training = false;
        running = true;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == 97){
        learning = false;
    }
    if (key == 113){
        running = false;
        randomizeVals();
    }
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


//--------------------------------------------------------------
void ofApp::exit() {
    
    // clean up
    midiOut.closePort();
}

