#include "ToneFilter.h"

ToneFilter::ToneFilter(float sampleRate)
{
    lowPassFilter.Init(sampleRate);
    highPassFilter.Init(sampleRate); 
}

void ToneFilter::setFreq(float freq)
{
    lowPassFilter.SetFreq(freq);
}

float ToneFilter::process(float in)
{
    return lowPassFilter.Process(in);
}