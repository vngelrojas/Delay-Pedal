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
            DelayLine<float, MAX_DELAY> *delay; // Will point to a delayMem
            float currentDelay;                 // The current delay 
            float delayTarget;                  // The delay target that currentDelay will ramp up/down to 
            float feedback;                     // Feedback level of the delay

            float process(float in)
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
        /**
         * @brief Constructs the delay heads
         */
        Delayy();
        /**
         * @brief Stops all the delays from playing but does not delete their sound, 
         * meaning if you toggle a head, it will keep playing previous input
         * 
         */
        void stopAll();
        /**
         * @brief Sets the bpm for delay
         * 
         * @param bpm The new bpm for delay
         */
        void setBPM(const int& bpm);
        /**
         * @brief Set the Feedback of delay
         * 
         * @param feed The new feedback for delay
         */
        void setFeedback(const float& feed);
        /**
         * @brief Will proccess all delays using delayline class from DaisySP
         * 
         * @param in Current audio input
         * @return Summation of all processed delay signals
         */
        float process(float in);
        /**
         * @brief Will toggle on/off a single delay head
         * 
         * @param headNumber The delay head to toggle on/off
         */
        void toggleHead(const int& headNumber);
    private:
        bool delayHeadOn[NUM_OF_DELAY_HEADS]; // Each delay head will be turned on/off independently
        int bpm;                              // The bpm being used being used by all heads, might be useful if you add other features that need bpm
        float feedback;                       // The feedback of delays
    public:
        DelayHead delayHeads[NUM_OF_DELAY_HEADS];                   // Array of however many delays you want 

    
};