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
    factor = 1.f;
}

void ToneFilter::setFreq(float knobValue)
{
    if(knobValue < 0.00f)
    {
        float lpFreq = 5000.0f*(powf(10,2*knobValue))+100.f;
        lowPassFilter.SetFreq(lpFreq);
        lowPassOn = true;
        highPassOn = false;
        // As knobValue goes closer -1, factor increases
        factor = 1.f + (-1.f*knobValue/3.5f);
    }
    else
    {
        float hpFreq = 5000.0f*powf(10,2.f*knobValue-2);
        highPassFilter.SetFreq(hpFreq);
        lowPassOn = false;
        highPassOn = true;
        // As knobValue gets closer to 1, factor decreases 
        factor = 1.f-knobValue/1.75f;
    }
}

float ToneFilter::process(float in)
{
    // Incase this is called before the tone knob is read freq is set OR somehow both are on 
    if((!lowPassOn && !highPassOn) || (lowPassOn && highPassOn))
        return in;
    else if(lowPassOn)
        return lowPassFilter.Process(in);
    else
        return highPassFilter.Process(in);
}

float ToneFilter::getFactor()
{
    return factor;
}