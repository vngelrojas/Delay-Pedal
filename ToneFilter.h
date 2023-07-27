#pragma once
#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;
using namespace daisy::seed;

class ToneFilter
{
    public:
        /**
         * @brief Construct a new Tone Filter object by init lp and hp filter
         * 
         * @param sampleRate Sample rate to init filters
         */
        ToneFilter(float sampleRate);
        /**
         * @brief Set the Freq of filters
         * 
         * @param knobValue Value from -1 to 1
         */
        void setFreq(float knobValue);
        /**
         * @brief 
         * 
         * @param in 
         * @return float 
         */
        float process(float in);
        /**
         * @brief Gets the factor to multiply to signal by in sound balanceing. 
         * Makes HP less harsh and LP louder
         * 
         * @return Factor to multiply signal by 
         */
        float getFactor();
    private:
        Tone lowPassFilter;
        ATone highPassFilter;
        bool lowPassOn;
        bool highPassOn;
        float factor;
};

