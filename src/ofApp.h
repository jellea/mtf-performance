#pragma once

#include <cstdlib>
#include <ctime>

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxGrt.h"
#include "ofxOsc.h"
#include "ofxMidi.h"
#include "MSAPhysics3D.h"

#define PORT 6448

using namespace GRT;


class ofApp : public ofBaseApp{
private:
    void randomizeVals();
    
public:
    void setup();
    void update();
    void draw();
    
    RegressionData trainingData;      		//This will store our training data
    GestureRecognitionPipeline pipeline;        //This is a wrapper for our classifier and any pre/post processing modules
    
    VectorFloat targetVector;
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    
    ofxOscReceiver receiver;
    ofxMidiOut midiOut;
    int channel;
};
