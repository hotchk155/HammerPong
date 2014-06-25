/*
  Don't rely on millis... gets slowed when interrupts are turned off during strip updates
*/
#include <Arduino.h>
#include "Lights.h"
#include "Digits.h"
#include "Strip.h"
#include "Game.h"


// Pin for the hearbeat LED
#define P_HEARTBEAT 13


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//
// LOW LEVEL MIDI HANDLING
//
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// state variables
byte midiInRunningStatus;
byte midiOutRunningStatus;
byte midiNumParams;
byte midiParams[2];
char midiParamIndex;

////////////////////////////////////////////////////////////////////////////////
// MIDI INIT
void midiSetup()
{
  // init the serial port
  Serial.begin(31250);
  Serial.flush();

  midiInRunningStatus = 0;
  midiOutRunningStatus = 0;
  midiNumParams = 0;
  midiParamIndex = 0;
}


////////////////////////////////////////////////////////////////////////////////
// MIDI READ
byte midiRead()
{
  // loop while we have incoming MIDI serial data
  while(Serial.available())
  {    
    // fetch the next byte
    byte ch = Serial.read();

    if(!!(ch & 0x80))
    {
      midiParamIndex = 0;
      midiInRunningStatus = ch; 
      switch(ch & 0xF0)
      {
        case 0xD0: //  Channel Pressure  1  pressure  
          midiNumParams = 1;
          break;    
        case 0x80: //  Note-off  2  key  velocity  
        case 0x90: //  Note-on  2  key  veolcity  
        case 0xA0: //  Aftertouch  2  key  touch  
        case 0xB0: //  Continuous controller  2  controller #  controller value  
        case 0xC0: //  Patch change  2  instrument #   
        case 0xE0: //  Pitch bend  2  lsb (7 bits)  msb (7 bits)  
        default:
          midiNumParams = 2;
          break;        
      }
    }    
    else if(midiInRunningStatus)
    {
      // gathering parameters
      midiParams[midiParamIndex++] = ch;
      if(midiParamIndex >= midiNumParams)
      {
        midiParamIndex = 0;
        return midiInRunningStatus;        
      }
    }
  }
  return 0;
}



/*

/////////////////////////////////////////////////////////////////////
#define HALF_STRIP_JOIN 32 // represents a virtual "out of view" section at top of each strip
#define LEFT_STRIP_MIN (PIX_LENGTH-HALF_STRIP_JOIN)
#define LEFT_STRIP_MAX (-HALF_STRIP_JOIN)
#define RIGHT_STRIP_MAX HALF_STRIP_JOIN
#define RIGHT_STRIP_MIN (PIX_LENGTH+HALF_STRIP_JOIN)

typedef struct
{
  int x;         // which column (strip)
  float y;         // row position (0 is at the top of the strip)
  float intensity; // spark intensity, which ranges from bright white to dull red
  float dy;
} SPARK;

#define NUM_SPARKS 20
SPARK sparks[NUM_SPARKS];
void sparksInit()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {
    sparks[i].x=0;
    sparks[i].y=0;
    sparks[i].intensity=0;
    sparks[i].dy=0;
  }
}
void sparksRender()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {    
    if(sparks[i].intensity>5)
    {
      SPARK *pSpark = &sparks[i];
      byte *pCell = NULL;
      if(pSpark->x < 3)
      {
        // LEFT SIDE
        int y = pSpark->y - LEFT_STRIP_MIN;
        if(y >= 0 && y < PIX_LENGTH)
          pCell = &pixBuffer[pSpark->x][3*y];
      }
      else 
      {
        // RIGHT SIDE
        int y = pSpark->y - LEFT_STRIP_MIN;
        if(y >= 0 && y < PIX_LENGTH)
          pCell = &pixBuffer[pSpark->x-3][3*(PIX_LENGTH - y)];
      }
      if(pCell)
      {
#define THR        50
        pCell[1] = sparks[i].intensity;
        if(sparks[i].intensity > THR)
          pCell[0] = pCell[2] = THR + sparks[i].intensity/2;
      }
    }
  }
}
void sparksExplode()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {
    sparks[i].x=random(3);
    sparks[i].y=LEFT_STRIP_MIN;
    sparks[i].intensity=(float)random(150)+50;
    sparks[i].dy=(float)random(300)/100;
  }
  
}
void sparksRun()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {
    if(sparks[i].intensity)
    {
      sparks[i].y+=sparks[i].dy;
      sparks[i].dy*=0.93;
      sparks[i].intensity*=0.95;
    }
  }
  
}

void puckRender(int y)
{

  int q=y;
  byte *p;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &pixBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &pixBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
}
*/
volatile WoReg *timeValue;
volatile WoReg *timeReset;
unsigned long ticks = 0;
void setup() {  
  pinMode(P_HEARTBEAT, OUTPUT);
  digitalWrite(P_HEARTBEAT, LOW);
  //sparksInit();
//  pixInit();
  midiSetup();
  stripSetup();
  lightsSetup();
  digitsSetup();
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
  
  lightsSetButton(1);
  delay(1000);
}
//int puckY=0;
//int q=0;
byte c=0;
unsigned long nextTick = 0;
#define TICK_SCALAR 900
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
    //sparksExplode();
  }
  //sparksRun();
  
  //runBlobs();
//  pixClear();
//  sparksRender();
//  pixUpdate();
  if(midiRead() == 0x90 && midiParams[1])
  {
    switch(midiParams[0])
    {
      case 60:
        digitsSetBoth(0xff,0);
        break;
      case 61:
        digitsSetBoth(0,0xff);
        break;
    }
  }
  gameRun(ticks);
//  stripRun(0);
}

