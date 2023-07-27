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
         * @param freq The freq to set
         */
        void setFreq(float freq);
        /**
         * @brief 
         * 
         * @param in 
         * @return float 
         */
        float process(float in);
    private:
        Tone lowPassFilter;
        ATone highPassFilter;
};

