#include <Arduino.h>
#include "Lights.h"
#include "Digits.h"
#include "Strip.h"
#include "Game.h"
#include "Midi.h"
#include "Player.h"
#include "Puck.h"
#include "Trail.h"
#include "Animation.h"



// Pin for the hearbeat LED
#define P_HEARTBEAT 13


CMIDI     MIDI;
CDigits   Digits;
CLights   Lights;
CStrip    Strip;
CPlayer   PlayerLeft(CPlayer::LEFT, CPlayer::BLUE, 0);
CPlayer   PlayerRight(CPlayer::RIGHT, CPlayer::GREEN, 0);
CPuck     Puck;
CTrail    Trail;



volatile WoReg *timeValue;
volatile WoReg *timeReset;
unsigned long ticks = 0;
void setup() {  
  pinMode(P_HEARTBEAT, OUTPUT);
  digitalWrite(P_HEARTBEAT, LOW);
  MIDI.setup();
  Strip.setup();
  Lights.setup();
  Digits.setup();
  Puck.setup();
  Trail.setup();
  gameSetup();
  
  
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
  
  Lights.setButton(1);
  delay(1000);
}



/////////////////////////////////////////////////////////////////////
//int puckY=0;
//int q=0;
byte c=0;
unsigned long nextTick = 0;
#define TICK_SCALAR 900
CRaindropAnimation Drops;
void loop() 
{
  // If the time has reached our threshold then increment
  // our tick counter and reset the time. We can't use anything
  // that relies on interrupts (like millise) since they are disabled
  // most of the time, so instead we poll this timer. The result is
  // approximate, but it will do!
  if(*timeValue >= TICK_SCALAR)
  {
    ticks+=(*timeValue / TICK_SCALAR);
    *timeReset = TC_CCR_CLKEN | TC_CCR_SWTRG;   
  }
  
  // Heartbeat LED
  if(ticks > nextTick)
  {
    nextTick = ticks + 200;
    digitalWrite(P_HEARTBEAT, c);
    c=!c;
  }
  if(MIDI.read() == 0x90 && MIDI.param2())
  {
    switch(MIDI.param1())
    {
      case 60:
        Digits.setBoth(0xff,0);
        break;
      case 61:
        Digits.setBoth(0,0xff);
        break;
    }
  }
  Strip.clear();
  Trail.render();
  PlayerLeft.render();
  PlayerRight.render();
  Puck.render();
  Strip.refresh();
  
  gameRun(ticks);
  PlayerLeft.run(ticks);
  PlayerRight.run(ticks);
  Puck.run(ticks, Trail);
  Trail.run(ticks);
  
}

