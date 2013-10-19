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
    void keyUp( KeyEvent event );
	void update();
	void draw();
    void reset();
	
	void RFIDListChange();
	
private:
    
	RFID_SimpleReader           reader;
	
	float						mFrameRate;
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
    bool                        bDoWriteChip;
    bool                        bKeyF1down;
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
    
    reader.sRFIDListChanged.connect( boost::bind(&BetaLayoutRFIDApp::RFIDListChange, this) );
    
	mFont = Font( "Arial", 12 );
	mSize = Vec2f( 800, 600 );
	bTextureComplete = false;
    bRefreshList = true;
    bHasRFIDTags = false;
    bDoWriteChip = false;
    bKeyF1down = false;
}

void BetaLayoutRFIDApp::mouseDown( MouseEvent event )
{
}

void BetaLayoutRFIDApp::update()
{
	// Update FPS
	mFrameRate = getAverageFps();
	reader.update();
    	
//	if(mTagIndexCurrent != mTagIndex){
//		mTagIndex = mTagIndexCurrent;
//		RFIDListChange();
//	}
    
    //    string txt = mRFIDMesse.getAllTagsInfoString();
    //    string txt = reader.getAllTagsInfoString();
    if(bRefreshList){
        string txt = "SIMPLE READER : \n\n\n";
        if(bDoWriteChip) txt += "WRITING DATA\n\n\n";
        txt += reader.getAllTagsInfoString();
        
        TextBox tbox = TextBox().alignment( TextBox::LEFT ).font( mFont ).size( mSize ).text( txt );
        tbox.setColor( Color( 1.0f, 0.65f, 0.35f ) );
        tbox.setBackgroundColor( ColorA( 0.5, 0, 0, 1 ) );
        Vec2i sz = tbox.measure();
        mTextTexture = gl::Texture( tbox.render() );
        bTextureComplete = true;
        bRefreshList = false;
    }
	
    if(bDoWriteChip){
        bDoWriteChip = false;
        writeTagData();
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
}


void BetaLayoutRFIDApp::keyDown( KeyEvent event )
{
	if(event.getCode() == KeyEvent::KEY_ESCAPE) quit();
    if(event.getCode() == KeyEvent::KEY_SPACE){
//        reader.writeTagData( reader.getTag( reader.mTagsID[0] ) , "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
    if(event.getCode() == KeyEvent::KEY_F1){
        if(!bKeyF1down){
            bDoWriteChip = true;
        }
        bKeyF1down = true;
    }
}

void BetaLayoutRFIDApp::keyUp( KeyEvent event )
{
    if(event.getCode() == KeyEvent::KEY_F1){
        bDoWriteChip = false;
        bKeyF1down = false;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// TAGGY STUFF
////////////

void BetaLayoutRFIDApp::reset(){
    reader.reset();
    refreshParameterList();
}


void BetaLayoutRFIDApp::RFIDListChange()
{
    
    refreshParameterList();
	bRefreshList = true;
    
}

void BetaLayoutRFIDApp::refreshParameterList(){

//    console() << "=========================" << std::endl << "RFID LIST UPDATE:" << std::endl <<     reader.getAllTagsInfoString() << "=========================" << std::endl;
    
}


void BetaLayoutRFIDApp::writeTagData(){
    
    vector<string> ids = reader.getTagIDs();
    for(int i=0;i<ids.size();i++){
        RFID_Tag* tag = reader.getTag(ids[i]);
        reader.writeTagData(tag, "HALLO DU CHIPS!" );
    }
}


CINDER_APP_NATIVE( BetaLayoutRFIDApp, RendererGl )
