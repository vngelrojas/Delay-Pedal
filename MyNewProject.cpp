#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;
using namespace daisy::seed;

#define MAX_DELAY static_cast<size_t>(48000 * 3.f) // Max delay of 3 seconds which is 20 bpm

Switch ON_BUTTON;                                   // The on/off button
Switch TEMPO_BUTTON;                                // The tap tempo button
Switch headSwitches[4];                            // One switch for each head
static DaisySeed hw;                               // The daisy seed harfware

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delayMems[4]; // 4 delay heads
static CrossFade cfade;                                 // Used to blend the wet/dry and maintain a constant mixed volume 

struct Delay
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

Delay DELAYS[4];                  // This creates a delay structure to store delay parameters
bool DELAY_ON[4];                  // Each delay head will be turned on/off independently
float FEEDBACK = 0.8f;            // The feedback parameter for all delays
float MAX_FEEDBACK = 1.1f;         // Max value of feedback knob, maxFeedback=1 -> forever repeats but no selfoscillation, values over 1 allow runaway feedback fun
float drywet_ratio = 0.5f;        // Drywet_ratio=0.0 is effect off
const float MAX_DELAY_SEC = 3.0f; // Max amount of seconds allowed to get 20 bpm
const float MIN_DELAY_SEC = 0.6f; // Min amount of seconds allowed to get 100 bpm
bool onButtonWasPressed = false;  // Flag for turning on/off delays, replace with onButton.risingEdge()
volatile float BPM = 20.0f;       // The BPM of delays
volatile bool TAPPING = false;    // True when user is TAPPING
TimerHandle TIMER;                // Timer that will be used to calculate bpm
TimerHandle::Config* configPtr;   // Pointer to config for timer, need because for some reason it wont let me create a config in global scope, so I create in main and point this to it
Parameter feedbackKnob;

// Initializes all the delays with the current value of hardware
void InitDelays(float samplerate);
// Sets the delays when there is a change
void SetDelays();
// Checks if TAPPING is occuring, updates bpm
void CheckTempo();
// Initialize buttons for the delay heads to the pins we want
void InitHeadButtons();
// Processes the controls and updates values that changed
void ProcessControls();
// Kills all delays (but doesnt delete them from delay line)
void StopAllDelays();

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        // 
        ProcessControls();
        // Check for tempo change
        CheckTempo();
        // Set delays with new bpm
        SetDelays();            


        ON_BUTTON.Debounce();
        // Check if the button is pressed and was not previously pressed
        if(ON_BUTTON.Pressed() && !onButtonWasPressed)
        {
            onButtonWasPressed = true; // set the flag to indicate that the button was pressed
            StopAllDelays();
        }

        // Check if the button was released
        if(!ON_BUTTON.Pressed() && onButtonWasPressed)
        {
            onButtonWasPressed = false; // reset the flag
        }

        float final_mix = 0;         // The final float value that will be outputted
	    float all_delay_signals = 0; // Summation of all delay signals
        float nonConstInput;         // Will store the a copy of input float value at in[channel][i] but will be but non-const,
                                     // Needed because cfade.SetPos requieres non-const  

    

        // Create the delayed signal for each of the 4 delays
		for(int d = 0; d < 4; d++)
        {
		    if(DELAY_ON[d] == true)
            {
                DELAYS[d].feedback = FEEDBACK;                    // Set the feedback of the delay to the global FEEDBACK
                all_delay_signals += DELAYS[d].Process(in[0][i]); // Sum all the processed signals
		    }
            else if(DELAY_ON[d] == false)
            {
                // delays[d].delay->Reset();
                // SetDelays();
            }
        }

        nonConstInput = in[0][i];

		// Use a crossfade object to maintain a constant power while mixing the delayed/raw audio mix
		cfade.SetPos(drywet_ratio);
		final_mix = cfade.Process(nonConstInput, all_delay_signals);
		out[0][i]  = out [1][i] = final_mix; // this sends 'final_mix' to the left and right output
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


    //set blocksize.
    hw.SetAudioBlockSize(4);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    // Initialize the delay lines with the same sample rate as the hardware
    InitDelays(hw.AudioSampleRate());

    // Set params for CrossFade object
    cfade.Init();
    // Sets crossfade to maintain constant power, which will maintain a constant volume as we go from full dry to full wet on the mix control
    cfade.SetCurve(CROSSFADE_CPOW);

    // Start callback
    hw.StartAudio(AudioCallback);

    //TIMER*****************************************************************************************************
    TimerHandle::Config config;                             // Config for the timer
    config.dir = TimerHandle::Config::CounterDir::UP;       // Set it to count up 
    config.periph = TimerHandle::Config::Peripheral::TIM_2; // Use timer 2 which is 32 bit counter
    config.enable_irq = 1;                                  // Enable interrupt for user based callback
    configPtr = &config;                                    // Point our global config to the one we just made

    // Init the timer with our config
    TIMER.Init(config);
    // Make sure the timer is off
    TIMER.Stop();
    //*********************************************************************************************************


    //FEEDBACK KNOB INIT*************************************************************
    // Configure the knob to pin D15
    AdcChannelConfig adcConfig;
    adcConfig.InitSingle(hw.GetPin(15));
    hw.adc.Init(&adcConfig,1);
    
    AnalogControl fbk;
    // Init the analog control to the same pin, D15, which is ADC channel 0 on the datasheet
    fbk.Init(hw.adc.GetPtr(0),hw.AudioSampleRate());
    feedbackKnob.Init(fbk,0.00,MAX_FEEDBACK,Parameter::LINEAR);
    //*******************************************************************************

    hw.adc.Start();

    while(1) 
    {
        //Not sure why the delay, taken from the Daisy petal MultiDelay.cpp example
        System::Delay(6);

    }
}

void InitDelays(float samplerate)
{
    for(int i = 0; i < 4; i++)
    {
        // Initialize delay
        delayMems[i].Init();
        // Point the delay of the Delay object to its corresponding delayMem
        DELAYS[i].delay = &delayMems[i];
        // Init the feedback
	    DELAYS[i].feedback = 0; 
        // Make sure they are all off
        DELAY_ON[i] = false;
    }
}

void SetDelays()
{
    for(int i = 0; i < 4; i++)
    {
	    //The (i+0.25-i*0.75) just sets the delay intervals to 
        // (1/16 note, 1/8 note, dotted 1/8 note, 1/4 note - or 0.25,0.5,0.75,1) for i=0,1,2,3
        // The 2880000/BPM sets BPM of delay 
	    DELAYS[i].delayTarget = (i+0.25-i*0.75)* 2880000/BPM;
    }
}

void CheckTempo()
{
    

    uint32_t tick;    // The position of the counter when the second tap occurs
    uint32_t freq;    // The frequency of each tick of the timer in Hz.
    float seconds;    // The seconds elapsed between first and second tap


    tick = TIMER.GetTick();            
    freq = TIMER.GetFreq();    
    seconds = (float)tick / (float)freq; // Calculating seconds from timer as recomended by documentation         

    //Check if the timer has gone past our max delay, if so, abandon this tap tempo and keep the old one
    if(seconds > MAX_DELAY_SEC && TAPPING)
    {
        //hw.PrintLine("Over 3 sec");
        TIMER.DeInit();
        TIMER.Init(*configPtr);

        // Turn the timer off
        TIMER.Stop();
        TAPPING = false;         // Reset the TAPPING flag 
    }

    // Check if the button was clicked
    TEMPO_BUTTON.Debounce();
    if(TEMPO_BUTTON.RisingEdge() )
    {
        // The first tap
        if(TAPPING == false)
        {
            TAPPING = true; // Set the TAPPING flag

            // Start the timer and begin counting
            TIMER.Start(); 


        }
        // The second tap
        else 
        {
            TAPPING = false; // Reset TAPPING flag

            // Stop the timer
            TIMER.Stop();
            TIMER.DeInit();
            TIMER.Init(*configPtr);
            // Turn the timer off
            TIMER.Stop();

            // Only set new BPM if its greater than our minimum
            if(seconds > MIN_DELAY_SEC )
            {
                BPM = -33.3333f*(seconds)+120; // Set the BPM for the delays 
                // Set all delays to new bpm
            }
            else
                hw.PrintLine("Under .6");


            
        }
    

    }


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
    FEEDBACK = feedbackKnob.Process();

    int i;
    for(i = 0; i < 4;i++)
    {
        headSwitches[i].Debounce();
        if(headSwitches[i].RisingEdge())
        {
            DELAY_ON[i] = !DELAY_ON[i];
        }
    }
}

void StopAllDelays()
{
    for(int i = 0;i < 4;i++)
    {
        DELAY_ON[i] = false;
    }
}




/*
Max Delay = 48,000 * 3 = 144,000 : Which is three seconds, the 1/4 delay will be 20 bpm

x/20 bpm = 144,000, solving for x 
x = 2,880,000

2,880,000/BPM = The required delay to go into SetDelay() to be at the correct BPM


Create linear equation with 2 points (3,20) and (.6,100)
BPM = −33.3333(sec)+120, where max delay is 3 seconds and min delay is .6 seconds (100 bpm)

*/

