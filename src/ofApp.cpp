#include "ofApp.h"

bool learning = false;
bool training = false;
bool running = false;
int selectedClass = 1;
int numInputs = 6;
int numOutputs = 12;

float inputVals[6];
float outputVals[12];


void ofApp::randomizeVals() {
    for(int i = 0; i < numOutputs; i++) {
        targetVector[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        outputVals[i] = targetVector[i];
    }
}

//--------------------------------------------------------------
void ofApp::setup(){
    
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
    mlp.setMinChange( 1.0e-4 ); //This sets the minimum change allowed in training error between any two epochs
    mlp.setLearningRate( 0.003 ); //This sets the rate at which the learning algorithm updates the weights of the neural network
    mlp.setNumRandomTrainingIterations( 1 ); //This sets the number of times the MLP will be trained, each training iteration starts with new random values
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
    while(receiver.hasWaitingMessages())
    {
        VectorFloat inputVector(numInputs);
        
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        string vals = "inputVals:";
        
        for(int i = 0; i < numInputs; i++) {
            inputVals[i] = m.getArgAsFloat(i);
            inputVector[i] = inputVals[i];
            vals += " " + std::to_string(inputVals[i]);
        }
        
        if (m.getArgAsInt(numInputs) == 1 || learning==true){
            trainingData.addSample(inputVector, targetVector );
        }
        
        if(training){
            if( pipeline.train( trainingData ) ){
                cout << "Pipeline Trained";
                training = false;
            }else cout << "WARNING: Failed to train pipeline";
        }
        
        if (pipeline.getTrained() && training){
            training = false;
        }
        
        if( pipeline.getTrained() && running ){
            pipeline.predict(inputVector);
            
            VectorFloat regressionData;
            regressionData = pipeline.getRegressionData();
            
            for(int i = 0; i < numOutputs; i++) {
                outputVals[i] = GRT::Util::limit(regressionData[i],0.0,1.0);
            }
        }
        
        for(int i = 0; i < 4; i++) {
            midiOut << NoteOn(i+1, 27+floor(outputVals[i]*60) ,  127);
        }
        
        //        for(int i = 0; i < 4; i++) {
        //            midiOut << NoteOn(i+1, 27+floor(outputVals[i]*60) ,  127);
        //        }
        
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

