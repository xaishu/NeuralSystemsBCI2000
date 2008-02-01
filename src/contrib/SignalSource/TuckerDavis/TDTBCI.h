/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef TDTBCIH
#define TDTBCIH

#include "GenericADC.h"
#include "TDTADC.h"
#include "RPCOXLib_OCX.h"
#include "ZBUSXLib_OCX.h"

class TDTBCI : public GenericADC
{
public:
	TDTBCI();
    ~TDTBCI();

    //
    void Preflight(const SignalProperties&, SignalProperties&) const;
    void Initialize(const SignalProperties&, const SignalProperties&);
    void Process( const GenericSignal&, GenericSignal&);
    void Halt();

private:
    class TRPcoX *RPcoX1;
    class TRPcoX *RPcoX2;
    class TZBUSx *ZBus;
    
    int	mSourceCh;
    int	mSampleBlockSize;
    int mSamplingRate;
    int mOffset;
    int nChannels;
    int nChannels1;
    int nChannels2;
    int nProcessors1;
    int nProcessors2;
    double LPFfreq;
    double HPFfreq;
    double notchBW;
    double TDTsampleRate;
    double TDTgain;
    int TDTbufSize;
    int blockSize;
    int curindex, stopIndex, indexMult;
    int devAddr[2];
    bool use2RX5;
    bool useECG;
    float ECGgain;
    int ECGchannel;
    int ECGoffset;
    int ECGstopIndex;
    
	float *dataA;// = new float[valuesToRead];
	float *dataB;// = new float[valuesToRead];
	float *dataC;// = new float[valuesToRead];
	float *dataD;// = new float[valuesToRead];
    /*float *dataA2;
    float *dataB2;
    float *dataC2;
    float *dataD2;*/
    //float *ECGdata;
};

//---------------------------------------------------------------------------
#endif