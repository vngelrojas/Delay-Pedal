#include "Delay.h"

Delayy::Delayy()
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
    bpm = 20;
}

void Delayy::stopAll()
{
    for(int i = 0;i < NUM_OF_DELAY_HEADS;i++)
        DELAY_ON[i] = false;
}

void Delayy::setBPM(const int& bpm)
{
    this->bpm = bpm;
    for(int i = 0; i < NUM_OF_DELAY_HEADS; i++)
    {
        //The (i+0.25-i*0.75) just sets the delay intervals to 
        // (1/16 note, 1/8 note, dotted 1/8 note, 1/4 note - or 0.25,0.5,0.75,1) for i=0,1,2,3
        // The 2880000/BPM sets BPM of delay 
        DELAYS[i].delayTarget = (i+0.25-i*0.75)* 2880000/bpm;
    }
}

void Delayy::setFeedback(const float& feed)
{
    for (int i = 0; i < NUM_OF_DELAY_HEADS; i++)
        DELAYS[i].feedback = feed;
}

float Delayy::process(float in)
{
    float allDelaySignals = 0;

    for (int i = 0; i < NUM_OF_DELAY_HEADS; i++)
        if(DELAY_ON[i])
            allDelaySignals = DELAYS[i].process(in);

    return allDelaySignals;
    
}

void Delayy::toggleHead(const int& headNumber)
{
    if(headNumber > NUM_OF_DELAY_HEADS || headNumber < 0)
        return;
    else
        DELAY_ON[headNumber] = !DELAY_ON[headNumber];
}