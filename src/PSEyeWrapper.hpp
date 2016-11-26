//
//  PSEyeWrapper.hpp
//  inf_srv
//
//  Created by Christian Clark on 4/3/16.
//
//

#ifndef PSEyeWrapper_hpp
#define PSEyeWrapper_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxPS3EyeGrabber.h"
#include "ofxGui.h"
#include "ofxCv.h"

class PSEyeWrapper{
public:
    PSEyeWrapper(int deviceId, int posX, int posY, int rotZ, int camWidth, int camHeight, int x, int y, string cameraName, bool undistort);
    PSEyeWrapper(int busNumber, int portNumber, int posX, int posY, int rotZ, int camWidth, int camHeight, int x, int y, string cameraName, bool undistort);
    ~PSEyeWrapper();
    
    static vector<ofVideoDevice> getDevices();
    
    // GUI functions
    int changeValue(int oldPos, int newPos);

    void autoGainChanged(bool & _autoGain);
    void autoWhiteBalanceChanged(bool & _wBalance);
    void horizontalFlipChanged(bool & _hFlip);
    void verticalFlipChanged(bool & _vFlip);
    
    
    void update();
    void draw();
    void drawGUI();
    void updateCamFromGUI();
    
    // GUI controls
    ofxFloatSlider xPos;
    ofxFloatSlider yPos;
    ofxFloatSlider rot;
    ofxIntSlider gain;
    ofxIntSlider  brigthness;
    ofxIntSlider  contrast;
    ofxIntSlider exposure;
    ofxIntSlider rBalance;
    ofxIntSlider gBalance;
    ofxIntSlider bBalance;
    
    ofxPanel gui;
    ofxToggle autoGain;
    ofxToggle autoWhiteBalance;
    ofxToggle horizontalFlip;
    ofxToggle verticalFlip;

    
    string camName;
    
    ofVideoGrabber*	camera;
    
    //camera
    int			camWidth, camHeight, frameRate;
    
    //interface
    int			x, y, rotZ;
    
    //calibration attributs
    
    ofImage undistorted;

    bool doUndistort;
    
    ofxCv::Calibration calibration;
    
    //end calibation attributes
    
private:
    int prevGain;
    int prevBrigthness;
    int prevExposure;
    int prevContrast;
    int prevRBalance;
    int prevGBalance;
    int prevBBalance;
    
    bool firstRun;
    
};


#endif /* PSEyeWrapper_hpp */
