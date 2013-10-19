/*
 *  RFID-SimpleReader.h
 *  RFID-SimpleReader
 *
 *  Created by say nono on 11.09.12.
 *  Copyright 2012 www.say-nono.com. All rights reserved.
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Serial.h"
#include <boost/signals2.hpp>
#include "cinder/utilities.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "RFID_Tag.h"

#include <sstream>


using namespace ci;
using namespace ci::app;
using namespace std;

#define BANK_USER_MEMORY_OFFSET 11;
#define RFID_MAX_DATA_LENGTH    64;

class RFID_SimpleReader{
public:

	enum RFID_COMMANDS
	{
		GET_SERIAL_NUMBER_COMMON		= 0x0101,
		GET_READER_TYPE_COMMON			= 0x0102,
		GET_HARDWARE_REVISION_COMMON	= 0x0103,
		GET_SOFTWARE_REVISION_COMMON	= 0x0104,
		GET_BOOTLOADER_REVISION_COMMON	= 0x0105,
		GET_CURRENT_SYSTEM_COMMON		= 0x0106,
		GET_CURRENT_STATE_COMMON		= 0x0107,
		GET_STATUS_REGISTER_COMMON		= 0x0108,
		GET_ANTENNA_COUNT_COMMON		= 0x0110,
		
		
		REBOOT							= 0x0301,
		SET_HEARTBEAT					= 0x0302,
		SET_ANTENNA_POWER				= 0x0303,
		RESTORE_FACTORY_SETTINGS		= 0x0320,
		SAVE_SETTINGS_PERMANENT			= 0x0321,
		SET_PARAM						= 0x0330,
		GET_PARAM						= 0x0331,
		SET_DEVICE_NAME					= 0x0332,
		GET_DEVICE_NAME					= 0x0333,
		SET_DEVICE_LOCATION				= 0x0334,
		GET_DEVICE_LOCATION				= 0x0335,
		
		SET_ANTENNA_SEQUENCE			= 0x0601,
		GET_ANTENNA_SEQUENCE			= 0x0602,
		SET_WORKING_ANTENNA				= 0x0603,
		GET_WORKING_ANTENNA				= 0x0604,
		
		ACTIVATE_NOTIFICATIONS_VERSION	= 0x1001,
		DEACTIVATE_NOTIFICATIONS		= 0x1002,
		GET_ACTIVE_NOTIFICATIONS		= 0x1003,
		
		INVENTORY_SINGLE_FUNC			= 0x5001,
		INVENTORY_CYCLIC_FUNC			= 0x5002, 
		READ_FROM_TAG_FUNC				= 0x5003,
		WRITE_TO_TAG_FUNC				= 0x5004,
		LOCK_TAG_FUNC					= 0x5005,
		KILL_TAG_FUNC					= 0x5006,
		COSTUM_TAG_COMMAND_FUNC			= 0x5010,
		READ_MULTIPLE_FROM_TAG_FUNC		= 0x5020,
		
		HEARTBEAT_INTERRUPT				= 0x9001,
		INVENTORY_CYCLIC_INTERRUPT		= 0x9002,
		STATE_CHANGED_INTERRUPT			= 0x9003,
		STATUS_REG_CHANGED_INTERRUPT	= 0x9004,
		BOOT_UP_FINISHED_INTERRUPT		= 0x9005,
		NOTIFICATION_INTERRUPT			= 0x9006,
        
	};
	
    enum HEARTBEAT
    {
        HEARTBEAT_OFF                   = 0x00,
        HEARTBEAT_ON                    = 0x01,
        HEARTBEAT_DUPLEX_ON             = 0x02
    };
    
	enum CURRENT_READER_STATE
	{
		RFE_STATE_IDLE                  = 0x00,
		RFE_STATE_REBOOTING             = 0x01,
		RFE_STATE_SCANNING              = 0x10,
		RFE_STATE_WRITING               = 0x11,
		RFE_STATE_READING               = 0x12,
	};	
	
	
	enum RFE_RET_VALUE {
		RFE_RET_SUCCESS							= 0x00, 
		RFE_RET_ERR_OP_NOT_SUPPORTED			= 0x50, 
		RFE_RET_ERR_UNKOWN_ERR					= 0x51, 
		RFE_RET_ERR_ON_EXEC_OP					= 0x52, 
		RFE_RET_ERR_COULD_NOT_WRITE				= 0x53, 
		RFE_RET_ERR_WRONG_PARAM_COUNT			= 0x54, 
		RFE_RET_ERR_WRONG_PARAM					= 0x55, 
		RFE_RET_TMI_TAG_UNREACHABLE				= 0xA0, 
		RFE_RET_TMI_MEM_OVERRUN					= 0xA1, 
		RFE_RET_TMI_MEM_LOCKED					= 0xA2, 
		RFE_RET_TMI_INSUFFICIENT_POWER			= 0xA3, 
		RFE_RET_TMI_WRONG_PASSWORD				= 0xA4
	};
    
	
	~RFID_SimpleReader();
	void setup();
	void update();
    void connectDevice();
	void scanSerialForSerialDevices();
	vector<string> getSerialDeviceList();
	
	void getCurrentState();
	
	void toggleContiniousRead();
	
	RFID_Tag* getTag(string rfid_id);
	vector<string> getTagIDs();
    void readEntireTagData(RFID_Tag* tag);
	void readTagData(RFID_Tag* tag);
	void writeTagData(RFID_Tag* tag, string str);
    void writeTagDataBank(RFID_Tag* tag,string str, int bank=0);
    
    string getAllTagsInfoString();
    string getTagInfoString(const RFID_Tag& tag);

	
	void reset();
	
	boost::signals2::signal<void(void)> sRFIDListChanged;
	
	
private:

	Serial serial;
	bool isConnected;
	vector<string> connectedSerialDevices;
	vector<char> messageIn;
	map<int,string> rfidCommandsValues;
	map<int,string> rfidReturnValues;
	
	RFID_Tag*		currentTagDataRequest;
    
	int                     currentState;
	string                  mReaderStatusString;
	map<string,RFID_Tag>    tagsMap;
    vector<string>          mTagsID;
	bool                    isContiniousReading;
	bool                    bRemoveOldChips;
	float                   mRemovalTimeSinceLastRefresh;
	
	int heartBeatCounter;
    double mConnectCounter;
    double mHeartbeatLast;
    int mHeartbeatIntervalms = 500;

	
    void reconnectToDevice();
    void removeOldEntries();
	void readSerial();
	bool validateCheckSum(int cs);
	void processMessage();
	void processState(vector<int> values);
	void processTagDataInfo(vector<int> values);
	void processInventorySingle(const vector<int>& values);
	void processInventoryCyclic(const vector<int>& values);
			
	void checkCommand();
	
	int getMessageCommand(int value);
	void getMessageValues(const vector<char>&message, vector<int>* values );
	void checkRFID_Tag(const string& hexTag, const vector<int>& tagValues);
	
	void sendCommand(int p);
	void sendCommand(int p, int v);
	void sendCommand(int p, int v[], int len);
	void sendCommandRaw(vector<char> bytes);
	
	
	string convertToHexString(const vector<int>& message, int len);
	string convertToHexString(const vector<char>& message, int len);
	
	void printMessage(const vector<int>& message);
	void printMessage(const vector<char>& message);
	
	
};


