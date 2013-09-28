//
//  BetaLayout_RFID_Messe.h
//  BetaLayout_RFID_Reader
//
//  Created by Vincent R. on 27.09.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/utilities.h"
#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "BetaLayout_RFID_Messe.h"
#include "RFID_SimpleReader.h"
#include "RFID_Tag.h"

#include <algorithm>


using namespace ci;
using namespace ci::app;
using namespace std;



struct messeRFID{
    
//    RFID_Tag*       tag;
    
    string          rfid_id;    
    string          mUserName;
    
    vector<int>     mData;
    
    bool            mVisitedMurata;
    bool            mVisitedNXP;
    bool            mVisitedBeta;
    
    char            mVisitedTerminal;
    
    int             mCounter;
    
    ptime           timeStampFirst;
	ptime           timeStampLast;

    
};

class BetaLayout_RFID_Messe{

public:

    ///////////////////
    /// public variables
    
    enum TAGS_VISITED
	{
		VISITED_NXP         = 0x00,
		VISITED_MURATA      = 0x01, 
		VISITED_BETA        = 0x02
	};	

    enum TERMINAL_VISITED
	{
		MAGIC_PCB           = 'M',
		PROTO_3D            = 'D', 
		FRONT_PANEL         = 'F'
	};
    
    
    map<string,messeRFID>   tagsMap;
    vector<string>          mTags;
	boost::signals2::signal<void(void)> sRFIDListChanged;
    
    ///////////////////
    /// public functions
    
	void setup();
    void reset();
    
    void RFIDListChange();
    void setRFIDReader(RFID_SimpleReader* reader);
    
    messeRFID* getTag(string rfid_id);
    void writeTag(messeRFID* tag);

    void setVisitedTags(messeRFID* tag, char visited);
    char getVisitedTags(messeRFID* tag);
    
    void printTagInfo(messeRFID* tag);
    string getTagInfoString(messeRFID* tag);
    string getAllTagsInfoString();
    
        
private:
    
    ///////////////////
    /// private variables
     
    RFID_SimpleReader*          mReader;
    ptime                       currentTime;

//    vector<messeRFID>           mTags;
    

    ///////////////////
    /// private functions
    
    bool processTag(string tagHex);
    void initRFIDTag(messeRFID* tag);
};



