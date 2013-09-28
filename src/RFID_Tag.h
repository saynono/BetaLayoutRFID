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
	
	void setup(string tag, vector<int> values, int tagSize){
		for (int i=0; i<tagSize; i++) {
			idValues.push_back(values[i]);
		}
		rfid_id = tag;
//		data = "";
		counter = 0;
		timeStampFirst = second_clock::universal_time(); 
		timeStampLast = second_clock::universal_time();
//		console() << "NEW RFID_Tag : " << boost::posix_time::second_clock::universal_time() << std::endl;
	}
	
	void update(){
		counter++;
		timeStampLast = second_clock::universal_time();
	}
	
	vector<int> idValues;
	string		rfid_id;
	int			counter;
	ptime		timeStampFirst;
	ptime		timeStampLast;
//	string		data;
    vector<int> data;
	
};


