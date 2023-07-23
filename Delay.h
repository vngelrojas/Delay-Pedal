#include "daisysp.h"
#include "daisy_seed.h"
#pragma once
using namespace daisysp;
using namespace daisy;
using namespace daisy::seed;

#define MAX_DELAY static_cast<size_t>(48000 * 3.f)  // Max delay of 3 seconds which is 20 bpm
const int NUM_OF_DELAY_HEADS = 4;                   // # of delay heads, simply change this number if you want more or less delay heads

class Delayy
{
private:
    struct DelayHead
    {
        DelayLine<float, MAX_DELAY> *delay;                     // Will point to a delayMem
        float currentDelay;                                     // The current delay 
        float delayTarget;                                      // The delay target that currentDelay will ramp up/down to 
        float feedback;                                         // Feedback level of the delay

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
public:
    Delayy()
    {
        for(int i = 0; i < NUM_OF_DELAY_HEADS; i++)
        {
            // Initialize delay line
            delayMems[i].Init();
            // For the current head, point its delayline ptr to one from delayMems
            DELAYS[i].delay = &delayMems[i];
            // Init the feedback
            DELAYS[i].feedback = 0; 
            // Make sure they are all off
            DELAY_ON[i] = false;
        }
    }
    void StopAllDelays()
    {
        for(int i = 0;i < 4;i++)
        {
            DELAY_ON[i] = false;
        }
    }
private:
    DelayLine<float, MAX_DELAY>  delayMems[NUM_OF_DELAY_HEADS]; // Array of 4 delay lines for each of the 4 heads
    DelayHead DELAYS[NUM_OF_DELAY_HEADS];                                    // This creates a delay structure to store delay parameters
    bool DELAY_ON[NUM_OF_DELAY_HEADS];                                       // Each delay head will be turned on/off independently

    
};