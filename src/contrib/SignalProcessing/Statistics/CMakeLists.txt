###########################################################################
## $Id$
## Authors:
## Description: Build information for the StatisticsSignalProcessing project
##              
##              
###########################################################################

# Set the executable name
BCI2000_INCLUDE( "STATISTICS" )
BCI2000_USE( "FFT" )

SET( DIR_STAT
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/statistics
)

SET( SRC_STAT
  ${DIR_STAT}/ChannelSetSource.cpp
  ${DIR_STAT}/DataSource.cpp
  ${DIR_STAT}/ExpressionSource.cpp
  ${DIR_STAT}/FunctionSource.cpp
  ${DIR_STAT}/ObserverSource.cpp
  ${DIR_STAT}/ViewSource.cpp
  ${DIR_STAT}/StatisticsFilter.cpp
)

ADD_SUBDIRECTORY( Cmdline )

SET( SRC_PROJECT
  PipeDefinition.cpp
  ${SRC_STAT}
)

SET( NAME StatisticsSignalProcessing1 )
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( ${NAME} ${SRC_PROJECT} )
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_DEFINITIONS STATISTICS_FILTER_POS=1 )

SET( NAME StatisticsSignalProcessing2 )
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( ${NAME} ${SRC_PROJECT} ) 
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_DEFINITIONS STATISTICS_FILTER_POS=2 )

IF( 0 )
SET( NAME StatisticsSignalProcessing3 )
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( ${NAME} ${SRC_PROJECT} ) 
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_DEFINITIONS STATISTICS_FILTER_POS=3 )
ENDIF( 0 )

SET( NAME StatisticsSignalProcessing4 )
BCI2000_ADD_SIGNAL_PROCESSING_MODULE( ${NAME} ${SRC_PROJECT} ) 
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_DEFINITIONS STATISTICS_FILTER_POS=4 )
