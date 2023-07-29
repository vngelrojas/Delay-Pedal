#include "Delay.h"
#include "ToneFilter.h"
#include "TapTempo.h"

using namespace daisysp;
using namespace daisy;
using namespace daisy::seed;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delayMems[4];

Delayy delay;
ToneFilter tone(48000.f);
static Balance balance;
TapTempo tapTempo;

Switch ON_BUTTON;                 // The on/off button
Switch TEMPO_BUTTON;              // The tap tempo button
Switch headSwitches[4];           // One switch for each head
static DaisySeed hw;              // The daisy seed harfware
static CrossFade cfade;           // Used to blend the wet/dry and maintain a constant mixed volume 


float MAX_FEEDBACK = 1.1f;        // Max value of feedback knob, maxFeedback=1 -> forever repeats but no selfoscillation, values over 1 allow runaway feedback fun
bool onButtonWasPressed = false;  // Flag for turning on/off delays, replace with onButton.risingEdge()
float SAMPLE_RATE = 48000.f;
bool PASS_THROUGH = true;

Parameter feedbackKnob;
Parameter toneKnob;
Parameter timeKnob;
Parameter dryKnob;

// Sets the delays when there is a change
void CheckTempo();
// Initialize buttons for the delay heads to the pins we want
void InitHeadButtons();
// Processes the controls and updates values that changed
void ProcessControls();
// Will point the delay heads in class to the global delay lines
void initDelay();

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        // Update control parameters
        ProcessControls();
        // Check for tempo change
        CheckTempo();
        // Set delays with new bpm
        delay.setBPM(tapTempo.getBPM());       


        
        if(PASS_THROUGH)
        {
            out[0][i]  = out [1][i] = in[0][i];//filter; // this sends 'final_mix' to the left and right output
            delay.clear();

        }
        else
        {

        
            float finalMix = 0;            // The final float value that will be outputted
            float allDelaySignals = 0;     // Summation of all delay signals
            float nonConstInput = in[0][i];// Will store the a copy of input float value but non-const : cfade.SetPos requieres non-const  

        

            allDelaySignals = delay.process(in[0][i]);
            float preFilter = allDelaySignals;
            allDelaySignals = tone.process(allDelaySignals);
            allDelaySignals = balance.Process(allDelaySignals,preFilter*tone.getFactor());



            // Use a crossfade object to maintain a constant power while mixing the delayed/raw audio mix
            cfade.SetPos(dryKnob.Process());
            finalMix = cfade.Process(nonConstInput, allDelaySignals);
            out[0][i]  = out [1][i] = finalMix;//filter; // this sends 'final_mix' to the left and right output
        }
    }
}

int main(void)
{
    // Initialize seed hardware.
    hw.Init();

    // Setting up serial and printing
    hw.StartLog();

    // Initialize the button to D28, Pin 35 to be the on/off button
    ON_BUTTON.Init(hw.GetPin(28),1000);
    // Init tap temp Button to D25, pin 32
    TEMPO_BUTTON.Init(hw.GetPin(25),1000);
    // Init the head on/off buttons
    InitHeadButtons();


    //set blocksize and sample rate
    hw.SetAudioBlockSize(4);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    // Set params for CrossFade object
    cfade.Init();
    // Sets crossfade to maintain constant power, which will maintain a constant volume as we go from full dry to full wet on the mix control
    cfade.SetCurve(CROSSFADE_CPOW);


    //KNOB INIT************************************************************************
    AdcChannelConfig fbkConfig;
    fbkConfig.InitSingle(A0);
    AdcChannelConfig toneConfig;
    toneConfig.InitSingle(A1);
    AdcChannelConfig timeConfig;
    timeConfig.InitSingle(A2); 
    AdcChannelConfig dryConfig;
    dryConfig.InitSingle(A3);

    // If adding more knobs, create config above, and update  array and # of configs
    AdcChannelConfig configs [4] = {fbkConfig,toneConfig,timeConfig,dryConfig};
    hw.adc.Init(configs,4);

    AnalogControl fbk;
    fbk.Init(hw.adc.GetPtr(0),hw.AudioSampleRate());
    feedbackKnob.Init(fbk,0.00,MAX_FEEDBACK,Parameter::LINEAR);

    AnalogControl tne;
    tne.Init(hw.adc.GetPtr(1),hw.AudioSampleRate());
    toneKnob.Init(tne,-1.f,1.f,Parameter::LINEAR);

    AnalogControl time;
    time.Init(hw.adc.GetPtr(2),hw.AudioSampleRate());
    timeKnob.Init(time,30.f,240.f,Parameter::LINEAR);

    AnalogControl dry;
    dry.Init(hw.adc.GetPtr(3),hw.AudioSampleRate());
    dryKnob.Init(dry,0,1,Parameter::LINEAR);

    hw.adc.Start();
    //*******************************************************************************


    // Init the delay object with the globally scoped delayMems array
    initDelay();
    // Init the balance object
    balance.Init(SAMPLE_RATE);
    // Start callback
    hw.StartAudio(AudioCallback);

    while(1) 
    {
        //Not sure why the delay, taken from the Daisy petal MultiDelay.cpp example
        System::Delay(6);

    }
}

void CheckTempo()
{
    bool tap = false;
    TEMPO_BUTTON.Debounce();
    if(TEMPO_BUTTON.RisingEdge())
        tap = true;

    tapTempo.update(tap);
}

void InitHeadButtons()
{
    // Iterate through head switch array
    for(int i = 0; i < 4;i++)
    {
        // Init buttons to D21 - D24
        headSwitches[i].Init(hw.GetPin(21+i),1000);
    }
}

void ProcessControls()
{
    delay.setFeedback(feedbackKnob.Process());
    tone.setFreq(toneKnob.Process());
    float tempoFromKnob = timeKnob.Process();

    ON_BUTTON.Debounce();
    if(ON_BUTTON.RisingEdge())
    {
        PASS_THROUGH = !PASS_THROUGH;
    }

    // Only update the delay time from the knob if it falls within + or - the current bpm to prevent extreme jumps in bpm. 
    // For example, you tap a bpm of 200 but the knob is at bpm 50, when you move the knob nothing will change until it reaches 200 bpm.
    // Once the knob reaches current bpm, it is free to change it up or down 
    if( tempoFromKnob > tapTempo.getBPM() - 1 &&  tempoFromKnob < tapTempo.getBPM() + 1 )
        tapTempo.setBPM(tempoFromKnob);
    
    for(int i = 0; i < 4;i++)
    {
        headSwitches[i].Debounce();
        if(headSwitches[i].RisingEdge())
        {            
            delay.toggleHead(i);
        }
    }
}

void initDelay()
{
    for (int i = 0; i < 4; i++)
    {
        delayMems[i].Init();
        delay.delayHeads[i].delay = &delayMems[i];
    }
}



