###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Core/" )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( SignalProcessing )
ADD_SUBDIRECTORY( SignalSource )
ADD_SUBDIRECTORY( Application )
ADD_SUBDIRECTORY( Operator )
ADD_SUBDIRECTORY( Tools )
