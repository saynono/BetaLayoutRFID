/*
 *  RFID-SimpleReader.cpp
 *  RFID-SimpleReader
 *
 *  Created by say nono on 11.09.12.
 *  Copyright 2012 www.say-nono.com. All rights reserved.
 *
 */

#include "RFID_SimpleReader.h"

RFID_SimpleReader::~RFID_SimpleReader(){
	if(isContiniousReading) toggleContiniousRead();
}

void RFID_SimpleReader::setup(){

	rfidCommandsValues[ GET_SERIAL_NUMBER_COMMON	] = "GET_SERIAL_NUMBER_COMMON";
	rfidCommandsValues[ GET_READER_TYPE_COMMON	] = "GET_READER_TYPE_COMMON";
	rfidCommandsValues[ GET_HARDWARE_REVISION_COMMON	] = "GET_HARDWARE_REVISION_COMMON";
	rfidCommandsValues[ GET_SOFTWARE_REVISION_COMMON	] = "GET_SOFTWARE_REVISION_COMMON";
	rfidCommandsValues[ GET_BOOTLOADER_REVISION_COMMON	] = "GET_BOOTLOADER_REVISION_COMMON";
	rfidCommandsValues[ GET_CURRENT_SYSTEM_COMMON 	] = "GET_CURRENT_SYSTEM_COMMON";
	rfidCommandsValues[ GET_CURRENT_STATE_COMMON	] = "GET_CURRENT_STATE_COMMON";
	rfidCommandsValues[ GET_STATUS_REGISTER_COMMON	] = "GET_STATUS_REGISTER_COMMON";
	rfidCommandsValues[ GET_ANTENNA_COUNT_COMMON	] = "GET_ANTENNA_COUNT_COMMON";
	
	rfidCommandsValues[ REBOOT	] = "REBOOT";
	rfidCommandsValues[ SET_HEARTBEAT	] = "SET_HEARTBEAT";
	rfidCommandsValues[ SET_ANTENNA_POWER	] = "SET_ANTENNA_POWER";
	rfidCommandsValues[ RESTORE_FACTORY_SETTINGS	] = "RESTORE_FACTORY_SETTINGS";
	rfidCommandsValues[ SAVE_SETTINGS_PERMANENT	] = "SAVE_SETTINGS_PERMANENT";
	rfidCommandsValues[ SET_PARAM	] = "SET_PARAM";
	rfidCommandsValues[ GET_PARAM	] = "GET_PARAM";
	rfidCommandsValues[ SET_DEVICE_NAME	] = "SET_DEVICE_NAME";
	rfidCommandsValues[ GET_DEVICE_NAME	] = "GET_DEVICE_NAME";
	rfidCommandsValues[ SET_DEVICE_LOCATION	] = "SET_DEVICE_LOCATION";
	rfidCommandsValues[ GET_DEVICE_LOCATION	] = "GET_DEVICE_LOCATION";
	
	rfidCommandsValues[ SET_ANTENNA_SEQUENCE	] = "SET_ANTENNA_SEQUENCE";
	rfidCommandsValues[ GET_ANTENNA_SEQUENCE	] = "GET_ANTENNA_SEQUENCE";
	rfidCommandsValues[ SET_WORKING_ANTENNA	] = "SET_WORKING_ANTENNA";
	rfidCommandsValues[ GET_WORKING_ANTENNA	] = "GET_WORKING_ANTENNA";
	
	rfidCommandsValues[ ACTIVATE_NOTIFICATIONS_VERSION	] = "ACTIVATE_NOTIFICATIONS_VERSION";
	rfidCommandsValues[ DEACTIVATE_NOTIFICATIONS	] = "DEACTIVATE_NOTIFICATIONS";
	rfidCommandsValues[ GET_ACTIVE_NOTIFICATIONS	] = "GET_ACTIVE_NOTIFICATIONS";
	
	rfidCommandsValues[ INVENTORY_SINGLE_FUNC	] = "INVENTORY_SINGLE_FUNC";
	rfidCommandsValues[ INVENTORY_CYCLIC_FUNC	] = "INVENTORY_CYCLIC_FUNC";
	rfidCommandsValues[ READ_FROM_TAG_FUNC	] = "READ_FROM_TAG_FUNC";
	rfidCommandsValues[ WRITE_TO_TAG_FUNC	] = "WRITE_TO_TAG_FUNC";
	rfidCommandsValues[ LOCK_TAG_FUNC	] = "LOCK_TAG_FUNC";
	rfidCommandsValues[ KILL_TAG_FUNC	] = "KILL_TAG_FUNC";
	rfidCommandsValues[ COSTUM_TAG_COMMAND_FUNC	] = "COSTUM_TAG_COMMAND_FUNC";
	rfidCommandsValues[ READ_MULTIPLE_FROM_TAG_FUNC	] = "READ_MULTIPLE_FROM_TAG_FUNC";
	
	rfidCommandsValues[ HEARTBEAT_INTERRUPT	] = "HEARTBEAT_INTERRUPT";
	rfidCommandsValues[ INVENTORY_CYCLIC_INTERRUPT	] = "INVENTORY_CYCLIC_INTERRUPT";
	rfidCommandsValues[ STATE_CHANGED_INTERRUPT	] = "STATE_CHANGED_INTERRUPT";
	rfidCommandsValues[ STATUS_REG_CHANGED_INTERRUPT	] = "STATUS_REG_CHANGED_INTERRUPT";
	rfidCommandsValues[ BOOT_UP_FINISHED_INTERRUPT	] = "BOOT_UP_FINISHED_INTERRUPT";
	rfidCommandsValues[ NOTIFICATION_INTERRUPT	] = "NOTIFICATION_INTERRUPT";
	
	
	
	
	rfidReturnValues[0x00] = "RFE_RET_SUCCESS";
	rfidReturnValues[0x50] = "RFE_RET_ERR_OP_NOT_SUPPORTED";
	rfidReturnValues[0x51] = "RFE_RET_ERR_UNKOWN_ERR";
	rfidReturnValues[0x52] = "RFE_RET_ERR_ON_EXEC_OP";
	rfidReturnValues[0x53] = "RFE_RET_ERR_COULD_NOT_WRITE";
	rfidReturnValues[0x54] = "RFE_RET_ERR_WRONG_PARAM_COUNT";
	rfidReturnValues[0x55] = "RFE_RET_ERR_WRONG_PARAM";
	rfidReturnValues[0xA0] = "RFE_RET_TMI_TAG_UNREACHABLE";
	rfidReturnValues[0xA1] = "RFE_RET_TMI_MEM_OVERRUN";
	rfidReturnValues[0xA2] = "RFE_RET_TMI_MEM_LOCKED";
	rfidReturnValues[0xA3] = "RFE_RET_TMI_INSUFFICIENT_POWER";
	rfidReturnValues[0xA4] = "RFE_RET_TMI_WRONG_PASSWORD";
			
	isConnected = false;
	isContiniousReading = false;
	
    mConnectCounter = 0;
	heartBeatCounter = 0;
	currentState = -1;
	mReaderStatusString = "N/A";
    bRemoveOldChips = true;
    mRemovalTimeSinceLastRefresh = 20.f;
    
    reconnectToDevice();
	
}

void RFID_SimpleReader::reconnectToDevice(){
    mConnectCounter = 0;
	// print the devices
	scanSerialForSerialDevices();
	try {
		Serial::Device dev = Serial::findDeviceByNameContains("tty.usbmodem");
		serial = Serial( dev, 115200);
		console() << "Connected to " << dev.getName() << "\n";
	}
	catch( ... ) {
		console() << "There was an error initializing the serial device!" << std::endl;
		isConnected = false;
		return;
	}
	isConnected = true;
	int durHeartBeat = 500;
	int vals[] = {0x01, 0xff&(durHeartBeat>>8), 0xff&durHeartBeat};
	sendCommand( SET_HEARTBEAT , vals , 3);
}

void RFID_SimpleReader::update(){
	if(!isConnected){
        mConnectCounter++;
        console() << "mConnectCounter : " << mConnectCounter << std::endl;
        if(mConnectCounter > 100) reconnectToDevice();
      return;  
    }
	// clear accumulated contact messages in buffer
	readSerial();
}

void RFID_SimpleReader::reset(){
	tagsMap.clear();
    mTagsID.clear();
}

void RFID_SimpleReader::toggleContiniousRead(){
	isContiniousReading = !isContiniousReading;
	if(isContiniousReading){
		sendCommand(INVENTORY_CYCLIC_FUNC, 0x01);
	}else{
		sendCommand(INVENTORY_CYCLIC_FUNC, 0x00);
	}
	
	console() << "toggleContiniousRead = " << (isContiniousReading?"TRUE\n" : "FALSE\n");
	
}

void RFID_SimpleReader::removeOldEntries(){
    
    if(!bRemoveOldChips) return;
    ptime timeCurrent = second_clock::universal_time();

    RFID_Tag* tag;
    vector<string> deleteKeys;
    map<string,RFID_Tag >::iterator it;
    for ( it=tagsMap.begin() ; it != tagsMap.end(); it++ ){
        tag = & (*it).second;
        time_duration age = timeCurrent - tag->timeStampLast;
        if(age.seconds() > mRemovalTimeSinceLastRefresh) deleteKeys.push_back(tag->rfid_id);
    }
    
    for(int i=0;i<deleteKeys.size();i++){
//        console() << "REMOVE TAG " << deleteKeys[i] << std::endl;
        tagsMap.erase(deleteKeys[i]);
    }
    deleteKeys.clear();

}

void RFID_SimpleReader::readSerial(){
//	console() << serial.getNumBytesAvailable();
	while(serial.getNumBytesAvailable() > 0)
	{
        try {
            char b = serial.readByte();
            if(validateCheckSum(b)) processMessage();
            else messageIn.push_back(b);
        }
        catch( ... ) {
            isConnected = false;
            console() << "Error... Serial not there anymore!" << std::endl;
            return;
        }
	}	
}

void RFID_SimpleReader::processMessage(){
	
    
	int msgCommand = getMessageCommand((messageIn[4]<<8)+messageIn[5]);
	vector<int> values = getMessageValues(messageIn);
    
	switch(msgCommand){
		case GET_CURRENT_STATE_COMMON:
			processState(values);
			break;
		case SET_HEARTBEAT:
//			console() << "HEARTBEAT\n";
			break;
		case HEARTBEAT_INTERRUPT:
//			console() << "HEARTBEAT_INTERRUPT\n";
			if(heartBeatCounter%4==0){
				sendCommand(GET_CURRENT_STATE_COMMON);
			}
            
//			if(!isContiniousReading && heartBeatCounter%2==0){
				sendCommand(INVENTORY_SINGLE_FUNC);
//			}
			heartBeatCounter++;
			heartBeatCounter %= 60;
			break;
		case INVENTORY_SINGLE_FUNC:
			processInventorySingle(values);
			break;
		case INVENTORY_CYCLIC_FUNC:
			break;
		case INVENTORY_CYCLIC_INTERRUPT:
			processInventoryCyclic(values);
		case READ_FROM_TAG_FUNC:
//			console() << "\n\n===============================================\n";
//			console() << "READ_FROM_TAG_FUNC => " + convertToHexString(values,values.size()) + "   = " + rfidReturnValues[values[0]]+"\n";
//			console() << "===============================================\n";
			processTagDataInfo(values);
			break;
		case READ_MULTIPLE_FROM_TAG_FUNC:
//            console() << "\n\n===============================================\n";
//            console() << "READ_FROM_TAG_FUNC => " + convertToHexString(values,values.size()) + "   = " + rfidReturnValues[values[0]]+"\n";
//            printf("Reading RAW  "); printMessage(messageIn);
//            console() << "===============================================\n";
			processTagDataInfo(values);
			break;
			
		case WRITE_TO_TAG_FUNC:
			console() << "\n\n===============================================\n";
			console() << "WRITE_TO_TAG_FUNC => " + convertToHexString(values,values.size()) + "   = " + rfidReturnValues[values[0]]+"\n";
			console() << "===============================================\n";
			break;
			
		default:
			console() << "\n\n===============================================\nUNKNOWN COMMAND =>";
			printf(" 0x%04X    ( %s )\n",msgCommand,rfidCommandsValues[msgCommand].c_str());
			printf("Reading RAW  "); printMessage(messageIn);
			console() << "===============================================\n";
			break;
	}
	
	messageIn.clear();
}


void RFID_SimpleReader::processState(vector<int> values){
	switch(values.at(0)){
		case RFE_STATE_IDLE:
			currentState = RFE_STATE_IDLE;
			mReaderStatusString = "IDLE";
			isContiniousReading = false;
			break;
		case RFE_STATE_REBOOTING:
			currentState = RFE_STATE_REBOOTING;			
			mReaderStatusString = "REBOOTING";
			break;
		case RFE_STATE_SCANNING:
			currentState = RFE_STATE_SCANNING;			
			mReaderStatusString = "SCANNING";
			isContiniousReading = true;
			break;
		case RFE_STATE_WRITING:
			currentState = RFE_STATE_WRITING;			
			mReaderStatusString = "WRITING";
			break;
		case RFE_STATE_READING:
			currentState = RFE_STATE_READING;
			mReaderStatusString = "READING";
			break;
		default:
			currentState = -1;			
			mReaderStatusString = "N/A";
			break;
	}
}

void RFID_SimpleReader::processTagDataInfo(vector<int> values){
	if(values[0] == RFE_RET_SUCCESS){
        currentTagDataRequest->data.clear();
        int len = values.size();
        int offset = 0;
        string retStr = "DATA : ";
		for(int j=2;j<len;j++){
            if(offset%9==0){
                retStr += "    BANK";
                retStr += toString((int)(offset/9));
                retStr += " : ";
            }   
            currentTagDataRequest->data.push_back(values[j]);
            char val[4];
            sprintf(val,"%02X ",values[j]); 
            retStr += val;
            offset++;
        }
//        printf("%s\n",retStr.c_str());
	}else{
		console() << "READ_ERROR : " << rfidReturnValues[values[0]] << "\n";
	}	
}

void RFID_SimpleReader::processInventorySingle(vector<int> values){
    removeOldEntries();
	int idCount = values.at(1);
	int packetIdCount = values.at(1);
	if(idCount>0){
		values.erase (values.begin(),values.begin()+3);
		for (int i=0; i<idCount; i++) {
			int tagSize = values.at(2);
			if(values.size() > tagSize){
				values.erase (values.begin(),values.begin()+1 + 2);
				checkRFID_Tag(values,tagSize);
				values.erase (values.begin(),values.begin()+tagSize);
			}else{
			}
		}
	}	
    mTagsID.clear();
	RFID_Tag* tag;
	map<string,RFID_Tag >::iterator it;
	for ( it=tagsMap.begin() ; it != tagsMap.end(); it++ ){
		tag = & (*it).second;
		mTagsID.push_back(tag->rfid_id);
	}
	sRFIDListChanged();
}

void RFID_SimpleReader::processInventoryCyclic(vector<int> values){
	printf("processInventoryCyclic RAW  "); printMessage(values);
	if(values.size() == 0) return;
	int idCount = values.at(0);
	int packetIdCount = values.at(0);
	if(idCount>0){
		values.erase (values.begin(),values.begin()+1);
		for (int i=0; i<idCount; i++) {
			int tagSize = values.at(0);
			if(values.size() > tagSize){
				values.erase (values.begin(),values.begin()+1);
				checkRFID_Tag(values,tagSize);
				values.erase (values.begin(),values.begin()+tagSize);
			}else{
			}
		}
	}	
//	checkRFID_Tag(values,values.size());
	sRFIDListChanged();
}


void RFID_SimpleReader::checkRFID_Tag(vector<int> values, int tagSize){
	
	string tagHex = convertToHexString(values,tagSize);    
	if(tagsMap.count(tagHex)){
//		printf("TAG EXISTS ALREADY : %s\n",tagHex.c_str());
        if(tagsMap[tagHex].data.size() == 0){
            readEntireTagData(&tagsMap[tagHex]);
        }
		tagsMap[tagHex].update();
	}else{
//		printf("TAG IS NEW\n");
		RFID_Tag tag;		
		tagsMap[tagHex] = tag;
		tagsMap[tagHex].setup(tagHex,values,tagSize);
//		readTagData(&tagsMap[tagHex]);
        readEntireTagData(&tagsMap[tagHex]);
	}
}






int RFID_SimpleReader::getMessageCommand(int value) {
	value = 0xffff & value;
	if(rfidCommandsValues.count(value) > 0) return value;
//	for(int i=0;i<rfidCommands.size();i++){
//		if(rfidCommands[i] == value) return rfidCommands[i];
//	}
	return -1;
}

vector<int> RFID_SimpleReader::getMessageValues(vector<char> message){
        
	int amount = message[7];
    if(message.size()-10 != amount){
        printf("|ERROR| Reading RAW  "); printMessage(messageIn);
        console() << "|ERROR| Something fishy here!!! is: " << (message.size()-9) << "  should be: " << amount << endl;
        printf("|ERROR| RFID_SimpleReader::getMessageValues  "); printMessage(message);	
        amount = message.size()-10;
    }
	vector<int> values;
	for (int i=0; i<amount; i++) {
		values.push_back( 0xff & messageIn.at(i+9) );
	}
	return values;
}


bool RFID_SimpleReader::validateCheckSum(int value) {
	if(messageIn.size() < 11) return false;	
	if(messageIn.at(messageIn.size()-1) != 4) return false;
	int cs = messageIn.at(0);
	for(int i=1;i<messageIn.size();i++){
		cs ^= messageIn.at(i);
	}
	return cs == value;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++				GETTERS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void RFID_SimpleReader::getCurrentState(){
	sendCommand(GET_CURRENT_STATE_COMMON);
}

RFID_Tag* RFID_SimpleReader::getTag(string rfid_id){
//	RFID_Tag* tag;
//	map<string,RFID_Tag >::iterator it;
//	for ( it=tagsMap.begin() ; it != tagsMap.end(); it++ ){
//		tag = & (*it).second;
//		if(tag->rfid_id.compare(rfid_id) == 0) return tag;
//		break;
//	}
    if(tagsMap.count(rfid_id) > 0) return &(tagsMap[rfid_id]); 
	return NULL;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++				READING
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RFID_SimpleReader::readEntireTagData(RFID_Tag* tag){
	currentTagDataRequest = tag;
	int lenTagHex = tag->idValues.size();
	int lenData = 8;
    lenData = 64;
    int banks = 1;
	int lenTot = 1 + lenTagHex + 8 * banks;
	int send[lenTot];
	int pos = 0;
	
	// tagIdCount			= 1b
	send[pos++] = lenTagHex;
	// tagId				= 03-23-5f-5d-34-16-00-00-00 ex
	for (int i=0; i<lenTagHex; i++) {
		send[pos++] = 0xff & tag->idValues.at(i);
	}
	
//	// memoryBank			= 0x01			, 1b
//	send[pos++] = 0x00;
//	
//	// startAddress			= 0x0000		, 2b
//	send[pos++] = 0;
//	send[pos++] = 0;
//	
//	// accessPassword		= 0x00000000	, 4b
//	send[pos++] = 0;
//	send[pos++] = 0;
//	send[pos++] = 0;
//	send[pos++] = 0;
//	
//	// bytesCount			= databytecound	, 1b
//	send[pos++] = lenData;

    for(int i=0;i<banks;i++){
        /////////
        // memoryBank			= 0x01			, 1b
        send[pos++] = i+3;
	
        // startAddress			= 0x0000		, 2b
        send[pos++] = 0;
        send[pos++] = 0;
	
        // accessPassword		= 0x00000000	, 4b
        send[pos++] = 0;
        send[pos++] = 0;
        send[pos++] = 0;
        send[pos++] = 0;
	
        // bytesCount			= databytecound	, 1b
        send[pos++] = lenData;
    }

	sendCommand(READ_MULTIPLE_FROM_TAG_FUNC, send, lenTot);	
}

void RFID_SimpleReader::readTagData(RFID_Tag* tag){
	currentTagDataRequest = tag;
	int lenTagHex = tag->idValues.size();
	int lenData = 8;
	int lenTot = 9 + lenTagHex;
	int send[lenTot];
	int pos = 0;
	
	// tagIdCount			= 1b
	send[pos++] = lenTagHex;
	// tagId				= 03-23-5f-5d-34-16-00-00-00 ex
	for (int i=0; i<lenTagHex; i++) {
		send[pos++] = 0xff & tag->idValues.at(i);
	}
	
	// memoryBank			= 0x01			, 1b
	send[pos++] = 3;
	
	// startAddress			= 0x0000		, 2b
	send[pos++] = 0;
	send[pos++] = 0;
	
	// accessPassword		= 0x00000000	, 4b
	send[pos++] = 0;
	send[pos++] = 0;
	send[pos++] = 0;
	send[pos++] = 0;
	
	// bytesCount			= databytecound	, 1b
	send[pos++] = lenData;
	
	sendCommand(READ_FROM_TAG_FUNC, send, lenTot);	
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++				WRITING
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RFID_SimpleReader::writeTagData(RFID_Tag* tag,string str){
    
    console() << " || RFID_SimpleReader::writeTagData : " << str << endl;
    writeTagDataBank(tag,str,0);
//    return;
//    int lenData = (int)str.size();
////    lenData = 16;
//    int bankCounter = 0;
//    for(int i=0;i<lenData;i+=8){
//        string data = str.substr(i,8);
//        writeTagDataBank(tag,data,bankCounter);
//        bankCounter++;
//    }
}

void RFID_SimpleReader::writeTagDataBank(RFID_Tag* tag,string str, int bank){
    //    str = "jojo";
    
    bank = bank+BANK_USER_MEMORY_OFFSET;
    console() << " || RFID_SimpleReader::writeTagDataBank " << bank << " : " << str << endl;

	int lenTagHex = tag->idValues.size();
//	int lenData = min(8,(int)str.size());
	int lenData = (int)str.size();
	int lenTot = 9 + lenTagHex + lenData;
	int send[lenTot];
	int pos = 0;
	
	// tagIdCount			= 1b
	send[pos++] = lenTagHex;
	// tagId				= 03-23-5f-5d-34-16-00-00-00 ex
	for (int i=0; i<lenTagHex; i++) {
		send[pos++] = 0xff & tag->idValues.at(i);
	}
	
	// memoryBank			= 0x01			, 1b
	send[pos++] = bank;//bank;
	
	// startAddress			= 0x0000			, 2b
	send[pos++] = 0;
	send[pos++] = 0;
    
	// accessPassword		= 0x00000000	, 4b
	send[pos++] = 0;
	send[pos++] = 0;
	send[pos++] = 0;
	send[pos++] = 0;
	
	// bytesCount			= databytecound	, 1b
	send[pos++] = lenData;
	
	// data				= 0f3f9319....	data as char array
	for (int i=0; i<lenData; i++) {
		send[pos++] = 0xff & str.at(i);
	}
	
	sendCommand(WRITE_TO_TAG_FUNC, send, lenTot);
    
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++				SERIAL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void RFID_SimpleReader::scanSerialForSerialDevices(){
	const vector<Serial::Device> &devices( Serial::getDevices() );
	for( vector<Serial::Device>::const_iterator deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt ) {
		console() << "Device: " << deviceIt->getName() << endl;
		connectedSerialDevices.push_back(deviceIt->getName());
	}
}


vector<string> RFID_SimpleReader::getSerialDeviceList(){
	return connectedSerialDevices;
}
	
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++				SENDING TO READER
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RFID_SimpleReader::sendCommand(int p){
	char bytes[] = {static_cast<char>(0xff & (p >> 8)), static_cast<char>((0xff & p))};
	char arr[] = {0x52, 0x46, 0x45, 0x01, bytes[0], bytes[1], 0x02, 0x00, 0x04}; 
	vector<char>  b(arr, arr + sizeof(arr) / sizeof(arr[0]) );
	sendCommandRaw(b);			
}

void RFID_SimpleReader::sendCommand(int p, int v){
	char bytes[] = {static_cast<char>(0xff & (p >> 8)), static_cast<char>((0xff & p))};
	console() << bytes[0] << "    " << bytes[1];
	char arr[] = {0x52, 0x46, 0x45, 0x01, bytes[0], bytes[1], 0x02, 0x01, 0x03, static_cast<char>(v), 0x04};
	vector<char>  b(arr, arr + sizeof(arr) / sizeof(arr[0]) );
	sendCommandRaw(b);			
}

void RFID_SimpleReader::sendCommand(int p, int v[], int len){
	char bytes[] = {static_cast<char>(0xff & (p >> 8)), static_cast<char>((0xff & p))};
	char arr[] = {0x52, 0x46, 0x45, 0x01, bytes[0], bytes[1], 0x02, static_cast<char>(len), 0x03};
	vector<char>  b(arr, arr + sizeof(arr) / sizeof(arr[0]) );
	for (int i=0; i<len; i++) {
		b.push_back(0xff&v[i]);			
	}
	b.push_back(0x04);	
	sendCommandRaw(b);			
}

void RFID_SimpleReader::sendCommandRaw(vector<char> bytes) {
	if(!isConnected) return;
	char cs = bytes[0];
	for(int i=1;i<bytes.size();i++){
		cs ^= bytes[i];
	}
	if(bytes[4] == 0x50 && bytes[5] == 0x04) printf("Sending RAW => %s  \n",convertToHexString(bytes,bytes.size()).c_str());
	bytes.push_back(cs);
    
	try {
        serial.writeBytes( &bytes[0], bytes.size() );
	}
	catch( ... ) {
        isConnected = false;
//        serial.close();
		console() << "Error... Serial not there anymore!" << std::endl;
		return;
	}
    
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++				UTILS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


string RFID_SimpleReader::convertToHexString(vector<int> message, int len){
	char buffer [2];
	string hexStr = "";
	for(int i=0;i<len;i++){
		sprintf (buffer, "%02X",0xff&message.at(i));
		hexStr += buffer;
		if(i<len-1) hexStr += "-";
	};
	return hexStr;
}

string RFID_SimpleReader::convertToHexString(vector<char> message, int len){
	char buffer [2];
	string hexStr = "";
	for(int i=0;i<len;i++){
		sprintf (buffer, "%02X",0xff&message.at(i));
		hexStr += buffer;
		if(i<len-1) hexStr += "-";
	};
	return hexStr;
}


void RFID_SimpleReader::printMessage(vector<int> message){
	if(message.size() == 0) return;
	string str = convertToHexString(message,message.size());
	printf("%s\n",str.c_str());
//	for(int i=1;i<message.size();i++){
//		printf("-%02X",0xff&message[i]);
//	}
}

void RFID_SimpleReader::printMessage(vector<char> message){
	if(message.size() == 0) return;
	printf("%02X",message[0]);
	for(int i=1;i<message.size();i++){
		printf("-%02X",0xff&message[i]);
	}
	printf("\n");
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++				UTILS - TAG INFO
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



string RFID_SimpleReader::getAllTagsInfoString(){
    string str = "";
    if(tagsMap.size() > 0){
		map<string,RFID_Tag >::iterator it2;
		for ( it2=tagsMap.begin() ; it2 != tagsMap.end(); it2++ ){
            RFID_Tag* tag = &((*it2).second);
            str.append(getTagInfoString(tag));            
            str.append("\n");
        }    
    }
    return str;
}

string RFID_SimpleReader::getTagInfoString(RFID_Tag* tag){
    string str = "TAG ID:";
    str.append(tag->rfid_id);
    str.append("  DATA_LEN:");
    str.append( toString((int)tag->data.size()));
    str.append("  COUNTER:");
    str.append( toString(tag->counter));
    
    
    return str;
}
