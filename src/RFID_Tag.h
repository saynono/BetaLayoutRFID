/*
 *  RFID_Tag.h
 *  RFID-SimpleReader
 *
 *  Created by say nono on 13.09.12.
 *  Copyright 2012 www.say-nono.com. All rights reserved.
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Serial.h"
#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include <sstream>


using namespace ci;
using namespace ci::app;
using namespace std;

using namespace boost::posix_time;
using namespace boost::gregorian;

class RFID_Tag{
	
public:
	
	void setup(string tag, const vector<int>& hexValues){
		for (int i=0; i<hexValues.size(); i++) {
			idValues.push_back(hexValues[i]);
		}
		rfid_id = tag;
//		data = "";
		timeStampFirst = App::get()->getElapsedSeconds();
		timeStampLast = timeStampFirst;
        doRevalidate = true;
        isInitialised = false;
//		console() << "NEW RFID_Tag : " << boost::posix_time::second_clock::universal_time() << std::endl;
	}
	
	void update(){
		timeStampLast = App::get()->getElapsedSeconds();
        age = timeStampLast - timeStampFirst;
	}
	
	vector<int> idValues;
	string		rfid_id;
	int			counter;
	double		timeStampFirst;
	double		timeStampLast;
	double		age;
//	string		data;
    vector<int> data;
    string      dataString;
    bool        doRevalidate;
    bool        isInitialised;
	
};


