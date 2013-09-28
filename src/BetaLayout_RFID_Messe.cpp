//
//  BetaLayout_RFID_Messe.cpp
//  BetaLayout_RFID_Reader
//
//  Created by Vincent R. on 27.09.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "BetaLayout_RFID_Messe.h"

	
void BetaLayout_RFID_Messe::setup(){
		
}

void BetaLayout_RFID_Messe::setRFIDReader(RFID_SimpleReader* reader){
    mReader = reader;
	mReader->sRFIDListChanged.connect( boost::bind(&BetaLayout_RFID_Messe::RFIDListChange, this) );
}

void BetaLayout_RFID_Messe::reset(){
    tagsMap.clear();
    mTags.clear();
}

messeRFID* BetaLayout_RFID_Messe::getTag(string rfid_id){
    if(tagsMap.count(rfid_id)>0) return &tagsMap[rfid_id];
    return NULL;
}

void BetaLayout_RFID_Messe::writeTag(messeRFID* tag){
    string data = "  ";
    data[0] = tag->mVisitedTerminal;
    data[1] = getVisitedTags(tag);
    data.append(tag->mUserName);
    while(data.length() < 64) data.append("0");    
//    while(data.length() < 64) data.append(" ");    
    RFID_Tag* rtag = mReader->getTag(tag->rfid_id);
    if(rtag != NULL) mReader->writeTagData(rtag, data);    
}

void BetaLayout_RFID_Messe::RFIDListChange()
{    
    currentTime = boost::posix_time::second_clock::universal_time();
    bool changed = false;
    int len = mReader->mTagsID.size();
	if(len > 0){
		for ( int i=0;i<len;i++ ){
            if(processTag(mReader->mTagsID[i])) changed = true;
		}
	}
    
    mTags.clear();
    vector<string> deleteKeys;
	if(tagsMap.size() > 0){
		map<string,messeRFID >::iterator it2;
		for ( it2=tagsMap.begin() ; it2 != tagsMap.end(); it2++ ){
            messeRFID* tag = &((*it2).second);
            if(tag->mCounter <= 0) deleteKeys.push_back(tag->rfid_id);
            else{
                mTags.push_back(tag->rfid_id);
                tag->mCounter--;
            }
        }  
        changed = true;
    }
    for(int i=0;i<deleteKeys.size();i++){
        tagsMap.erase(deleteKeys[i]);
        changed = true;
    }
    if(changed) sRFIDListChanged();
}

bool BetaLayout_RFID_Messe::processTag(string tagHex){
    
    RFID_Tag* tag = mReader->getTag(tagHex);
    if(tag == NULL){
        return false;
    }
    time_duration diff = currentTime - tag->timeStampLast;
    if(diff.total_seconds() > 2){
        return false;
    }
    
//    bool changed = false;
	if(tagsMap.count(tagHex)>0){   
        tagsMap[tagHex].mCounter+=2;
        tagsMap[tagHex].mCounter = min(tagsMap[tagHex].mCounter,10);
    }else{
        console() << "+";
        if(tag->data.size() > 0){
            messeRFID mtag;		
            tagsMap[tagHex] = mtag;
            tagsMap[tagHex].rfid_id = tag->rfid_id;
            tagsMap[tagHex].mUserName = "NA";
            tagsMap[tagHex].mVisitedNXP = false;
            tagsMap[tagHex].mVisitedMurata = false;
            tagsMap[tagHex].mVisitedBeta = false;
            tagsMap[tagHex].mVisitedTerminal = 'v';            
            tagsMap[tagHex].mCounter = 1;
            tagsMap[tagHex].timeStampFirst = second_clock::universal_time(); 
            tagsMap[tagHex].timeStampLast = second_clock::universal_time();

            tagsMap[tagHex].mData.clear();
            for(int i=0;i<tag->data.size();i++) tagsMap[tagHex].mData.push_back( tag->data[i] );
            initRFIDTag(&tagsMap[tagHex]);
//            changed = true;
        }
    }
//    return changed;
    return true;
}

void BetaLayout_RFID_Messe::initRFIDTag(messeRFID* tag){
    
    string name = "";
    console() << "TAG-> "<< std::endl;
    for(int i=0;i<tag->mData.size();i++){
        console() << " " << (int)tag->mData[i];
        int val = tag->mData[i];
        if(val == 246) val = 111;
        if(val == 228) val = 97;
        if(val == 252) val = 117;
        if(val == 214) val = 79;
        if(val == 196) val = 65;
        if(val == 220) val = 85;
        
//        246 228 252 214 196 220
//        111 97 117 79 65 85
        
        char s = char(val);
        
//        console() << "  XXXXX "<< ((int)'ß') << std::endl;
//        console() << string(s);
//        if(s == 'ü') s = 'u';
//        if(s == 'ö') s = 'o';
//        if(s == 'ä') s = 'a';
//        if(s == 'Ü') s = 'U';
//        if(s == 'Ä') s = 'A';
//        if(s == 'Ö') s = 'O';
//        if(s == 'ß') s = ' ';
//        s = 'X';
//        string name = tag->mUserName;
//        //    console() << "name 1 : " << name << std::endl;
//        for(int j=0;j<name.size();j++){
//            if(name[j] == 'ü') name[j] = 'u';
//            if(name[j] == 'ö') name[j] = 'o';
//            if(name[j] == 'ä') name[j] = 'a';
//            if(name[j] == 'Ü') name[j] = 'U';
//            if(name[j] == 'Ä') name[j] = 'A';
//            if(name[j] == 'Ö') name[j] = 'O';
//            if(name[j] == 'ß') name[j] = ' ';
//        }
//        tag->mUserName = name;
        
        if(i>1) name += s;
    }
    
//    246 228 252 214 196 220
    
//    return;
    tag->mUserName = "";
//    replace(name.begin(),name.end(),'Ü','x');
    tag->mUserName = name;

    console() << std::endl;
        
    // remove all spaces at the end.
    while(tag->mUserName[tag->mUserName.size()-1] == ' ') tag->mUserName.resize(tag->mUserName.size()-1);
    while(tag->mUserName[tag->mUserName.size()-1] == '0') tag->mUserName.resize(tag->mUserName.size()-1);
    
    tag->mVisitedTerminal = tag->mData[0];
    setVisitedTags(tag, tag->mData[1]);
//    tag->mUserName = "Dummy";
    printTagInfo(tag);
}

//string BetaLayout_RFID_Messe::replaceStrChr(string str, char c1, char c2){
//    
//}

char BetaLayout_RFID_Messe::getVisitedTags(messeRFID* tag){
    char ret = 0x00;
    if(tag->mVisitedNXP)     ret |= (0x01 << VISITED_NXP);
    if(tag->mVisitedMurata)  ret |= (0x01 << VISITED_MURATA);
    if(tag->mVisitedBeta)    ret |= (0x01 << VISITED_BETA);    
//    string visitedString = "";
//    for(int i=7;i>=0;i--){
//        visitedString += ((ret >> i) & 0x01) == 1 ? "1" : "0";
//    }
    return ret;
}


void BetaLayout_RFID_Messe::setVisitedTags(messeRFID* tag, char visited){
//    string visitedString = "";
//    for(int i=7;i>=0;i--){
//        visitedString += ((visited >> i) & 0x01) == 1 ? "1" : "0";
//    }    
//    console() << "visitedString : " << visitedString << endl;
    tag->mVisitedNXP         = ((visited >> VISITED_NXP) & 0x01 == 0x01);
    tag->mVisitedMurata      = ((visited >> VISITED_MURATA) & 0x01 == 0x01);
    tag->mVisitedBeta        = ((visited >> VISITED_BETA) & 0x01 == 0x01);
}

void BetaLayout_RFID_Messe::printTagInfo(messeRFID* tag){
    console() << "========================= " << endl;
    console() << "TAG : " << tag->rfid_id << endl;
    console() << "========================= " << endl;
    console() << "User Name" << endl;
    console() << "  " << tag->mUserName << endl;    
    console() << "Visited" << endl;
    if(tag->mVisitedNXP) console() << "  " << "NXP" << endl;
    if(tag->mVisitedMurata) console() << "  " << "Murata" << endl;
    if(tag->mVisitedBeta) console() << "  " << "Beta" << endl;    
    console() << "From Terminal" << endl;
    if(tag->mVisitedTerminal == MAGIC_PCB)   console() << "  " << "MAGIC_PCB" << endl;
    else if(tag->mVisitedTerminal == PROTO_3D)    console() << "  " << "PROTO_3D" << endl;
    else if(tag->mVisitedTerminal == FRONT_PANEL) console() << "  " << "FRONT_PANEL" << endl;        
    else console() << "  " << "NONE" << endl;        
    console() << "========================= " << endl;
}


string BetaLayout_RFID_Messe::getAllTagsInfoString(){
    string str = "";
    if(tagsMap.size() > 0){
		map<string,messeRFID >::iterator it2;
		for ( it2=tagsMap.begin() ; it2 != tagsMap.end(); it2++ ){
            messeRFID* tag = &((*it2).second);
            str.append(getTagInfoString(tag));            
            str.append("\n");
        }    
    }
    return str;
}

string BetaLayout_RFID_Messe::getTagInfoString(messeRFID* tag){
    string str = "TAG ID:";
    str.append(tag->rfid_id);
    str.append("  USER:");
    str.append(tag->mUserName);

    str.append("  VISITED:");
    if(tag->mVisitedNXP) str.append(" NXP");
    if(tag->mVisitedMurata) str.append(" Murata");
    if(tag->mVisitedBeta) str.append(" Beta");    
    
    str.append("  TERMINAL:");
    if(tag->mVisitedTerminal == MAGIC_PCB)          str.append(" MAGIC_PCB");
    else if(tag->mVisitedTerminal == PROTO_3D)      str.append(" PROTO_3D");
    else if(tag->mVisitedTerminal == FRONT_PANEL)   str.append(" FRONT_PANEL");        
    
    str.append("  COUNTER:");
    str.append( toString(tag->mCounter));
    
    return str;
}


