/******************************************************************************
 * $Id$
 * Program:   BioRadio.exe                                                    *
 * Module:    bioutils.cpp                                                    *
 * Comment:   Functions required for bioradio
 * Version:   0.03                                                            *
 * Author:    Yvan Pearson Lecours                                            *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 12/15/2005 - First start                                           *
 * V0.02 - 1/1/2006   - Removed unnecessary functions for BCI2000
 * V0.03 - 5/1/2006   - Mod for production
 * V0.04 - 7/3/2006   - Minor changes for clarity,
 *                      juergen.mellinger@uni-tuebingen.de
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "bioutils.h"
#include "BR_defines.h"

#include <fstream>
#include <iomanip>

using namespace std;

// **************************************************************************
// Function:   getPort
// Purpose:    Converts int index to a char array
// Parameters: num [COM number index] (int)
// Returns:    The COM value associated with an index (char*)
// **************************************************************************
const char*
bioutils::GetPort(int num)
{
  switch(num)
  {
    case COM1:
      return "COM1";
    case COM2:
      return "COM2";
    case COM3:
      return "COM3";
    case COM4:
      return "COM4";
    case COM5:
      return "COM5";
    case COM6:
      return "COM6";
    case COM7:
      return "COM7";
    case COM8:
      return "COM8";
    case COM9:
      return "COM10";
    case COM10:
      return "COM11";
    case COM11:
      return "COM12";
    case COM12:
      return "COM12";
    case COM13:
      return "COM13";
    case COM14:
      return "COM14";
    case COM15:
      return "COM15";
    case AUTO:
      return "AUTO";
    default:
      return NULL;
  }
}

// **************************************************************************
// Method:      getBioRadioRangeValue
// Purpose:     (1) Gets the proper value associated with a specifed voltage
//                  range
// Parameters:  range (int)
// Returns:    range definition (double)
// **************************************************************************
double
bioutils::GetBioRadioRangeValue(int range)
{
  switch(range)
  {
    case _100mV:
      return RANGE100mV;
    case _50mV:
      return RANGE50mV;
    case _25mV:
      return RANGE25mV;
    case _12mV:
      return RANGE12mV;
    case _6mV:
      return RANGE6mV;
    case _3mV:
      return RANGE3mV;
    case _1p5mV:
      return RANGE1p5mV;
    case _750uV:
      return RANGE750uV;
    default:
      return XERROR;
   }
}

// **************************************************************************
// Method:      vRange2IndexOfRange
// Purpose:     (1) Gets the index value associated with a specifed voltage
//                  range
// Parameters:  vRange (double)
// Returns:    range index (int)
// **************************************************************************
int
bioutils::VRange2IndexOfRange(double vRange)
{
  int indexOfRange = ZERO;
  double range[] = {RANGE750uV, RANGE1p5mV, RANGE3mV, RANGE6mV, RANGE12mV, RANGE25mV, RANGE50mV, RANGE100mV};

  for(size_t i = ZERO; i < sizeof(range)/sizeof(*range); ++i)
    if(range[i] == vRange)
      {
        indexOfRange = i;
        break;
      }

   return indexOfRange;
}

// **************************************************************************
// Function:   writeBioRadioConfig
// Purpose:    (1) Write a new config for the bioradio
// Parameters: fs [sampling frequency] (int)
//             bitRes [bit resolution] (int)
//             vRange [upper scale (voltage range)] (double)
//             pathFile [path of file to be created] (const char*)
// Returns:    Returns success flag if no errors
// **************************************************************************
int
bioutils::WriteBioRadioConfig(int fs, int bitRes, double vRange, const char *pathFile)
{
  ofstream file;
  file.open( pathFile );
  if( file.is_open() )
  {
    file << "[System]\n"
         << "SystemSampleRate=" << fs << "\n"
         << "BitResolution=" << bitRes << "\n"
         << "SweepsPerPacket=10\n"
         << "Format=0\n"
         << "[DAQ Board]\n";

    for( int i = 0; i < LAST_CHANNEL; ++i )
      file << "Channel_" << setfill( '0' ) << setw( 2 ) << i
           << "="
           << "BitResolution" << bitRes << ","
           << "Index=0,"
           << "SampleRate=" << fs << ","
           << "UpperScale=" << vRange << ","
           << "ADCUpper=65536,"
           << "ADCLower=0,"
           << "Truncate=0,"
           << "RangeIndex=" << VRange2IndexOfRange( vRange ) << ","
           << "Enabled=1,"
           << "InputType=0"
           << "\n";
  }
  return ( file.is_open() && file ) ? 0 : 1;
}

