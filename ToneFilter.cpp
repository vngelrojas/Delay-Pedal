#include "ToneFilter.h"

ToneFilter::ToneFilter(float sampleRate)
{
    lowPassFilter.Init(sampleRate);
    highPassFilter.Init(sampleRate); 
    // Set filters to extreme to not affect audio at the very start 
    float lpFreq = 1000000.0f;
    float hpFreq = 0.f;
    lowPassFilter.SetFreq(lpFreq);
    highPassFilter.SetFreq(hpFreq);
    lowPassOn = false;
    highPassOn = false;
}

void ToneFilter::setFreq(float knobValue)
{
    if(knobValue < 0.00f)
    {
        float lpFreq = 5000.0f*(powf(10,2*knobValue))+100.f;
        lowPassFilter.SetFreq(lpFreq);
        lowPassOn = true;
        highPassOn = false;
    }
    else
    {
        float hpFreq = 5000.0f*powf(10,2.f*knobValue-2);
        highPassFilter.SetFreq(hpFreq);
        lowPassOn = false;
        highPassOn = true;
    }
}

float ToneFilter::process(float in)
{
    // Incase this is called before the tone knob is read freq is set 
    if(!lowPassOn && !highPassOn)
        return in;
    else if(lowPassOn)
        return lowPassFilter.Process(in);
    else
        return highPassFilter.Process(in);
}