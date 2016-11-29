//
//  PSEyeWrapper.cpp
//  :
//
//  Created by Christian Clark on 4/3/16.
//
//

#include "PSEyeWrapper.hpp"

using namespace ofxCv;
using namespace cv;

const float diffThreshold = 2.5; // maximum amount of movement
const float timeThreshold = 1; // minimum time between snapshots
const int startCleaning = 10; // start cleaning outliers after this many samples


PSEyeWrapper::PSEyeWrapper(int deviceId, int posX, int posY, int rotZ, int camWidth, int camHeight, int x, int y, string cameraName, bool undistort){
    this->camWidth = camWidth;
    this->camHeight = camHeight;
    this->xPos = posX;
    this->yPos = posY;
    this->rotZ = rotZ;
    this->frameRate = 30;
    
    //interface
    this->x = x;
    this->y = y;
    this->camName = cameraName;
    
    this->camera = new ofVideoGrabber();
    this->camera->setGrabber(std::make_shared<ofxPS3EyeGrabber>());

    camera->setDeviceID(deviceId);
    
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setBusNumber(0);
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setPortNumber(0);
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setDeviceAddress(0);
    
    camera->setDesiredFrameRate(60);
    camera->setPixelFormat(OF_PIXELS_RGB);
    camera->setup(320, 240);
    
    // GUI

    autoGain.addListener(this, &PSEyeWrapper::autoGainChanged);
    autoWhiteBalance.addListener(this, &PSEyeWrapper::autoWhiteBalanceChanged);
    horizontalFlip.addListener(this, &PSEyeWrapper::horizontalFlipChanged);
    verticalFlip.addListener(this, &PSEyeWrapper::verticalFlipChanged);
    
    gui.setup(cameraName);
    gui.setShape(x, y, 320, 20);
    gui.setDefaultHeight(20);
    gui.setDefaultWidth(300);
    
    gui.add(xPos.setup("X", 0, -1000, 1000));
    gui.add(yPos.setup("Y", 0, -500, 500));
    gui.add(rot.setup("Rot", 0, -180, 180));
    gui.add(horizontalFlip.setup("HFlip",false));
    gui.add(verticalFlip.setup("VFlip",false));
    
    gui.add(autoGain.setup("Auto_Gain",true));
    gui.add(gain.setup("Gain",10,0,63));

    gui.add(autoWhiteBalance.setup("Auto_White_Balance",true));
    gui.add(rBalance.setup("RBalance",127,0,255));
    gui.add(gBalance.setup("GBalance",127,0,255));
    gui.add(bBalance.setup("BBalance",127,0,255));

    gui.add(exposure.setup("Exposure",0,0,255));
    gui.add(brigthness.setup("Brightness",0,0,255));
    gui.add(contrast.setup("Contrast",0,0,255));
    
    gui.loadFromFile("settings_" + cameraName + ".xml");
    
    prevGain=gain;
    prevBrigthness=brigthness;
    prevExposure=exposure;
    prevContrast=contrast;
    prevRBalance=rBalance;
    prevGBalance=gBalance;
    prevBBalance=bBalance;

    firstRun = true;
    
    //calibration setup
    
    ofxCv::imitate(undistorted, *(camera));
    
    if(undistort){
        
        FileStorage settings(ofToDataPath("settings.yml"), FileStorage::READ);
        
        if(settings.isOpened()) {
            int xCount = settings["xCount"], yCount = settings["yCount"];
            calibration.setPatternSize(xCount, yCount);
            float squareSize = settings["squareSize"];
            calibration.setSquareSize(squareSize);
            CalibrationPattern patternType;
            switch(settings["patternType"]) {
                case 0: patternType = CHESSBOARD; break;
                case 1: patternType = CIRCLES_GRID; break;
                case 2: patternType = ASYMMETRIC_CIRCLES_GRID; break;
            }
            calibration.setPatternType(patternType);
            calibration.load(cameraName + "_calibration.yml");
            doUndistort = undistort;
        }
    }
}


PSEyeWrapper::PSEyeWrapper(int busNumber, int portNumber, int posX, int posY, int rotZ, int camWidth, int camHeight, int x, int y, string cameraName, bool undistort){
    this->doUndistort = false;
    this->camWidth = camWidth;
    this->camHeight = camHeight;
    this->xPos = posX;
    this->yPos = posY;
    this->rotZ = rotZ;
    this->frameRate = 30;
    
    //interface
    this->x = x;
    this->y = y;
    this->camName = cameraName;
    
    this->camera = new ofVideoGrabber();
    this->camera->setGrabber(std::make_shared<ofxPS3EyeGrabber>());
    
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setBusNumber(busNumber);
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setPortNumber(portNumber);
    //this->camera->getGrabber<ofxPS3EyeGrabber>()->setDeviceAddress(deviceAddress);
    
    camera->setDesiredFrameRate(60);
    camera->setPixelFormat(OF_PIXELS_RGB);
    camera->setup(320, 240);
    
    // GUI
    
    autoGain.addListener(this, &PSEyeWrapper::autoGainChanged);
    autoWhiteBalance.addListener(this, &PSEyeWrapper::autoWhiteBalanceChanged);
    horizontalFlip.addListener(this, &PSEyeWrapper::horizontalFlipChanged);
    verticalFlip.addListener(this, &PSEyeWrapper::verticalFlipChanged);
    
    gui.setup(cameraName);
    gui.setShape(x, y, 320, 20);
    gui.setDefaultHeight(20);
    gui.setDefaultWidth(300);
    
    gui.add(xPos.setup("X", 0, -1000, 1000));
    gui.add(yPos.setup("Y", 0, -500, 500));
    gui.add(rot.setup("Rot", 0, -180, 180));
    gui.add(horizontalFlip.setup("HFlip",false));
    gui.add(verticalFlip.setup("VFlip",false));
    
    gui.add(autoGain.setup("Auto_Gain",true));
    gui.add(gain.setup("Gain",10,0,63));
    
    gui.add(autoWhiteBalance.setup("Auto_White_Balance",true));
    gui.add(rBalance.setup("RBalance",127,0,255));
    gui.add(gBalance.setup("GBalance",127,0,255));
    gui.add(bBalance.setup("BBalance",127,0,255));
    
    gui.add(exposure.setup("Exposure",0,0,255));
    gui.add(brigthness.setup("Brightness",0,0,255));
    gui.add(contrast.setup("Contrast",0,0,255));
    
    gui.loadFromFile("settings_" + cameraName + ".xml");
    
    prevGain=gain;
    prevBrigthness=brigthness;
    prevExposure=exposure;
    prevContrast=contrast;
    prevRBalance=rBalance;
    prevGBalance=gBalance;
    prevBBalance=bBalance;
    
    firstRun = true;
    
    //calibration setup

   
    ofxCv::imitate(undistorted, *(camera));
 
    if(undistort){
    
        FileStorage settings(ofToDataPath("settings.yml"), FileStorage::READ);
        
        if(settings.isOpened()) {
            int xCount = settings["xCount"], yCount = settings["yCount"];
            calibration.setPatternSize(xCount, yCount);
            float squareSize = settings["squareSize"];
            calibration.setSquareSize(squareSize);
            CalibrationPattern patternType;
            switch(settings["patternType"]) {
                case 0: patternType = CHESSBOARD; break;
                case 1: patternType = CIRCLES_GRID; break;
                case 2: patternType = ASYMMETRIC_CIRCLES_GRID; break;
            }
            calibration.setPatternType(patternType);
            calibration.load(cameraName + "_calibration.yml");
            doUndistort = undistort;
        }
    }
}

PSEyeWrapper::~PSEyeWrapper(){
    
}

void PSEyeWrapper::update() {
    camera->update();
    
    if(camera->isFrameNew()){
        if(doUndistort){
            calibration.undistort(toCv(camera->getPixels()), toCv(undistorted));
            undistorted.update();
        }
    }
    
    //updating Camera GUI parameters.
    
    updateCamFromGUI();
    
    
}

void PSEyeWrapper::updateCamFromGUI(){
    
    if(firstRun){
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setGain(gain);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setBrightness(brigthness);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setExposure(exposure);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setContrast(contrast);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setRedBalance(rBalance);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setGreenBalance(gBalance);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setBlueBalance(bBalance);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setAutogain(autoGain);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setAutoWhiteBalance(autoWhiteBalance);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setHorizontalFlip(horizontalFlip);
        this->camera->getGrabber<ofxPS3EyeGrabber>()->setVerticalFlip(verticalFlip);

        
        firstRun=false;
    }
    else{
        if(gain!=prevGain){
            this->camera->getGrabber<ofxPS3EyeGrabber>()->setGain(gain);
            prevGain = gain;
        }
        if(brigthness!=prevBrigthness){
            this->camera->getGrabber<ofxPS3EyeGrabber>()->setBrightness(brigthness);
            prevBrigthness = brigthness;
        }
        if(exposure!=prevExposure){
            this->camera->getGrabber<ofxPS3EyeGrabber>()->setExposure(exposure);
            prevExposure = exposure;
        }
        if(contrast!=prevContrast){
            this->camera->getGrabber<ofxPS3EyeGrabber>()->setContrast(contrast);
            prevContrast = contrast;
        }
        if(rBalance!=prevRBalance){
            this->camera->getGrabber<ofxPS3EyeGrabber>()->setRedBalance(rBalance);
            prevRBalance = rBalance;
        }
        if(gBalance!=prevGBalance){
            this->camera->getGrabber<ofxPS3EyeGrabber>()->setGreenBalance(gBalance);
            prevGBalance = gBalance;
        }
        if(bBalance!=prevBBalance){
            this->camera->getGrabber<ofxPS3EyeGrabber>()->setBlueBalance(bBalance);
            prevBBalance = bBalance;
        }
    }
}

void PSEyeWrapper::draw() {
    if(doUndistort){
        undistorted.draw(0,0);
    }
    else{
        camera->draw(0,0);
    }
}

void PSEyeWrapper::drawGUI() {
    ofFill();
    ofSetHexColor(0x333333);
    ofDrawBox(0, 0, 320, 20);
    ofSetHexColor(0xffffff);
    if(doUndistort){
        undistorted.draw(0,0);
    }
    else{
        camera->draw(0,0);
    }
    ofDrawBitmapString(camName, 5, 13);

}

vector<ofVideoDevice> PSEyeWrapper::getDevices(){
    ofxPS3EyeGrabber grabber;
    return grabber.listDevices();
}

void PSEyeWrapper::autoGainChanged(bool & _autoGain){
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setAutogain(_autoGain);
}

void PSEyeWrapper::autoWhiteBalanceChanged(bool & _wBalance){
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setAutoWhiteBalance(_wBalance);
}

void PSEyeWrapper::horizontalFlipChanged(bool & _hFlip){
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setHorizontalFlip(_hFlip);
}

void PSEyeWrapper::verticalFlipChanged(bool & _vFlip){
    this->camera->getGrabber<ofxPS3EyeGrabber>()->setVerticalFlip(_vFlip);
}

void PSEyeWrapper::saveSettings(){
    this->gui.saveToFile("settings_" + camName + ".xml");
}


