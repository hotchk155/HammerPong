////////////////////////////////////////////////////////////////////////////
// HAMMER PONG MAIN ARDUINO SKETCH FILE
// CODE FOR ARDUINO DUE
////////////////////////////////////////////////////////////////////////////

//
// INCLUDE FILES
//
#include <Arduino.h>
#include "Heartbeat.h"
#include "Game.h"

//
// MACRO DEFS
//
#define TICK_SCALAR 900  // How many counter increments there are in a tick

//
// GLOBALS
//
CHeartbeat Heartbeat;  // Handler for heartbeat LED
CGame Game;            // Handler for Game

volatile WoReg *timeValue;  // Counter register pointer for timer used to implement "ticks"
volatile WoReg *timeReset;  // Counter reset register pointer for timer used to implement "ticks"
unsigned long ticks = 0;    // Tick counter used for scheduling. A tick is of the order of 1ms

////////////////////////////////////////////////////////////////////////////
void setup() 
{    
  // Setup the game
  Heartbeat.setup();
  Game.setup();

  // Short wait before we run it (hopefully avoiding giving kids any
  // reason to hammer the reset button over and over to get glitchy sounds etc)
  delay(1000);
  
  // Setup up a timer that we can use to provide some sense
  // of time for pacing the game. Since the strip update code
  // disables interrupts we can't rely on millis. Instead we 
  // use a poll of a counter to provide an approximate time
  // base on which we can time events  
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk((uint32_t)TC6_IRQn);
  TC_Configure(TC2, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP | TC_CMR_TCCLKS_TIMER_CLOCK4);
  TC_Start(TC2, 0);  
  timeValue = &(TC2->TC_CHANNEL[0].TC_CV);  
  timeReset = &(TC2->TC_CHANNEL[0].TC_CCR);
  *timeReset = TC_CCR_CLKEN | TC_CCR_SWTRG;     

}

/////////////////////////////////////////////////////////////////////
void loop() 
{
  // If the time has reached our threshold then increment
  // our tick counter and reset the time. We can't use anything
  // that relies on interrupts (like millise) since they are disabled
  // most of the time, so instead we poll this timer. The result is
  // approximate, but it will do!
  if(*timeValue >= TICK_SCALAR)
  {
    unsigned long oldTicks = ticks;
    ticks+=(*timeValue / TICK_SCALAR);
    *timeReset = TC_CCR_CLKEN | TC_CCR_SWTRG;   
    
    // check for counter rollover.. a tick value of 
    // zero is used to signal rollover
    if(ticks < oldTicks)
     ticks = 0;
  }

  // run the game and render to strip
  Game.run(ticks);  
  Game.render();
  
  // flash the diagnostic LED
  Heartbeat.run(ticks);
}

