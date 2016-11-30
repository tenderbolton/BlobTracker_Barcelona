#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    this->drawGUI = true;
    this->sendI = true;
    this->sendB = true;
    this->sendingI = "";
    this->sendingB = "";
    this->errorMessage="";
    this->backgroundWait = 2.0f;
    
    this->minArea = 20;
    this->maxArea = 25600;
    
    //blobPos.x = 0.0f;
    //blobPos.y = 0.0f;
    
    // dimensiones de camaras
    camWidth = 320;
    camHeight = 240;
    
    cantCameras = 0;
    
    // load port & ip
    try{
        loadFromXML();
    }
    catch(std::exception const& e){
        errorMessage = e.what();
    }
    
    if(loadOK){
    
        sender.setup(destinationIP,destinationPort);
        senderInteraction.setup(destinationInteractionIP,destinationInteractionPort);
        //tienen que ser cargados en loadFromXML
        
        colorImg.allocate(fbo.getWidth(), fbo.getHeight());
        grayImage.allocate(fbo.getWidth(), fbo.getHeight());
        grayBg.allocate(fbo.getWidth(), fbo.getHeight());
        grayDiff.allocate(fbo.getWidth(), fbo.getHeight());
        
        imageJoined = new ofImage();
        imageJoined->allocate(fbo.getWidth(),fbo.getHeight(), OF_IMAGE_GRAYSCALE);
        
        bLearnBakground = true;

        // ------------------ PSEye ------------------------
        //GUIBlobs
        guiSender.setup("Sender");
        guiSender.setShape(0, 120, 320, 20);
        guiSender.add(sendBlobs.setup("Send blobs ", false));
        guiSender.add(texto.setup("Press space to learn background",""));
        guiSender.add(threshold.setup("Threshold",80,0,200));
        guiSender.add(minArea.setup("MinArea",20,0,30000));
        guiSender.add(maxArea.setup("MaxArea",25600,0,40000));
        guiSender.add(minYIndex.setup("MinYIndex",0.0f,0.0f,1.0f));
        guiSender.add(maxYIndex.setup("MaxYIndex",1.0f,0.0f,1.0f));
        guiSender.add(minXIndex.setup("MinXIndex",0.0f,0.0f,1.0f));
        guiSender.add(maxXIndex.setup("MaxXIndex",1.0f,0.0f,1.0f));
        guiSender.add(desiredFramerate.setup("FrameRate",20,1,60));

        guiSender.loadFromFile("settings.xml");
        
        prevFrameRate = desiredFramerate;
        
        ofSetFrameRate(desiredFramerate);
        
        ofAddListener(blobTracker.blobAdded, this, &ofApp::blobAdded);
        ofAddListener(blobTracker.blobMoved, this, &ofApp::blobMoved);
        ofAddListener(blobTracker.blobDeleted, this, &ofApp::blobDeleted);
        
    }
    

}

void ofApp::loadFromXML() {
    
    loadOK = true;
    
    if (XML.loadFile("appConfig.xml")) {
        
        ofLogNotice("appConfig.xml loaded!");
        
        int numOSCSenderTags = 0;
        int numCAMs = 0;
        int numFBO = 0;
        int numBGLearn = 0;
        int numSettingsTags = XML.getNumTags("settings");
        if (numSettingsTags > 0) {
            XML.pushTag("settings", numSettingsTags - 1);
            //Blob OSC Senders
            
            int numOSCSenderTags = XML.getNumTags("OSCSenderSettingsAudio");
            
            if(numOSCSenderTags==0){
                loadOK = false;
                throw std::runtime_error(std::string("No OSC Audio configuration found! Please configure the OSC sender and restart application."));
            }
            
            if (numOSCSenderTags > 0) {
                string ip = XML.getAttribute("OSCSenderSettingsAudio", "ip", "127.0.0.1", 0);
                int port = ofToInt(XML.getAttribute("OSCSenderSettingsAudio", "port", "6000", 0));
                
                this->destinationIP = ip;
                this->destinationPort = port;
            }
            
            int numOSCSenderInteractionTags = XML.getNumTags("OSCSenderSettingsInteraction");
            
            if(numOSCSenderInteractionTags==0){
                loadOK = false;
                throw std::runtime_error(std::string("No OSC Interaction configuration found! Please configure the OSC sender and restart application."));
            }
            
            if (numOSCSenderInteractionTags > 0) {
                string ip = XML.getAttribute("OSCSenderSettingsInteraction", "ip", "127.0.0.1", 0);
                int port = ofToInt(XML.getAttribute("OSCSenderSettingsInteraction", "port", "6001", 0));
                
                this->destinationInteractionIP = ip;
                this->destinationInteractionPort = port;
            }
            
            //Tracker Configuration
            int numTracker = XML.getNumTags("Tracker");
            
            if(numTracker==0){
                loadOK = false;
                throw std::runtime_error(std::string("No Tracker defined in the XML! Please configure the Tracker and restart application."));
            }
            
            
            if (numTracker > 0) {
                this->maxBlobs = ofToInt(XML.getAttribute("Tracker", "maxBlobs", "10", 0));
                this->assignedIds = new int[maxBlobs]();
                
                //clearing all assigned ids
                for(int i=0; i<maxBlobs; i++){
                    assignedIds[i] = -1;
                }
                
            }

            
            //FBO Configuration
            numFBO = XML.getNumTags("FBO_CONFIG");
            
            if(numFBO==0){
                loadOK = false;
                throw std::runtime_error(std::string("No FBO defined in the XML! Please configure the FBO and restart application."));
            }

            
            if (numFBO > 0) {
                float width = ofToFloat(XML.getAttribute("FBO_CONFIG", "width", "320", 0));
                float height = ofToFloat(XML.getAttribute("FBO_CONFIG", "height", "720", 0));
                fbo.allocate(width, height, GL_RGB);
            }
            //BG Learn timeout
            
            numBGLearn = XML.getNumTags("BG_LEARN");
            
            if (numBGLearn > 0) {
                this->backgroundWait = ofToFloat(XML.getAttribute("BG_LEARN", "timeout", "2.0", 0));
            }

            
            //Cameras Configuration
            numCAMs = XML.getNumTags("CAMs");
            
            if(numCAMs==0){
                loadOK = false;
                throw std::runtime_error(std::string("No cameras defined in the XML! Please configure cameras and restart application."));
            }
            
            int showListCameras = ofToInt(XML.getAttribute("CAMs", "listCams", "0", 0));
            
            if(showListCameras == 0) {
            
                if (numOSCSenderTags > 0) {
                    
                    std::vector<ofVideoDevice> devices = PSEyeWrapper::getDevices();
                    
                    if(devices.size()==0){
                        loadOK = false;
                        throw std::runtime_error(std::string("No cameras detected! Please connect and configure cameras and restart application."));
                    }
                    
                    XML.pushTag("CAMs", numSettingsTags - 1);
                    
                    numCAMs = XML.getNumTags("CAM");
                    try {
                        
                        for (int i = 0; i < numCAMs; i++) {
                            int id = ofToInt(XML.getAttribute("CAM", "id", "0", i));
                            float posX = ofToFloat(XML.getAttribute("CAM", "drawX", "0", i));
                            float posY = ofToFloat(XML.getAttribute("CAM", "drawY", "0", i));
                            float rot = ofToFloat(XML.getAttribute("CAM", "rot", "0", i));
                            int guid = ofToInt(XML.getAttribute("CAM", "registered_guid", "0", i));
                            
                            int bus=ofToInt(XML.getAttribute("CAM", "bus", "0", i));
                            int port=ofToInt(XML.getAttribute("CAM", "port", "0", i));
                            int address=ofToInt(XML.getAttribute("CAM", "address", "0", i));
                            
                            bool undistort=ofToBool(XML.getAttribute("CAM", "undistort", "false", i));
                            
                            string nom = "PSEYE_" + ofToString(id);
                            
                            //PSEyeWrapper* newCam = new PSEyeWrapper(devices[guid].id, posX, posY, rot, camWidth, camHeight, 60+(i*330), 450, nom);
                            PSEyeWrapper* newCam = new PSEyeWrapper(bus, port, posX, posY, rot, camWidth, camHeight, 60+(i*330), 450, nom, undistort);
                            
                            pseyes.push_back(newCam);
                            cantCameras += 1;
                        }
                        
                    } catch (std::exception const& e){
                        loadOK = false;
                        throw std::runtime_error(std::string("Oops! Something went wrong while initializing cameras. Please check the XML camera definitions."));
                    }
                    XML.popTag();
                }
            }
            else{
                loadOK = false;
                std::vector<ofVideoDevice> cams = PSEyeWrapper::getDevices();
                
                string listing ="Camera listing: ";
                
                for (int i=0; i<cams.size(); i++){
                    listing += "\n" + cams[i].serialID;
                }
                
                throw std::runtime_error(std::string(listing));
            }
            
            
            XML.popTag();
        }
        
    }
    else {
        loadOK = false;
        throw std::runtime_error(std::string("XML not found! Please create the appConfig.xml."));
    }
}



//--------------------------------------------------------------
void ofApp::update(){
    if(loadOK){
        float elapsedTime = ofGetElapsedTimef();
        
        for (int i = 0; i < pseyes.size(); i++) {
            pseyes[i]->update();
        }
        
        //loading FBO with data
        fbo.begin();
        ofClear(0, 0, 0, 255);
        for (int i = 0; i < cantCameras; i++) {
            ofPushMatrix();
            ofTranslate(pseyes[i]->xPos, pseyes[i]->yPos, 0);
            ofRotateZ(pseyes[i]->rot);
            pseyes[i]->draw();
            ofPopMatrix();
        }
        fbo.end();
        
        ofPixels p = imageJoined->getPixels();
        fbo.readToPixels(p);
        
        colorImg.setFromPixels(p);
        colorImg.updateTexture();
        grayImage = colorImg;
        grayImage.updateTexture();
        
        //we process blobs and we send the resulting blob to through OSC
        
        if (bLearnBakground == true) {
            if(backgroundWait<=0.0f){
                blobTracker.bUpdateBackground=true;
                bLearnBakground = false;
            }
            else{
                if(ofGetElapsedTimef()>backgroundWait){
                    backgroundWait = 0.0f;
                }
            }
        }
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        
        //ofxCvGrayscaleImage& input, int _threshold, int _minArea,int _maxArea , int _nConsidered , double _hullPress , bool _bFindHoles , bool _bUseApproximation
        
        
        blobTracker.update(grayImage, threshold, minArea, maxArea, 10, 0.0f, false,false);
        
        //contourFinder.findContours(grayDiff, minArea, maxArea, 10, false, false);	// find holes
        
        
        
        //setting desired framerate
        
        if(prevFrameRate!=desiredFramerate){
            ofSetFrameRate(desiredFramerate);
            prevFrameRate=desiredFramerate;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackgroundGradient(ofColor::white, ofColor::gray);
    
    ofSetHexColor(0xffffff);
    
    if(loadOK){
        
        //grayImage.draw(10, 280);
        fbo.draw(10, 250);
        
        drawIndexLimits();
        
        for (int i = 0; i < cantCameras; i++) {
            ofPushMatrix();
            ofTranslate(10.0f+(i * 350.0f),0.0f);
            pseyes[i]->drawGUI();
            ofPopMatrix();
        }
        
        blobTracker.draw(10,250);

        // fps
        ofSetHexColor(0xffffff);
        string fpsText = "FPS: " + ofToString(ofGetFrameRate());
        ofDrawBitmapString(fpsText, 10,ofGetHeight()-20);
        
        if (this->drawGUI) {
            for (int i = 0; i < cantCameras; i++) {
                pseyes[i]->gui.draw();
            }
            this->guiSender.draw();
        }
        
        
        /*
         
         ofSetHexColor(0xffffff);
         for (int i = 0; i < cameras.size(); i++) {
         cameras[i]->draw(i * cameras[i]->getWidth(),0);
         ofDrawBitmapString(ofToString(cameras[i]->getRealFrameRate()), i * cameras[i]->getWidth() + 20, 20);
         }
         */
    }
    else{
        ofDrawBitmapString(errorMessage, 20, 20);
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(loadOK){
        switch (key) {
            case ' ':
                bLearnBakground = true;
                break;
            case 'g':
                this->drawGUI = !this->drawGUI;
                break;
            case 's':
                for (int i = 0; i < cantCameras; i++) {
                    pseyes[i]->saveSettings();
                }

                this->guiSender.saveToFile("settings.xml");
                break;
        }
    }
}

void ofApp::drawIndexLimits(){
    
    //drawing limits for indexes
    ofSetHexColor(0xff0000);
    ofPolyline lineMaxY;
    
    //max yline
    lineMaxY.addVertex(10.0f, 250.0f + (fbo.getHeight()*maxYIndex));
    lineMaxY.addVertex(10.0f + fbo.getWidth(), 250.0f + (fbo.getHeight()*maxYIndex));
    lineMaxY.draw();
    
    ofSetHexColor(0x00ff00);
    ofPolyline lineMinY;
    
    //minYline
    lineMinY.addVertex(10.0f, 250.0f + (fbo.getHeight()*minYIndex));
    lineMinY.addVertex(10.0f + fbo.getWidth(), 250.0f + (fbo.getHeight()*minYIndex));
    lineMinY.draw();
    
    ofSetHexColor(0x00ff00);
    ofPolyline lineMinX;
    
    //minXline
    lineMinX.addVertex(10.0f + (fbo.getWidth()*minXIndex), 250.0f );
    lineMinX.addVertex(10.0f + (fbo.getWidth()*minXIndex), 250.0f + fbo.getHeight());
    lineMinX.draw();
    
    ofSetHexColor(0xff0000);
    ofPolyline lineMaxX;
    //maxXline
    lineMaxX.addVertex(10.0f + (fbo.getWidth()*maxXIndex), 250.0f );
    lineMaxX.addVertex(10.0f + (fbo.getWidth()*maxXIndex), 250.0f + fbo.getHeight());
    lineMaxX.draw();
    
    ofSetHexColor(0xffffff);

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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

ofVec2f ofApp::remapBlobIndexes(ofVec2f blobPos){
    //remapping to current max and min x and y
    //truncating to limits
    if (blobPos.x>maxXIndex){
        blobPos.x=maxXIndex;
    }
    if (blobPos.x<minXIndex){
        blobPos.x=minXIndex;
    }
    if (blobPos.y>maxYIndex){
        blobPos.y=maxYIndex;
    }
    if (blobPos.y<minYIndex){
        blobPos.y=minYIndex;
    }
    
    ///remapping
    
    if((maxYIndex - minYIndex)>0){
        blobPos.y = (blobPos.y - minYIndex)/(maxYIndex - minYIndex);
    }
    else{
        blobPos.y = 0.0f;
    }
    
    if((maxXIndex - minXIndex)>0){
        blobPos.x = (blobPos.x - minXIndex)/(maxXIndex - minXIndex);
    }
    else{
        blobPos.x = 0.0f;
    }
    
    return blobPos;

}


void ofApp::blobAdded(ofxBlob &_blob){
    
    //trying to assing the blob to a slot in the assignedIds
    
    bool slotFound = false;
    int selectedSlot = -1;
    int cont = 0;
    while (!slotFound && cont<this->maxBlobs){
        if(this->assignedIds[cont]==-1){
            slotFound = true;
            selectedSlot = cont;
            this->assignedIds[cont] = _blob.id;
        }else{
            cont++;
        }
    }
    
    if(this->sendBlobs && selectedSlot!=-1){
        sendBlobInformation("new", selectedSlot, ofVec2f(_blob.centroid.x,_blob.centroid.y));
    }
}

void ofApp::blobMoved(ofxBlob &_blob){
    
    int selectedSlot = findSlot(_blob.id);
    
    if(this->sendBlobs && selectedSlot!=-1){
        sendBlobInformation("updated", _blob.id, ofVec2f(_blob.centroid.x,_blob.centroid.y));
    }
}

void ofApp::blobDeleted(ofxBlob &_blob){
    
    int selectedSlot = findSlot(_blob.id);
    this->assignedIds[selectedSlot] = -1;
    
    if(this->sendBlobs){
        sendBlobInformation("deleted", selectedSlot, ofVec2f(_blob.centroid.x,_blob.centroid.y));
    }
}

int ofApp::findSlot(int blobId){
    
    bool slotFound = false;
    int selectedSlot = -1;
    int cont = 0;
    
    while (!slotFound && cont<this->maxBlobs){
        if(this->assignedIds[cont]==blobId){
            slotFound = true;
            selectedSlot = cont;
        }else{
            cont++;
        }
    }
    
    return selectedSlot;

}

void ofApp::sendBlobInformation(string event, int blobId, ofVec2f blobPos) {
    
    //blobPos.x = blobPos.x/fbo.getWidth();
    //blobPos.y = blobPos.y/fbo.getHeight();
    //blobPos = remapBlobIndexes(blobPos);

    // sending
    
    ofxOscMessage m;
    m.setAddress("/blob/" + event);
    m.addInt32Arg(blobId);
    m.addFloatArg(blobPos.x);
    m.addFloatArg(blobPos.y);
    sender.sendMessage(m);
    senderInteraction.sendMessage(m);
}

