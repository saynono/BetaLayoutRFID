#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "cinder/utilities.h"

#include "BetaLayout_RFID_Messe.h"
#include "RFID_SimpleReader.h"
#include "RFID_Tag.h"


using namespace ci;
using namespace ci::app;
using namespace std;


class BetaLayoutRFIDApp : public AppBasic {
public:
    void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();
    void reset();
	
	void RFIDListChange();
	
private:
    
	RFID_SimpleReader           reader;
    BetaLayout_RFID_Messe       mRFIDMesse;
	
	float						mFrameRate;
	ci::params::InterfaceGl		mParams;
	std::vector<std::string>	mSerialConnections;
	std::vector<std::string>	*mMeshTitles;
	int32_t						mMeshIndex;
	int32_t						mTagIndexCurrent;
	int32_t						mTagIndex;
	gl::Texture					mTextTexture;
	Vec2f						mSize;
	Font						mFont;
	bool						bTextureComplete;
    bool                        bRefreshList;
    bool                        bHasRFIDTags;
	vector<string>				mTagDescriptionList;
	map<string,char>            mTagDescriptionMap;
	map<char,int>               mTagDescriptionMapIDReverse;
    int                         mTagListLength;
    
    string                      mChipTagName;
    bool                        mChipTagStandMurata;
    bool                        mChipTagStandNXP;
    bool                        mChipTagStandBeta;
	vector<string>				mChipTagComingFromList;
    int                         mChipTagComingFromID;
    
    string                      mCurrentChipID;
    
    
    void                        toggleMurata();
    void                        toggleNXP();
    void                        toggleBeta();
    
    void                        refreshParameterList();
	void						toggleContiniousRead();
	void						resetReader();
    void                        processTag(RFID_Tag* tag);
	void						writeTagData();
    
};

void BetaLayoutRFIDApp::prepareSettings( Settings *settings )
{
	settings->setResizable( true );
    settings->setTitle("BetaLayout RFID Reader");
    settings->setFrameRate(15);
}

void BetaLayoutRFIDApp::setup()
{
    reader.setup();
    mRFIDMesse.setup();
    mRFIDMesse.setRFIDReader(&reader);
    
    reader.sRFIDListChanged.connect( boost::bind(&BetaLayoutRFIDApp::RFIDListChange, this) );
    mRFIDMesse.sRFIDListChanged.connect( boost::bind(&BetaLayoutRFIDApp::RFIDListChange, this) );
        
	mMeshIndex = reader.getSerialDeviceList().size()-1;
	mTagIndex = 0;
	mTagIndexCurrent = 0;
    mTagListLength = 0;
    
    mChipTagName = "not read yet";
    mChipTagComingFromID = 0;
    mChipTagStandMurata = false;
    mChipTagStandNXP = false;
    mChipTagStandBeta = false;
    
    mCurrentChipID = "";
	
	// Set up the params GUI
	mParams = params::InterfaceGl( "Params", Vec2i( 400, 320 ) );
	mParams.addParam( "Frame rate",		&mFrameRate,									"", true							);
	mParams.addParam( "RFID-Reader-Status",		&reader.mReaderStatusString,							"", true							);
	mParams.addSeparator();
	mParams.addParam( "Serial Connection",		reader.getSerialDeviceList(), &mMeshIndex			);
	mParams.addSeparator();
    mParams.addParam( "Remove old Chips", &reader.bRemoveOldChips,									""							);
    mParams.addParam( "Remove when older than: ", &reader.mRemovalTimeSinceLastRefresh,         "min=-10f max=180f step=1"    );
    
    //mParams.addParam( "CAM ANGLE Y",		&flightControl.path.mCameraAngleToCarY,         "min=-180f max=180f step=.01"   );
    
    //	mParams.addButton( "RFID continious read", std::bind( &RFID_SimpleReader::toggleContiniousRead, &reader ) );
    //	mParams.addParam( " ",		&reader.isContiniousReading,							"", true							);
	mParams.addSeparator();
	mParams.addButton( "Reset Reader",			bind( &BetaLayoutRFIDApp::reset, this ),                        "key=x" );
    
	mParams.addButton( "Quit",			bind( &BetaLayoutRFIDApp::quit, this ),			"key=q"							);
	mParams.addSeparator();
    
    string str;
    str = " label='Write data: '";
	mParams.addParam( "WriteData", &mChipTagName, str );
    str = " label='Vistited NXP: '";
    mParams.addParam( "VisitNXP", &mChipTagStandNXP, str );
    str = " label='Vistited Murata: '";
    mParams.addParam( "VisitMurata", &mChipTagStandMurata , str );
    str = " label='Vistited Beta: '";
    mParams.addParam( "VisitBeta", &mChipTagStandBeta , str );
    
    mTagDescriptionMap["NOT VISITED ANYTHING"] = '-';
    mTagDescriptionMap["MAGIC_PCB"] = 'M';
    mTagDescriptionMap["PROTO_3D"] = 'D';
    mTagDescriptionMap["FRONT_PANEL"] = 'F';
    mTagDescriptionMapIDReverse['-'] = 0;
    mTagDescriptionMapIDReverse['M'] = 1;
    mTagDescriptionMapIDReverse['D'] = 2;
    mTagDescriptionMapIDReverse['F'] = 3;
    
    mChipTagComingFromList.push_back("NOT VISITED ANYTHING");
    mChipTagComingFromList.push_back("MAGIC_PCB");
    mChipTagComingFromList.push_back("PROTO_3D");
    mChipTagComingFromList.push_back("FRONT_PANEL");
    
    //mParams.addParam( "TAG-List ",		mRFIDMesse.mTags,   &mTagIndexCurrent);
    mParams.addParam( "ComingFrom", mChipTagComingFromList, &mChipTagComingFromID, str );
    
    mParams.addButton( "Write now", std::bind( &BetaLayoutRFIDApp::writeTagData, this ) );
    
    
    
    //    mParams.hide();
	
	mFont = Font( "Arial", 12 );
	mSize = Vec2f( 800, 600 );
	bTextureComplete = false;
    bRefreshList = true;
    bHasRFIDTags = false;
}

void BetaLayoutRFIDApp::mouseDown( MouseEvent event )
{
}

void BetaLayoutRFIDApp::update()
{
	// Update FPS
	mFrameRate = getAverageFps();
	reader.update();
	
	if(mTagIndexCurrent != mTagIndex){
		mTagIndex = mTagIndexCurrent;
		RFIDListChange();
	}
    
    //    string txt = mRFIDMesse.getAllTagsInfoString();
    //    string txt = reader.getAllTagsInfoString();
    if(bRefreshList){
        string txt = "SIMPLE READER : \n";
        txt += reader.getAllTagsInfoString();
        txt += "MESSE READER : \n";
        txt += mRFIDMesse.getAllTagsInfoString();
        
        TextBox tbox = TextBox().alignment( TextBox::LEFT ).font( mFont ).size( mSize ).text( txt );
        tbox.setColor( Color( 1.0f, 0.65f, 0.35f ) );
        tbox.setBackgroundColor( ColorA( 0.5, 0, 0, 1 ) );
        Vec2i sz = tbox.measure();
        mTextTexture = gl::Texture( tbox.render() );
        bTextureComplete = true;
        bRefreshList = false;
        
    }
	
}

void BetaLayoutRFIDApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	if(bTextureComplete){
		glColor3f( 1.0f, 1.0f, 1.0f );
		gl::draw( mTextTexture, Vec2f( 10, 10 ) );
	}
	// Draw params GUI
	mParams.draw();
}


void BetaLayoutRFIDApp::keyDown( KeyEvent event )
{
	if(event.getCode() == KeyEvent::KEY_ESCAPE) quit();
    if(event.getCode() == KeyEvent::KEY_SPACE){
        reader.writeTagData( reader.getTag( reader.mTagsID[0] ) , "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// TAGGY STUFF
////////////

void BetaLayoutRFIDApp::reset(){
    reader.reset();
    mRFIDMesse.reset();
    refreshParameterList();
}


void BetaLayoutRFIDApp::RFIDListChange()
{
    
    refreshParameterList();
	bRefreshList = true;
    
}

void BetaLayoutRFIDApp::refreshParameterList(){
    
    bHasRFIDTags = false;
	if(mRFIDMesse.mTags.size()==0) return;
    bHasRFIDTags = true;
    
    if(mRFIDMesse.mTags.size()-1 < mTagIndex) mTagIndex = 0;
    
    if(mRFIDMesse.mTags.size() != mTagListLength){
        mTagListLength = mRFIDMesse.mTags.size();
        mParams.removeParam( "TAG-List ");
        mParams.addParam( "TAG-List ",		mRFIDMesse.mTags,   &mTagIndexCurrent);
    }
    
    messeRFID* tag = mRFIDMesse.getTag(mRFIDMesse.mTags[mTagIndex]);
    if( tag->rfid_id.compare(mCurrentChipID) == 0 ) return;
    mCurrentChipID = tag->rfid_id;
    
    console() << "tag : " << tag->rfid_id << std::endl;
    mChipTagStandNXP = tag->mVisitedNXP;
    mChipTagStandMurata = tag->mVisitedMurata;
    mChipTagStandBeta = tag->mVisitedBeta;
    mChipTagName = tag->mUserName;
    
    if(mTagDescriptionMapIDReverse.count(tag->mVisitedTerminal) == 0){
        mChipTagComingFromID = 0;
    }else{
        mChipTagComingFromID = mTagDescriptionMapIDReverse[tag->mVisitedTerminal];
    }
    
    
    //    mTagDescriptionMap["NOT VISITED ANYTHING"] = '-';
    //    mTagDescriptionMap["MAGIC_PCB"] = 'M';
    //    mTagDescriptionMap["PROTO_3D"] = 'D';
    //    mTagDescriptionMap["FRONT_PANEL"] = 'F';
    //
    //    mChipTagComingFromList.push_back("NOT VISITED ANYTHING");
    //    mChipTagComingFromList.push_back("MAGIC_PCB");
    //    mChipTagComingFromList.push_back("PROTO_3D");
    //    mChipTagComingFromList.push_back("FRONT_PANEL");
    
    
}


void BetaLayoutRFIDApp::writeTagData(){
    if(mRFIDMesse.mTags.size()-1 < mTagIndex){
        console() << "Chip doesn't exist anymore!" << std::endl;
    }
    messeRFID* tag = mRFIDMesse.getTag(mRFIDMesse.mTags[mTagIndex]);
    tag->mUserName = mChipTagName;
    tag->mVisitedNXP = mChipTagStandNXP;
    tag->mVisitedMurata = mChipTagStandMurata;
    tag->mVisitedBeta = mChipTagStandBeta;
    tag->mVisitedTerminal = mTagDescriptionMap[mChipTagComingFromList[mChipTagComingFromID]];
    
    mRFIDMesse.writeTag(tag);
    refreshParameterList();
}


CINDER_APP_NATIVE( BetaLayoutRFIDApp, RendererGl )
