////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This BCI2000 filter calls the Matlab engine to act upon signals,
//    parameters, and states, thus providing the full BCI2000 filter interface
//    to a Matlab filter implementation.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MatlabFilter.h"
#include "defines.h"

using namespace std;

RegisterFilter( MatlabFilter, 2.C );

#define MATLAB_NAME( x )   "bci_"#x  // How variables and functions are named in Matlab.

// Matlab function names
#define CONSTRUCT   MATLAB_NAME( Construct )
#define DESTRUCT    MATLAB_NAME( Destruct )
#define PREFLIGHT   MATLAB_NAME( Preflight )
#define INITIALIZE  MATLAB_NAME( Initialize )
#define PROCESS     MATLAB_NAME( Process )
#define START_RUN   MATLAB_NAME( StartRun )
#define STOP_RUN    MATLAB_NAME( StopRun )
#define RESTING     MATLAB_NAME( Resting )
#define HALT        MATLAB_NAME( Halt )

// Matlab variable names
#define PARAM_DEFS      MATLAB_NAME( ParamDefs )
#define STATE_DEFS      MATLAB_NAME( StateDefs )
#define IN_SIGNAL       MATLAB_NAME( InSignal )
#define IN_SIGNAL_DIMS  MATLAB_NAME( InSignalDims )
#define OUT_SIGNAL      MATLAB_NAME( OutSignal )
#define OUT_SIGNAL_DIMS MATLAB_NAME( OutSignalDims )
#define PARAMETERS      MATLAB_NAME( Parameters )
#define STATES          MATLAB_NAME( States )

MatlabFilter::MatlabFilter()
: mBci_Process( PROCESS )
{
  MatlabFunction bci_Construct( CONSTRUCT );
  bci_Construct.OutputArgument( PARAM_DEFS )
               .OutputArgument( STATE_DEFS );
  if( CallMatlab( bci_Construct ) )
  {
    // Add the parameters and states requested by the Matlab bci_Construct function.
    int numParamDefs = MatlabEngine::GetScalar( "max(size(" PARAM_DEFS "))" );
    for( int i = 1; i <= numParamDefs; ++i )
    {
      ostringstream expr;
      expr << PARAM_DEFS << "{" << i << "}";
      string paramDef = MatlabEngine::GetString( expr.str() );
      if( !Parameters->Add( paramDef ) )
        bcierr << "Error in parameter definition: " << paramDef << endl;
    }
    int numStateDefs = MatlabEngine::GetScalar( "max(size(" STATE_DEFS "))" );
    for( int i = 1; i <= numStateDefs; ++i )
    {
      ostringstream expr;
      expr << STATE_DEFS << "{" << i << "}";
      string stateDef = MatlabEngine::GetString( expr.str() );
      if( !States->Add( stateDef ) )
        bcierr << "Error in state definition: " << stateDef << endl;
    }
  }
  MatlabEngine::ClearVariable( PARAM_DEFS );
  MatlabEngine::ClearVariable( STATE_DEFS );

  // Issue a warning to indicate potential mis-configuration if there is no
  // Preflight, Initialize, or Process function available for Matlab.
  const char* essentialFunctions[] =
  {
    PREFLIGHT,
    INITIALIZE,
    PROCESS,
  };
  ostringstream oss;
  for( size_t i = 0; i < sizeof( essentialFunctions ) / sizeof( *essentialFunctions ); ++i )
    if( !MatlabFunction( essentialFunctions[ i ] ).Exists() )
      oss << ", " << essentialFunctions[ i ];
  if( !oss.str().empty() )
    bciout << "The following functions could not be found in the Matlab path:\n"
           << oss.str().substr( 2 ) << ".\n"
           << "Make sure that the m-files exist within the path, and contain "
           << "appropriate function definitions."
           << endl;

  // Initialize the bci_Process function for more efficient calling during Process().
  mBci_Process.InputArgument( IN_SIGNAL )
              .OutputArgument( OUT_SIGNAL );
}

MatlabFilter::~MatlabFilter()
{
  MatlabFunction bci_Destruct( DESTRUCT );
  CallMatlab( bci_Destruct );

  MatlabEngine::ClearVariable( IN_SIGNAL );
  MatlabEngine::ClearVariable( OUT_SIGNAL );
  MatlabEngine::ClearVariable( PARAMETERS );
  MatlabEngine::ClearVariable( STATES );
}

void
MatlabFilter::Preflight( const SignalProperties& Input,
                               SignalProperties& Output ) const
{
  MatlabEngine::ClearVariable( PARAMETERS );
  MatlabEngine::CreateGlobal( PARAMETERS );
  ParamsToMatlabWS();
  MatlabEngine::ClearVariable( STATES );
  MatlabEngine::CreateGlobal( STATES );
  StatesToMatlabWS();

  Output = Input;
  MatlabEngine::PutMatrix( IN_SIGNAL_DIMS, Input );
  MatlabFunction bci_Preflight( PREFLIGHT );
  bci_Preflight.InputArgument( IN_SIGNAL_DIMS )
               .OutputArgument( OUT_SIGNAL_DIMS );
  if( CallMatlab( bci_Preflight ) )
    Output = MatlabEngine::GetMatrix( OUT_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( IN_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( OUT_SIGNAL_DIMS );
}

void
MatlabFilter::Initialize( const SignalProperties& Input,
                          const SignalProperties& Output )
{
  MatlabEngine::PutMatrix( IN_SIGNAL_DIMS, Input );
  MatlabEngine::PutMatrix( OUT_SIGNAL_DIMS, Output );
  MatlabFunction bci_Initialize( INITIALIZE );
  bci_Initialize.InputArgument( IN_SIGNAL_DIMS )
                .InputArgument( OUT_SIGNAL_DIMS );
  CallMatlab( bci_Initialize );
  MatlabEngine::ClearVariable( IN_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( OUT_SIGNAL_DIMS );
  MatlabEngine::PutMatrix( IN_SIGNAL, GenericSignal( Input ) );
  MatlabEngine::PutMatrix( OUT_SIGNAL, GenericSignal( Output ) );
}

void
MatlabFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  StatesToMatlabWS();
  MatlabEngine::PutMatrix( IN_SIGNAL, Input );
  if( CallMatlab( mBci_Process ) )
    Output = MatlabEngine::GetMatrix( OUT_SIGNAL );
  else
    Output = Input;
  MatlabWSToStates();
}

void
MatlabFilter::StartRun()
{
  MatlabFunction bci_StartRun( START_RUN );
  if( bci_StartRun.Exists() )
  {
    ParamsToMatlabWS();
    CallMatlab( bci_StartRun );
  }
}

void
MatlabFilter::StopRun()
{
  MatlabFunction bci_StopRun( STOP_RUN );
  if( CallMatlab( bci_StopRun ) )
    MatlabWSToParams();
}

void
MatlabFilter::Resting()
{
  MatlabFunction bci_Resting( RESTING );
  if( CallMatlab( bci_Resting ) )
    MatlabWSToParams();
}

void
MatlabFilter::Halt()
{
  MatlabFunction bci_Halt( HALT );
  CallMatlab( bci_Halt );
}

void
MatlabFilter::StatesToMatlabWS() const
{
  for( int i = 0; i < States->Size(); ++i )
  {
    const string& name = ( *States )[ i ].Name();
    MatlabEngine::PutScalar( string( STATES "." ) + name, State( name ) );
  }
}

void
MatlabFilter::MatlabWSToStates()
{
  for( int i = 0; i < States->Size(); ++i )
  {
    const string& name = ( *States )[ i ].Name();
    State( name ) = MatlabEngine::GetScalar( string( STATES "." ) + name );
  }
}

void
MatlabFilter::ParamsToMatlabWS() const
{
  for( int i = 0; i < Parameters->Size(); ++i )
  {
    Param& p = ( *Parameters )[ i ];
    MatlabEngine::PutCells( string( PARAMETERS "." ) + p.Name(), p );
  }
}

void
MatlabFilter::MatlabWSToParams()
{
  for( int i = 0; i < Parameters->Size(); ++i )
  {
    Param& p = ( *Parameters )[ i ];
    MatlabEngine::StringMatrix values = MatlabEngine::GetCells( string( PARAMETERS "." ) + p.Name() );
    int newRows = values.size(),
        newCols = newRows ? values.at( 0 ).size() : 1;
    if( newRows != p.NumRows() || newCols != p.NumColumns() )
      p.SetDimensions( newRows, newCols );
    for( int row = 0; row < newRows; ++row )
      for( int col = 0; col < newCols; ++col )
        if( string( p.Value( row, col ) ) != values.at( row ).at( col ) )
          p.Value( row, col ) = values.at( row ).at( col );
  }
}

bool
MatlabFilter::CallMatlab( MatlabFunction& inFunction ) const
{
  if( !inFunction.Exists() )
    return false;
  string err = inFunction.Execute();
  if( !err.empty() )
  {
    bcierr__ << "Matlab function \"" << inFunction.Name() << "\": "
             << err
             << endl;
    return false;
  }
  return true;
}
