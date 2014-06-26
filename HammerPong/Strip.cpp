#include "Arduino.h"
#include "Strip.h"

// pins
/*
#define P_STRIP0 5
#define P_STRIP1 6
#define P_STRIP2 7
#define P_STRIP3 36
#define P_STRIP4 37
#define P_STRIP5 38
*/

#define P_STRIP0 51
#define P_STRIP1 49
#define P_STRIP2 47
#define P_STRIP3 45
#define P_STRIP4 41
#define P_STRIP5 39


// Timing constants
#define SCALE      VARIANT_MCK / 2UL / 1000000UL
#define INST       (2UL * F_CPU / VARIANT_MCK)
#define TICKS_PHASE_0 ((int)(0.40 * SCALE + 0.5) - (5 * INST))
#define TICKS_PHASE_1 ((int)(0.80 * SCALE + 0.5) - (5 * INST))
#define TICKS_PHASE_2 ((int)(1.25 * SCALE + 0.5) - (5 * INST))

byte stripBuffer[STRIP_NUMBUFFERS][STRIP_BUFSSIZE]; // The huge display buffer!
uint32_t endTime;  // Instant in time the strips were last refreshed, used to enfore minimum inter-refresh delay
byte stripNeedsUpdate;

/////////////////////////////////////////////////////////////////////
// REFRESH ALL 6 LED STRIPS
// Based on Adafruit Neopixel code
// Modified to drive six strips in parallel
void stripRefresh()
{
  // Ensure that the minimum "latch" command time has elapsed since the 
  // last refresh
  while ((micros() - endTime) < 50L);
  
  // turn off interrupts
  noInterrupts(); 
  
  // Get the timer ready
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk((uint32_t)TC3_IRQn);
  TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP | TC_CMR_TCCLKS_TIMER_CLOCK1);
  TC_Start(TC1, 0);
  
  // get port address... Note: all the pins need to be on same port reg
  Pio *port      = g_APinDescription[P_STRIP0].pPort; 
  
  // Get register addresses
  volatile WoReg *portSet   = &(port->PIO_SODR);            
  volatile WoReg *portClear = &(port->PIO_CODR);            
  
  // Get timer
  volatile WoReg *timeValue = &(TC1->TC_CHANNEL[0].TC_CV);  
  volatile WoReg *timeReset = &(TC1->TC_CHANNEL[0].TC_CCR);

  // Get the pin mask for each pin driving a strip
  int pinMask0 = g_APinDescription[P_STRIP0].ulPin; 
  int pinMask1 = g_APinDescription[P_STRIP1].ulPin; 
  int pinMask2 = g_APinDescription[P_STRIP2].ulPin; 
  int pinMask3 = g_APinDescription[P_STRIP3].ulPin; 
  int pinMask4 = g_APinDescription[P_STRIP4].ulPin; 
  int pinMask5 = g_APinDescription[P_STRIP5].ulPin; 

  // The strip is driven in 3 phases... 
  // Phase 0: All the output pins are HIGH
  // Phase 1: All those output pins transmitting a "zero" are set low
  // Phase 2: All the remaining pins are set low
  // 
  // Set up the bit mask for phase 0...
  int phase0 = pinMask0|pinMask1|pinMask2|pinMask3|pinMask4|pinMask5;
  
  // Loop through all the bytes per strip
  for(int index = 0; index < STRIP_BUFSSIZE; ++index)  
  {          
    // Fetch all six colour byte values
    byte colourValue0 = stripBuffer[0][index];
    byte colourValue1 = stripBuffer[1][index];
    byte colourValue2 = stripBuffer[2][index];
    byte colourValue3 = stripBuffer[3][index];
    byte colourValue4 = stripBuffer[4][index];
    byte colourValue5 = stripBuffer[5][index];

    // Loop through the bits per byte
    uint8_t bitMask = 0x80;
    while(bitMask)
    {    
      // Calculate the phase 1 value (bit mask of all 
      // those bits corresponding to the output pins
      // for strips to which we're sending a "zero" bit
      int phase1 = phase0; 
      if(colourValue0 & bitMask) phase1 ^= pinMask0; 
      if(colourValue1 & bitMask) phase1 ^= pinMask1; 
      if(colourValue2 & bitMask) phase1 ^= pinMask2; 
      if(colourValue3 & bitMask) phase1 ^= pinMask3; 
      if(colourValue4 & bitMask) phase1 ^= pinMask4; 
      if(colourValue5 & bitMask) phase1 ^= pinMask5; 
      
      // Calculate phase 2 value (bit mask of all those
      // output pins that were not cleared at phase 1)
      int phase2 = phase0 ^ phase1; 
      
      // Wait until we're ready to start the next bit
      while (*timeValue < TICKS_PHASE_2);    
      
      // Phase 0: All outputs high, reset the timer
      // and wait for the phase 0 duration
      *portSet   = phase0;
      *timeReset = TC_CCR_CLKEN | TC_CCR_SWTRG;      
      while (*timeValue < TICKS_PHASE_0);
      
      // Phase 1: Clear outputs for zero data bits
      // and wait until end of phase 1
      *portClear = phase1;
      while (*timeValue < TICKS_PHASE_1);
      
      // Phase 2: Clear remaining outputs
      *portClear = phase2;
  
      // shift the bit mask
      bitMask >>= 1;
    }
  }
  
  // Wait for final bit to complete
  while (*timeValue < TICKS_PHASE_2);    
  
  // Stop the timer
  TC_Stop(TC1, 0);

  // Interrupts, you can come back...
  interrupts();
  
  // Remember the current time so we can enforce minimum 
  // latch period between refreshes
  endTime = micros(); 
}

/////////////////////////////////////////////////////////////////////
// ONE OFF INITIALISATION OF THE DISPLAY DRIVE CODE
void stripSetup()
{
  // Set up pins
  pinMode(P_STRIP0, OUTPUT);
  pinMode(P_STRIP1, OUTPUT);
  pinMode(P_STRIP2, OUTPUT);
  pinMode(P_STRIP3, OUTPUT);
  pinMode(P_STRIP4, OUTPUT);
  pinMode(P_STRIP5, OUTPUT);

  // Set default output value
  digitalWrite(P_STRIP0, LOW);
  digitalWrite(P_STRIP1, LOW);
  digitalWrite(P_STRIP2, LOW);
  digitalWrite(P_STRIP3, LOW);
  digitalWrite(P_STRIP4, LOW);
  digitalWrite(P_STRIP5, LOW);

  
  // Initialise the "last update time"
  endTime = 0;
  stripClear();
  stripRefresh();
  stripNeedsUpdate = 0;
}

/////////////////////////////////////////////////////////////////////
void stripClear()
{
  memset(stripBuffer, 0, STRIP_NUMBUFFERS * STRIP_BUFSSIZE);
  stripNeedsUpdate = 1;
}


/////////////////////////////////////////////////////////////////////
void stripRun(unsigned long milliseconds)
{
  if(stripNeedsUpdate)
  {
    stripRefresh();
    stripNeedsUpdate = 0;
  }
}

/////////////////////////////////////////////////////////////////////
void stripInvalidate()
{
  stripNeedsUpdate = 1;
}

