#include "ToneFilter.h"

ToneFilter::ToneFilter(float sampleRate)
{
    lowPassFilter.Init(sampleRate);
    highPassFilter.Init(sampleRate); 
}

void ToneFilter::setFreq(float& freq)
{
    lowPassFilter.SetFreq(freq);
}