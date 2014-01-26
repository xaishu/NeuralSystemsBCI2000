////////////////////////////////////////////////////////////////////////////////
// $Id: B2B.cpp 4008 2012-05-15 12:42:40Z dsarma $
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "B2B.h"
#include "Localization.h"
#include "DFBuildScene2D.h"

#include "buffers.h"
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <algorithm>
#include <csignal>
#include <cerrno>

#include <QImage>

// State variable for initialization and other usage(s)
#define CURSOR_POS_BITS "12"

// Initial values of several state variables
#define SCORE_BITS "16"
#define TARGET_XWIDTH_BITS "12"
#define TARGET_YWIDTH_BITS "12"
#define CURSOR_RADIUS_BITS "12"
#define PRIMARY_AXIS_BITS "12"

// Used for reading from socket(s)
#define DEFAULT_LINE_BUFFER 50

RegisterFilter( DynamicFeedbackTask, 3 );

using namespace std;

DynamicFeedbackTask::DynamicFeedbackTask()
: mpFeedbackScene( NULL ),
  mpMessage( NULL ),
  mpMessage2( NULL ),
  mCursorColor( RGBColor::White ),
  mRunCount( 0 ),
  mTrialCount( 0 ),
  mCurFeedbackDuration( 0 ),
  mMaxFeedbackDuration( 0 ),
  mCursorSpeedX( 1.0 ),
  mCursorSpeedY( 1.0 ),
  mCursorSpeedZ( 1.0 ),
  mScore(0.0),
  mScoreCount(0.0),
  mrWindow( Window() ),
  mVisualFeedback( false),
  mIsVisualCatchTrial( false )
{
  BEGIN_PARAMETER_DEFINITIONS
	  "Application:Targets matrix Targets= "
      " 2 " // rows
      " [pos%20x pos%20y width%20x width%20y] " // columns
      " 0  0 100 10 "
      " 0 90 100 10 "
      " // Number of targets and their positions/dimensions in percentage coordinates",
    "Application:Targets int TargetColor= 0x0000FF % % % " // Blue
       " // target color (color)",
	
    "Application:Cursor float CursorWidth= 10 10 0.0 % "
      " // feedback cursor width in percent of screen width",
	"Application:Cursor float CursorSpeedMult= 3"
       " // Cursor speed from to 1(slowest) to 10(fastest) ",
	"Application:Cursor float CursorGravity= 1"
       " // Centering cursor gravity from to 0 (weakest) to 10 (strongest) ",
    "Application:Cursor int CursorColor= 0xff0000 % % % " // Red
       " // Cursor color (color)",
    "Application:Cursor floatlist CursorPos= 3 50 50 50 % % "
      " // cursor starting position",

    "Application:Sequencing float MaxFeedbackDuration= 3s % 0 % "
      " // abort a trial after this amount of feedback time has expired",

    "Application:Feedback int VisualFeedback= 1 1 0 1 "
      "// provide visual stimulus (boolean)",
    "Application:Feedback intlist VisualCatchTrials= 4 1 3 4 2 % % % // "
	  "// list of visual catch trials, leave empty for none",

	"Application:Connector string ConnectorAddress= % "
    "localhost:20320 % % // IP address/port to read from, e.g. localhost:20320",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "CursorPosX " CURSOR_POS_BITS " 0 0 0",
    "CursorPosY " CURSOR_POS_BITS " 0 0 0",
	"GameScore " SCORE_BITS " 0 0 0", //score bits
	"TargetXWidth " TARGET_XWIDTH_BITS " 0 0 0", //target width in x
	"TargetYWidth " TARGET_YWIDTH_BITS " 0 0 0", //target width in y
	"CursorRadius " CURSOR_RADIUS_BITS " 0 0 0", //cursor radius
  END_STATE_DEFINITIONS

  // Title screen message
  GUI::Rect rect = { 0.5f, 0.4f, 0.5f, 0.6f };
  mpMessage = new TextField( mrWindow );
  mpMessage->SetTextColor( RGBColor::Lime ) //Lime
            .SetTextHeight( 0.8f )
            .SetColor( RGBColor::Gray ) //Grey
            .SetAspectRatioMode( GUI::AspectRatioModes::AdjustWidth )
            .SetObjectRect( rect );

  // Score message in top right corner
  GUI::Rect rect3 = { 0.89f, 0.01f, .99f, 0.09f };
  mpMessage2 = new TextField( mrWindow );
  mpMessage2->SetTextColor( RGBColor::Black )
            .SetTextHeight( 0.45f )
            .SetColor( RGBColor::White )
			.SetAspectRatioMode( GUI::AspectRatioModes::AdjustNone)
            .SetObjectRect( rect3 );
}

DynamicFeedbackTask::~DynamicFeedbackTask() {
  delete mpFeedbackScene;
}

void
DynamicFeedbackTask::OnPreflight( const SignalProperties& Input ) const {
  if( Parameter( "CursorPos" )->NumValues() != 3 ) {
      bcierr << "Parameter \"CursorPos\" must have 3 entries" << endl;
  }

  const char* colorParams[] = {
    "CursorColor",
    "TargetColor",
  };
  for( size_t i = 0; i < sizeof( colorParams ) / sizeof( *colorParams ); ++i ) {
    if( RGBColor( Parameter( colorParams[ i ] ) ) == RGBColor( RGBColor::NullColor ) ) {
      bcierr << "Invalid RGB value in " << colorParams[ i ] << endl;
    }
  }

  if( Parameter( "FeedbackDuration" ).InSampleBlocks() <= 0 )
    bcierr << "FeedbackDuration must be greater 0" << endl;

  if( (Parameter( "CursorSpeedMult" ) > 10) || (Parameter( "CursorSpeedMult" ) < 1) )
    bcierr << "Cursor speed must be between 1 and 10" << endl;

  if( (Parameter( "CursorGravity" ) > 10) ||  (Parameter( "CursorGravity" ) < 0))
    bcierr << "Cursor gravity must be between 1 and 10" << endl;

  Parameter( "SampleBlockSize" );

  if( Parameter( "VisualFeedback" ) == 1) {
	  ParamRef visualCatch = Parameter( "VisualCatchTrials" );
	  for( int i = 0; i < visualCatch->NumValues(); ++i )
		if( visualCatch( i ) < 1 )
		  bcierr << "Invalid stimulus code "
				 << "(" << visualCatch( i ) << ") "
				 << "at visualCatch(" << i << ")"
				 << endl;
  }
  
  string connectorAddress = string( Parameter( "ConnectorAddress" ) );
}

void
DynamicFeedbackTask::OnInitialize( const SignalProperties& Input ) {
  mConnectorAddress = string( Parameter( "ConnectorAddress" ) );

  // Cursor speed in pixels per signal block duration:
  float feedbackDuration = Parameter( "FeedbackDuration" ).InSampleBlocks();
  
  // On average, we need to cross half the workspace during a trial.
  mCursorSpeedX = 100.0 / feedbackDuration / 2;
  mCursorSpeedY = 100.0 / feedbackDuration / 2;
  mCursorSpeedZ = 100.0 / feedbackDuration / 2;
  
  mMaxFeedbackDuration = static_cast<int>( Parameter( "MaxFeedbackDuration" ).InSampleBlocks() );
  
  mCursorColor = RGBColor( Parameter( "CursorColor" ) );

  delete mpFeedbackScene;
  mpFeedbackScene = new DFBuildScene2D( mrWindow );
  mpFeedbackScene->Initialize();
  mpFeedbackScene->SetCursorColor( mCursorColor );

  mrWindow.Show();
  DisplayMessage( "Timeout" );
  DisplayScore("0");

  mVisualFeedback = ( Parameter( "VisualFeedback" ) == 1);

  if( mVisualFeedback == true) {
	  mVisualCatchTrials.clear();
      for( int j = 0; j < Parameter( "VisualCatchTrials" )->NumValues(); ++j )
	      mVisualCatchTrials.push_back( Parameter( "VisualCatchTrials" )( j ) );
  }
}

void
DynamicFeedbackTask::OnStartRun() {
  ++mRunCount;
  mTrialCount = 0;
  mTrialStatistics.Reset();
  mScore = 0;
  State("GameScore") = mScore;
  State("CursorRadius") = Parameter( "CursorWidth" );

  AppLog << "Run #" << mRunCount << " started" << endl;
  DisplayMessage( ">> Get Ready! <<" );
}

void
DynamicFeedbackTask::DoPreRun( const GenericSignal&, bool& doProgress ) {
  doProgress = false;

  // If the socket has not been established, try to connect
  if ( !mSocket.connected() ) {
    // Interrupt the open() call after 1 second
    // This is to give the operator time to start up the Countdown game
    signal( SIGALRM, SIG_DFL );
    alarm( 1 );
    mSocket.open( mConnectorAddress );
    alarm( 0 );
    
    // Did the socket connect or get interrupted?
    if ( errno == EINTR ) {
      bciout << "Waiting for Countdown game..." << endl;
      return;
    }
  }
  
  // Wait for the start signal
  if (mSocket.can_read()) {
	char buffer[DEFAULT_LINE_BUFFER];
    mSocket.read(buffer, DEFAULT_LINE_BUFFER);
	
	std::string line(buffer);
    if (line.compare("t_start") != 0) {
      bciout << "DoPreRun: Unexpected input = \"" << line << "\"" << endl;
      return;
    }
    doProgress = true;
  }
}

void
DynamicFeedbackTask::OnTrialBegin() {
  ++mTrialCount;
  AppLog.Screen << "Trial #" << mTrialCount
                << ", target: " << State( "TargetCode" )
                << endl;

  if (mVisualFeedback == true) {
	  mIsVisualCatchTrial = false;
	  for (size_t i = 0; i < mVisualCatchTrials.size(); i++)
		  mIsVisualCatchTrial = (mVisualCatchTrials.at(i) == mTrialCount);

	  if (mIsVisualCatchTrial == true) {
		  AppLog.Screen << "<- visual catch trial" << endl;
	  }
  }


  enum { x, y, dx, dy };
  ParamRef Targets = Parameter( "Targets" );
  State("TargetXWidth") = Targets( State("TargetCode") - 1, dx );
  State("TargetYWidth") = Targets( State("TargetCode") - 1, dy );

  DisplayMessage( "" );
  RGBColor targetColor = RGBColor( Parameter( "TargetColor" ) );
  for( int i = 0; i < mpFeedbackScene->NumTargets(); ++i )
  {
    mpFeedbackScene->SetTargetColor( targetColor, i );
    mpFeedbackScene->SetTargetVisible( State( "TargetCode" ) == i + 1, i );
  }
}

void
DynamicFeedbackTask::OnFeedbackBegin() {
  mCurFeedbackDuration = 0;

  enum { x, y, z };
  ParamRef CursorPos = Parameter( "CursorPos" );
  MoveCursorTo( CursorPos( x ), CursorPos( y ), CursorPos( z ) );
  if (mVisualFeedback == true && mIsVisualCatchTrial == false)
    mpFeedbackScene->SetCursorVisible( true );
}

void
DynamicFeedbackTask::DoFeedback( const GenericSignal& ControlSignal, bool& doProgress ) {
	doProgress = false;
  
  // Update cursor position
  float x = mpFeedbackScene->CursorXPosition(),
        y = mpFeedbackScene->CursorYPosition(),
        z = mpFeedbackScene->CursorZPosition();
  
	// Restrict cursor movement to the inside of the bounding box:
	float r = mpFeedbackScene->CursorRadius();
	x = max( r, min( 100 - r, x ) ),
	y = max( r, min( 100 - r, y ) ),
	z = max( r, min( 100 - r, z ) );
	mpFeedbackScene->SetCursorPosition( x, y, z );

	const float coordToState = ( ( 1 << CURSOR_POS_BITS ) - 1 ) / 100.0;
	State( "CursorPosX" ) = static_cast<int>( x * coordToState );
	State( "CursorPosY" ) = static_cast<int>( y * coordToState );

  if( mpFeedbackScene->TargetHit( State( "TargetCode" ) - 1 ) ) {
    State( "ResultCode" ) = State( "TargetCode" );
    mpFeedbackScene->SetCursorColor( RGBColor::White );
    mpFeedbackScene->SetTargetColor( RGBColor::Red, State( "ResultCode" ) - 1 );

    // Send message of hit out to B2B Game
    mSocket.write("1", 1);
    doProgress = true;
  }

  if (mSocket.can_read()) {
	char buffer[DEFAULT_LINE_BUFFER];
    mSocket.read(buffer, DEFAULT_LINE_BUFFER);
	
	std::string line(buffer);
    if (line.compare("t_stop") != 0) {
      bciout << "DoFeedback: Unexpected input = \"" << line << "\"" << endl;
      return;
    }
    doProgress = true;
  }
}

void
DynamicFeedbackTask::OnFeedbackEnd() {
  if( State( "ResultCode" ) == 0 ) {
    AppLog.Screen << "-> aborted" << endl;
    mTrialStatistics.UpdateInvalid();

  } else {
    mTrialStatistics.Update( State( "TargetCode" ), State( "ResultCode" ) );
    if( State( "TargetCode" ) == State( "ResultCode" ) ) {
      AppLog.Screen << "-> hit\n " << "Your Score:" << mScore << endl;
	    State("GameScore") = mScore;
    } else {
      mScore = mScore;
	    AppLog.Screen << "-> miss\n " << "Your Score:" << mScore << endl;
	    State("GameScore") = mScore;
    }
  }

  //Persistent Score Display
  stringstream ss (stringstream::in | stringstream::out);
  int intScore = mScore >= 0 ? (int)(mScore + 0.5) : (int)(mScore - 0.5);
  ss << intScore;
  DisplayScore(ss.str());
}

void
DynamicFeedbackTask::DoITI( const GenericSignal&, bool& doProgress ) {
  // Wait for the start signal
  doProgress = false;
  if (mSocket.can_read()) {
	char buffer[DEFAULT_LINE_BUFFER];
    mSocket.read(buffer, DEFAULT_LINE_BUFFER);
	
	std::string line(buffer);
    if (line.compare("t_stop") == 0) {
      bciout << "DoITI: Already waiting for t_start" << endl;
    }

    if (line.compare("t_start") != 0) {
      bciout << "DoITI: Unexpected input = \"" << line << "\"" << endl;
      return;
    }
    doProgress = true;
  }
}

void
DynamicFeedbackTask::OnStopRun() {
  AppLog   << "Run " << mRunCount << " finished: "
           << mTrialStatistics.Total() << " trials, "
           << mTrialStatistics.Hits() << " hits, "
           << mTrialStatistics.Invalid() << " invalid.\n";
  int validTrials = mTrialStatistics.Total() - mTrialStatistics.Invalid();
  if( validTrials > 0 )
    AppLog << ( 200 * mTrialStatistics.Hits() + 1 ) / validTrials / 2  << "% correct, "
           << mTrialStatistics.Bits() << " bits transferred.\n, "
		   << "Game Score:\n " << mScore ;
  AppLog   << "====================="  << endl;

  DisplayMessage( "Timeout" );
  mSocket.close();
}


// Access to graphic objects
void
DynamicFeedbackTask::MoveCursorTo( float inX, float inY, float inZ )
{
  mpFeedbackScene->SetCursorPosition( inX, inY, inZ );
}

void
DynamicFeedbackTask::DisplayMessage( const string& inMessage )
{
  if( inMessage.empty() )
  {
    mpMessage->Hide();
  }
  else
  {
    mpMessage->SetText( " " + inMessage + " " );
    mpMessage->Show();
  }
}
void
DynamicFeedbackTask::DisplayScore( const string& inMessage )
{
  if( inMessage.empty() )
  {
    mpMessage2->Hide();
  }
  else
  {
    mpMessage2->SetText( "+" + inMessage + " " );
    mpMessage2->Show();
  }
}