#include "daisysp.h"
#include "daisy_seed.h"
#pragma once
#define MAX_DELAY static_cast<size_t>(48000 * 3.f) // Max delay of 3 seconds which is 20 bpm
using namespace daisysp;
using namespace daisy;
using namespace daisy::seed;
class Delayy
{
    DelayLine<float, MAX_DELAY> *delay;        // Will point to a delayMem
    float                        currentDelay; // The current delay 
    float                        delayTarget;  // The delay target that currentDelay will ramp up/down to 
    float                        feedback;     // Feedback level of the delay

    float Process(float in)
    {

        float readSample; // Will store a sample from the delay line

        // This smoothes out the delay when you turn the delay control?
        fonepole(currentDelay, delayTarget, .0002f); 
        // Set delay time
        delay->SetDelay(currentDelay);          
        readSample = delay->Read(); // Read in the next sample from the delay line

        // Write the readSample * the feedback amount + the input sample into the delay line
        delay->Write((feedback * readSample) + in); 
        
        return readSample;
    }
};