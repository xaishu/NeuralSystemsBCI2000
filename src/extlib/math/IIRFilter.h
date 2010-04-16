////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An IIR filter implementation.
//   The filter's behavior is determined by the Gain, Poles, and Zeros
//   properties which define a rational transfer function in terms of
//   its numerator roots (Zeros), denominator roots (Poles), and a
//   multiplicative factor (Gain); use the FilterDesign class to calculate
//   Zeros, Poles, and Gain from desired filter properties.
//
//   Due to its implementation as a sequence of order 1 stages in DF I form,
//   the filter is numerically stable regardless of filter order.
//
//   The filter's Process() method computes an output signal from an input
//   signal, and saves its internal state (delays) for the next call to
//   Process(). Thus, continuous processing may be achieved by calling
//   Process() repeatedly.
//   Process() is templatized for its argument signal type T. This type must
//   provide the following member functions:
//     T::Channels() to return the number of channels,
//     T::Elements() to return the number of elements (samples),
//     T::operator()(channel, sample) for read/write access,
//     T::operator=() to copy one instance of T into another.
//
//   The filter's Initialize() method adapts the number of internal delays
//   to the number of channels provided, and resets all delays to zero.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef IIR_FILTER_H
#define IIR_FILTER_H

#include "FilterDesign.h"
#include <cassert>

template<typename Real>
class IIRFilter
{
 public:
  typedef FilterDesign::ComplexVector ComplexVector;

  IIRFilter()
    {}
  ~IIRFilter()
    {}

  // Properties
  const ComplexVector& Zeros() const
    { return mZeros; }
  IIRFilter& SetZeros( const ComplexVector& z )
    { mZeros = z; return Initialize(); }
  const ComplexVector& Poles() const
    { return mPoles; }
  IIRFilter& SetPoles( const ComplexVector& p )
    { mPoles = p; return Initialize(); }
  const Real& Gain() const
    { return mGain; }
  IIRFilter& SetGain( const Real& g )
    { mGain = g; return Initialize(); }
  int Channels() const
    { return mDelays.size(); }
  IIRFilter& SetChannels( int c )
    { return Initialize( c ); }

  // Methods
  IIRFilter& Initialize();
  IIRFilter& Initialize( size_t inChannels );
  template<typename T>
   IIRFilter& Process( const T&, T& );

 private:

  Real                       mGain;
  ComplexVector              mZeros,
                             mPoles;
  std::vector<ComplexVector> mDelays;
};


template<typename Real>
inline IIRFilter<Real>&
IIRFilter<Real>::Initialize()
{
  size_t channels = mDelays.size();
  return Initialize( channels );
}

template<typename Real>
inline IIRFilter<Real>&
IIRFilter<Real>::Initialize( size_t inChannels )
{
  mDelays.clear();
  mDelays.resize( inChannels, ComplexVector( mZeros.size() + 1, 0 ) );
  return *this;
}

template<typename Real>
template<typename T>
inline IIRFilter<Real>&
IIRFilter<Real>::Process( const T& Input, T& Output )
{
  assert( mZeros.size() == mPoles.size() );
  size_t numStages = mZeros.size();
  if( numStages == 0 )
  {
    Output = Input;
  }
  else
  {
    for( int ch = 0; ch < Input.Channels(); ++ch )
    {
      assert( mDelays[ch].size() == numStages + 1 );
      for( int sample = 0; sample < Input.Elements(); ++sample )
      {
        // Implementing the filter as a sequence of complex-valued order 1
        // stages in DF I form will give us higher numerical stability and
        // lower code complexity than a sequence of real-valued order 2 stages.
        // - Numerical stability: Greatest for lowest order stages.
        // - Code complexity: Poles and zeros immediately translate into complex
        //    coefficients, and need not be grouped into complex conjugate pairs
        //    as would be the case for real-valued order 2 stages.
        FilterDesign::Complex stageOutput = Input( ch, sample ) * mGain;
        for( size_t stage = 0; stage < numStages; ++stage )
        {
          FilterDesign::Complex stageInput = stageOutput;
          stageOutput = stageInput
            - mZeros[stage] * mDelays[ch][stage]
            + mPoles[stage] * mDelays[ch][stage+1];
          mDelays[ch][stage] = stageInput;
        }
        mDelays[ch][numStages] = stageOutput;
        Output( ch, sample ) = real( stageOutput );
      }
    }
  }
  return *this;
}

#endif // IIR_FILTER_H
