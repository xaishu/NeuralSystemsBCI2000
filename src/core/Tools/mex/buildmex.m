function buildmex( varargin )
% Usage: buildmex <options> <target>
% For a list of options, see 'help mex'.
%
% Troubleshooting
%
% - The buildmex function fails with a 'Could not detect a compiler
%   on local system' error.
% + Run 'mex -setup' to configure mex file compilation.
%
% - Under Linux, compiling and linking works fine but I get an
%   "Invalid MEX-file" error during the test stage, with the error
%   message indicating a failure to locate a required version of
%   GLIBCXX.
% + Matlab, and all software started from within it, use their own
%   copies of the GLIBC and GLIBCXX libraries, matching the gcc version
%   that was used to compile Matlab. When building a mex file, however,
%   the installed gcc will dynamically link with its own versions of those
%   libraries, which are sometimes incompatible with Matlab's (note that
%   the list of officially supported gcc versions appears not to apply to
%   MEX files using the C++ library, so you can't rely on your gcc version
%   being listed there). Switching to the gcc version that your version of
%   Matlab was built with should fix the problem.
%
% The buildmex function is part of the BCI2000 project.
% $BEGIN_BCI2000_LICENSE$
% 
% This file is part of BCI2000, a platform for real-time bio-signal research.
% [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
% 
% BCI2000 is free software: you can redistribute it and/or modify it under the
% terms of the GNU General Public License as published by the Free Software
% Foundation, either version 3 of the License, or (at your option) any later
% version.
% 
% BCI2000 is distributed in the hope that it will be useful, but
%                         WITHOUT ANY WARRANTY
% - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
% A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License along with
% this program.  If not, see <http://www.gnu.org/licenses/>.
% 
% $END_BCI2000_LICENSE$

%  Matlab M-file to build BCI2000 Matlab mex files.
%  $Id$

TARGETS = { ...
    'load_bcidat', ...
    'save_bcidat', ...
    'convert_bciprm', ...
    'mem', ...
    };

BCIROOT = '../../../../';
BCIFRM = [ BCIROOT 'src/shared/' ];
TESTFILE = 'testdata';

MEXSRC = { ...
    [ BCIFRM 'fileio/dat/BCI2000FileReader.cpp' ], ...
    [ BCIFRM 'fileio/dat/BCI2000OutputFormat.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/EDFOutputBase.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/EDFOutputFormat.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/GDFOutputFormat.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/GDF.cpp' ], ...
    [ BCIFRM 'accessors/Environment.cpp' ], ...
    [ BCIFRM 'accessors/ParamRef.cpp' ], ...
    [ BCIFRM 'modules/MessageHandler.cpp' ], ...
    [ BCIFRM 'modules/MessageQueue.cpp' ], ...
    [ BCIFRM 'modules/signalprocessing/ARGroup.cpp' ], ...
    [ BCIFRM 'modules/signalprocessing/ARChannel.cpp' ], ...
    [ BCIFRM 'types/Param.cpp' ], ...
    [ BCIFRM 'types/ParamList.cpp' ], ...
    [ BCIFRM 'types/EncodedString.cpp' ], ...
    [ BCIFRM 'types/LabelIndex.cpp' ], ...
    [ BCIFRM 'types/HierarchicalLabel.cpp' ], ...
    [ BCIFRM 'types/Brackets.cpp' ], ...
    [ BCIFRM 'types/State.cpp' ], ...
    [ BCIFRM 'types/StateList.cpp' ], ...
    [ BCIFRM 'types/StateVector.cpp' ], ...
    [ BCIFRM 'types/StateVectorSample.cpp' ], ...
    [ BCIFRM 'types/GenericSignal.cpp' ], ...
    [ BCIFRM 'types/SignalProperties.cpp' ], ...
    [ BCIFRM 'types/SignalType.cpp' ], ...
    [ BCIFRM 'types/PhysicalUnit.cpp' ], ...
    [ BCIFRM 'types/SysCommand.cpp' ], ...
    [ BCIFRM 'types/Status.cpp' ], ...
    [ BCIFRM 'types/GenericVisualization.cpp' ], ...
    [ BCIFRM 'types/BitmapImage.cpp' ], ...
    [ BCIFRM 'utils/Expression/ArithmeticExpression.cpp' ], ...
    [ BCIFRM 'utils/Expression/Expression.cpp' ], ...
    [ BCIFRM 'utils/Expression/ExpressionParser.cpp' ], ...
    [ BCIFRM 'utils/ClassName.cpp' ], ...
    [ BCIFRM 'utils/OSMutex.cpp' ], ...
    [ BCIFRM 'utils/OSThread.cpp' ], ...
    [ BCIFRM 'utils/OSEvent.cpp' ], ...
    [ BCIFRM 'utils/MeasurementUnits.cpp' ], ...
    [ BCIFRM 'utils/VersionInfo.cpp' ], ...
    [ BCIFRM 'bcistream/BCIException.cpp' ], ...
    [ BCIFRM 'bcistream/BCIError.cpp' ], ...
    [ BCIFRM 'bcistream/BCIError_mex.cpp' ], ...
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
    [ '-I' BCIFRM '/modules/signalprocessing' ], ...
    [ '-I' BCIFRM '/types' ], ...
    [ '-I' BCIFRM '/utils' ], ...
    [ '-I' BCIFRM '/utils/Expression' ], ...
    };

LIBRARIES = { ...
    };
LIBPATHS = { ...
    };

BINDIR = [ BCIROOT '/tools/mex' ];

DEFINES = { ...
    '-DBCI_TOOL', ...
    '-DBCI_MEX', ...
    '-DNO_STRICT', ...
    '-DNO_PCHINCLUDES', ...
    '-D_USE_MATH_DEFINES', ...
    '-v',...
    };

switch( computer )
  case { 'PCWIN', 'PCWIN64' }
    build_version_header = 'cmd /c "cd ..\..\..\shared\config && "%ProgramFiles%\TortoiseSVN\bin\SubWCRev" ..\.. Version.h.in Version.h"';
  otherwise % we assume gcc on all other platforms
    build_version_header = '(cd ../../../buildutils && ./update_version_header.sh)';
    CXXFLAGS = '-fPIC -include gccprefix.h';
    LDFLAGS = '-dead_strip';
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

% Check for -use-qt command line option.
USE_QT = false;
options_ = options;
options = {};
for( i = 1:length( options_ ) )
    if( strcmpi( '-use-qt', options_{i} ) || strcmpi( '-use_qt', options_{i} ) )
        USE_QT = true;
    else
        options = { options{:} options_{i} };
    end
end

% Check for Qt availability
if( ~strcmp( target, 'all' ) && ~strcmp( target, 'test' ) )
    if( USE_QT )
        QT_FOUND = false;
        [ status, QT_VERSION ] = system( 'qmake -query "QT_VERSION"' );
        if( status == 0 )
            [ status, QT_INSTALL_HEADERS ] = system( 'qmake -query "QT_INSTALL_HEADERS"' );
        end
        if( status == 0 )
            [ status, QT_INSTALL_LIBS ] = system( 'qmake -query "QT_INSTALL_LIBS"' );
        end
        if( status == 0 )
            QT_FOUND = true;
            fprintf( 1, 'Linking against Qt version %s', QT_VERSION );
            fprintf( 1, 'Qt headers located in %s', QT_INSTALL_HEADERS );
            fprintf( 1, 'Qt libs located in %s', QT_INSTALL_LIBS );
            fprintf( 1, ['Warning: mexfiles are dynamically linked against Qt,\n' ...
                         'and may not run on machines without Qt installed.\n'] );
        else
            fprintf( 1, ['No Qt installation found.\n' ...
                         'Note that the Qt installation coming with BCI2000 cannot' ...
                         ' be used to build mex files.\n' ...
                         'No multithreading will be available for the MEM mex file.\n'] );
        end
        if( QT_FOUND )    
            INCLUDEPATHS = { INCLUDEPATHS{:} ...
                [ '-I' strtrim( QT_INSTALL_HEADERS ) ] ...
                [ '-I' strtrim( QT_INSTALL_HEADERS ) '/QtCore' ] ...
            };
            if( strncmp( computer, 'MAC', 3 ) )
                LDFLAGS = [ LDFLAGS ' -framework QtCore' ];
            else
                LIBRARIES = { LIBRARIES{:} '-lQtCore' '-lws2_32' };
                LIBPATHS = { LIBPATHS{:} [ '-L' QT_INSTALL_LIBS ] };
            end
            DEFINES = { DEFINES{:} [ '-DUSE_QT' ] };
        end
    else
        fprintf( 1, ['Mex file will be built without multithreading support.\n' ...
                     'To enable multithreading in mex files, specify -USE_QT ' ...
                     'as an option to the buildmex command.\n'] );
    end
end

switch( target )
    
    case 'all'
        for( i = 1:length( TARGETS ) )
            buildmex( options_{:}, TARGETS{i} );
        end
        buildmex test;
        
    case 'build'
        system( build_version_header );
        buildmex( options_{:}, 'all' );
        
    case 'test'
        fprintf( 1, [ 'Testing mex files ... ' ] );
        success = true;
        try
            [ signal, states, parameters ] = load_bcidat( [ TESTFILE '.dat' ] );
            ref = load( [ TESTFILE '.mat' ] );
            if( any( signal ~= ref.signal ) )
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
            spectrum_ = mem( double( signal ), [16, 0, 0.4, 0.02, 15, 0] );
            if( any( spectrum_ ~= ref.spectrum_ ) )
                rel_err = sqrt( norm( spectrum_ - ref.spectrum_, 'fro' ) / norm( spectrum_, 'fro' ) );
                fprintf( 1, 'Warning: mem: Mismatch between computed spectra (relative error is %d)\n', rel_err );
                err_limit = 1e-6;
                if( rel_err > err_limit )
                    error( 'Testing mem: Mismatch between computed spectra exceeds relative error limit of %f\n', err_limit );
                end
            end
            clear signal states parameters spectrum ref;
        catch err
            success = false;
        end
        if( success )
            fprintf( 1, 'Mex files tested OK.\n' );
        else
            fprintf( 1, 'Error: %s.\n', err.message );
        end
        
    otherwise
        fprintf( 1, [ 'Building ' target ' ...\n' ] );
        args = { options{:}, INCLUDEPATHS{:}, LIBRARIES{:}, LIBPATHS{:}, DEFINES{:} };
        if( exist( 'CXXFLAGS' ) )
          CXXFLAGS = [ 'CXXFLAGS=""\$CXXFLAGS ' CXXFLAGS '""' ];
          args = { args{:}, CXXFLAGS };
        end
        if( exist( 'LDFLAGS' ) )
          LDFLAGS =  [ 'LDFLAGS=""\$LDFLAGS ' LDFLAGS '""' ];
          args = { args{:}, LDFLAGS };
        end
        args = { args{:}, [target '.cpp'], MEXSRC{:} };
        mex( args{:} );
        if( ~exist( BINDIR ) )
            mkdir( BINDIR );
        end
        copyfile( [ target '.' mexext ], BINDIR );
        if( exist( [ target '.m' ] ) )
            copyfile( [ target '.m' ], BINDIR );
        end
        fprintf( 1, [ 'Finished building ' target '\n' ] );
end

% A helper function to test structs for equality.
function result = equal_structs( inStruct1, inStruct2 )

result = true;
fnames = fieldnames( inStruct1 );
if( length( fnames )~=length(fieldnames(inStruct2)) )
    result = false;
elseif( ~strcmp( fnames, fieldnames( inStruct2 ) ) )
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
        elseif( isnumeric( inStruct1.(fnames{i}) ) )
            result = isequalwithequalnans( inStruct1.(fnames{i}), inStruct2.(fnames{i}) );
        else
            if( any( inStruct1.(fnames{i}) ~= inStruct2.(fnames{i}) ) )
                result = false;
            end
        end
    end
end
