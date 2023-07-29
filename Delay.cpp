#include "Delay.h"



Delayy::Delayy()
{
    //delayMems = temp;
    for(int i = 0; i < 4; i++)
    {
        // Init the feedback
        delayHeads[i].feedback = 0; 
        // Make sure they are all off
        delayHeadOn[i] = false;
    }
    bpm = 20;

}

void Delayy::stopAll()
{
    for(int i = 0;i < NUM_OF_DELAY_HEADS;i++)
        delayHeadOn[i] = false;
}

void Delayy::setBPM(float bpm)
{
    this->bpm = bpm;
}

void Delayy::setFeedback(const float& feed)                           
{
    
    this->feedback = feed;

}

float Delayy::process(float in)
{
    // if(clearing)
    //     return 0;
    // else
    //{

        float allDelaySignals = 0;

        for (int i = 0; i < NUM_OF_DELAY_HEADS; i++)
        {
            delayHeads[i].feedback = this->feedback;
            //The (i+0.25-i*0.75) just sets the delay intervals to 
            // (1/16 note, 1/8 note, dotted 1/8 note, 1/4 note - or 0.25,0.5,0.75,1) for i=0,1,2,3
            delayHeads[i].delayTarget = (i+0.25-i*0.75)* (48000*(60/bpm));
            if(delayHeadOn[i])
                allDelaySignals += delayHeads[i].process(in);
        }

        return allDelaySignals;
    //}
}

void Delayy::toggleHead(const int& headNumber)
{
    if(headNumber >= NUM_OF_DELAY_HEADS || headNumber < 0)
        return;
    else
        delayHeadOn[headNumber] = !delayHeadOn[headNumber];
}

void Delayy::clear()
{
    
    for (int i = 0; i < NUM_OF_DELAY_HEADS; i++)
    {
        feedback = 0;
        delayHeads[i].feedback = 0;
        delayHeads[i].process(0);
    }
    
    
}
float Delayy::getBPM()
{
    return bpm;
}