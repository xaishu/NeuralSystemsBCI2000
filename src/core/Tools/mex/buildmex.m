%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% $Id$
% Author:      juergen.mellinger@uni-tuebingen.de
% Description: Matlab M-file to build BCI2000 Matlab mex files.
%
% (C) 2000-2008, BCI2000 Project
% http://www.bci2000.org
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function buildmex( varargin )
% Usage: buildmex <options> <target>
% For a list of possible options, see 'help mex'.
%
% If the buildmex function fails with a 'Could not detect a compiler 
% on local system' error, run 'mex -setup'.

TARGETS = { ...
    'load_bcidat', ...
    'convert_bciprm', ...
    'mem', ...
    };

BCIROOT = '../../../../';
BCIFRM = [ BCIROOT 'src/shared/' ];
CMDLINE = [ BCIROOT 'src/core/Tools/cmdline/' ];
BCIDATA = [ BCIROOT 'data/samplefiles/' ];
TESTFILE = 'eeg1_1';
                    
MEXSRC = { ...
    [ CMDLINE 'bci_stubs.cpp' ], ...
    [ BCIFRM 'fileio/dat/BCI2000FileReader.cpp' ], ...
    [ BCIFRM 'types/Param.cpp' ], ...
    [ BCIFRM 'types/ParamList.cpp' ], ...
    [ BCIFRM 'types/EncodedString.cpp' ], ...
    [ BCIFRM 'types/LabelIndex.cpp' ], ...
    [ BCIFRM 'types/HierarchicalLabel.cpp' ], ...
    [ BCIFRM 'types/Brackets.cpp' ], ...
    [ BCIFRM 'types/State.cpp' ], ...
    [ BCIFRM 'types/StateList.cpp' ], ...
    [ BCIFRM 'types/StateVector.cpp' ], ...
    [ BCIFRM 'types/GenericSignal.cpp'], ...
    [ BCIFRM 'types/SignalProperties.cpp'], ...
    [ BCIFRM 'types/SignalType.cpp'], ...
    [ BCIFRM 'types/PhysicalUnit.cpp'], ...
    [ BCIFRM 'bcistream/BCIError.cpp' ], ...
    [ BCIFRM 'bcistream/BCIError_mex.cpp' ], ...
    'bci_mexstubs.cpp', ...
    'mexutils.cpp', ...
    };
          
INCLUDEPATHS = { ...
  '-I../..', ...
  [ '-I' BCIROOT '/src/extlib/math' ], ...
  [ '-I' BCIFRM ], ...
  [ '-I' BCIFRM '/accessors' ], ...
  [ '-I' BCIFRM '/bcistream' ], ...
  [ '-I' BCIFRM '/config' ], ...
  [ '-I' BCIFRM '/fileio' ], ...
  [ '-I' BCIFRM '/fileio/dat' ], ...
  [ '-I' BCIFRM '/fileio/edf_gdf' ], ...
  [ '-I' BCIFRM '/modules' ], ...
  [ '-I' BCIFRM '/types' ], ...
  [ '-I' BCIFRM '/utils' ], ...
  [ '-I' BCIFRM '/utils/Expression' ], ...
  };

BINDIR = [ BCIROOT '/tools/mex' ];

DEFINES = { ...
  '-D_DEBUG', ...
  '-DBCI_TOOL', ...
  '-DBCI_MEX', ...
  '-DNO_STRICT', ...
  '-D_NO_VCL', ...
  '-DNO_PCHINCLUDES', ...
  };

switch( computer )
  case 'PCWIN'
    CXXFLAGS = {};
    LDFLAGS = {};
  otherwise % we assume gcc on all other platforms
    CXXFLAGS = { ...
      'CXXFLAGS=$CXXFLAGS -include gccprefix.h' ...
      };
    LDFLAGS = { ...
      'LDFLAGS=\$LDFLAGS -dead_strip' ...
      };
end;

options = {};
if( nargin < 1 )
  target = 'all';
else
  target = varargin{ nargin };
  options = { varargin{ 1 : end-1 } };
  if( target(1) == '-' )
    options = { options{:}, target };
    target = 'all';
  end
end

switch( target )

  case 'all'
    for( i = 1:length( TARGETS ) )
     buildmex( options{:}, TARGETS{i} );
    end
    buildmex test;
    
  case 'test'
    fprintf( 1, [ 'Testing mex files ... ' ] );
    success = true;
    try
      [ signal, states, parameters ] = load_bcidat( [ BCIDATA TESTFILE '.dat' ] );
      ref = load( [ TESTFILE '.mat' ] );
      if( signal ~= ref.signal )
        error( 'Testing load_bcidat: Signal data mismatch' );
      end
      if( ~equal_structs( states, ref.states ) )
        error( 'Testing load_bcidat: State vector data mismatch' );
      end
      if( ~equal_structs( parameters, ref.parameters ) )
        error( 'Testing load_bcidat: Parameter mismatch' );
      end
      if( ~equal_structs( parameters, convert_bciprm( convert_bciprm( parameters ) ) ) )
        error( 'Testing convert_bciprm: Mismatch when converting forth and back' );
      end
      spectrum_ = mem( double( signal ), [16, 0, 0.4, 0.02, 15] );
      if( spectrum_ ~= ref.spectrum_ )
        warning( 'Testing mem: Computed spectrum mismatch' );
      end
      clear signal states parameters spectrum ref;
    catch
      success = false;
    end
    if( success )
      fprintf( 1, 'OK.\n' );
    else
      err = lasterror;
      fprintf( 1, '%s.\n', err.message );
    end
    
  otherwise
    fprintf( 1, [ 'Building ' target ' ...\n' ] );
    mex( options{:}, CXXFLAGS{:}, LDFLAGS{:}, INCLUDEPATHS{:}, DEFINES{:}, [target '.cpp'], MEXSRC{:} );
    if( ~exist( BINDIR ) )
      mkdir( BINDIR );
    end
    copyfile( [ target '.' mexext ], BINDIR );
    if( exist( [ target '.m' ] ) )
      copyfile( [ target '.m' ], BINDIR );
    end
    
end

% A helper function to test structs for equality.
function result = equal_structs( inStruct1, inStruct2 )

  result = true;
  fnames = fieldnames( inStruct1 );
  if( ~strcmp( fnames, fieldnames( inStruct2 ) ) )
    result = false;
  else
    for( i = 1:length( fnames ) )
      if( isstruct( inStruct1.(fnames{i}) ) )
        if( ~equal_structs( inStruct1.(fnames{i}), inStruct2.(fnames{i}) ) )
          result = false;
        end
      elseif( ischar( inStruct1.(fnames{i}) ) || iscell( inStruct1.(fnames{i}) ) )
        if( ~strcmp( inStruct1.(fnames{i}), inStruct2.(fnames{i}) ) )
          result = false;
        end
      else
        if( inStruct1.(fnames{i}) ~= inStruct2.(fnames{i}) )
          result = false;
        end
      end
    end
  end
