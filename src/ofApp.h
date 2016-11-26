#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "ofxBlobTracker.h"
#include "PSEyeWrapper.hpp"

class ofApp : public ofBaseApp{

	public:
    
        void setup();
        void update();
        void draw();
        void loadFromXML();
        
        void keyPressed(int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);
        void windowResized(int w, int h);
        void dragEvent(ofDragInfo dragInfo);
        void gotMessage(ofMessage msg);
    
        void sendBlobInformation(string event, int blobId, ofVec2f blobPos) ;
        ofVec2f remapBlobIndexes(ofVec2f blobPos);
        void drawIndexLimits();
    
        void blobAdded(ofxBlob &_blob);
        void blobMoved(ofxBlob &_blob);
        void blobDeleted(ofxBlob &_blob);
    
        ofxOscSender sender;
        string destinationIP;
        int destinationPort;
    
        ofxOscSender senderInteraction;
        string destinationInteractionIP;
        int destinationInteractionPort;
    
        ofxCvColorImage			colorImg;
        ofxCvGrayscaleImage 	grayImage;
        ofxCvGrayscaleImage 	grayBg;
        ofxCvGrayscaleImage 	grayDiff;
        
        vector<PSEyeWrapper*> pseyes;
        
        ofImage					*imageJoined;
        
        ofxBlobTracker 	blobTracker;
        
        ofFbo					fbo;
        
        int 					posX, posY, camWidth, camHeight, cantCameras;
        bool					bLearnBakground;
    
        float backgroundWait;
    
        bool					drawGUI, sendI, sendB;
        
        string sendingI, sendingB;
        ofxXmlSettings XML;
        
        //Interfaz
        ofxPanel guiSender;
        ofxLabel texto;
        ofxToggle sendBlobs;
        ofxIntSlider desiredFramerate;
        int prevFrameRate;
        ofxIntSlider threshold;
        ofxIntSlider minArea;
        ofxIntSlider maxArea;
        ofxFloatSlider minYIndex;
        ofxFloatSlider maxYIndex;
        ofxFloatSlider minXIndex;
        ofxFloatSlider maxXIndex;
    
        bool loadOK;
        string errorMessage;
    
        //detected blob pos
        //ofVec2f blobPos;
		
};
