////////////////////////////////////////////////////////////////////////////////
// $Id: vAmpAdc.h 1967 2008-05-19 15:00:13Z awilson $
// Author: jadamwilson2@gmail.com
// Description: BCI2000 Source Module for BrainProducts V-Amp devices.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef VAMP_ADC_H
#define VAMP_ADC_H

#include "GenericADC.h"
#include "GenericVisualization.h"
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <Classes.hpp>
#include <SyncObjs.hpp>
#include "PrecisionTime.h"
#include "FirstAmp.h"
#include "vAmpChannelInfo.h"
#include "vAmpThread.h"

/*----------------------------------------------------------------------------*/
/* Defines */

#include "vAmpDefines.h"

using namespace std;

class vAmpADC : public GenericADC
{
 public:
			   vAmpADC();
	virtual      ~vAmpADC();

	virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
	virtual void Initialize( const SignalProperties&, const SignalProperties& );
	virtual void Process( const GenericSignal&, GenericSignal& );
	virtual void Halt();

 private:
	friend class vAmpThread;
	vAmpThread *mAcquire;
	int mNumEEGchannels;
	vector<int> mChList;
	int mTimeoutMs;
	vector<int> mDevList;
	bool mHighSpeed;

	float getSampleRate(float);
	int getDecimation() const;
};

#endif // VAMP_ADC_H



