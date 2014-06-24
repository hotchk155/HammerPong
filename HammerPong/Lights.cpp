////////////////////////////////////////////////////////////////////////
//
// LIGHTS
//
// Code for controlling the yellow lights
//
////////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "Lights.h"

// States of the left and right light arrays
unsigned int lightsStateLeft = 0;
unsigned int lightsStateRight = 0;

////////////////////////////////////////////////////////////////////////
// Internal refresh function
void lightsRefresh()
{
  unsigned int mask = 0x8000;
  digitalWrite(P_LIGHTS_ST, LOW);
  while(mask)
  {
    digitalWrite(P_LIGHTS_SH, LOW);
    digitalWrite(P_LIGHTS_DAT_1, (lightsStateLeft & mask)?HIGH:LOW);
    digitalWrite(P_LIGHTS_DAT_2, (lightsStateRight & mask)?HIGH:LOW);    
    delay(1);
    digitalWrite(P_LIGHTS_SH, HIGH);
    mask>>=1;
  }
  digitalWrite(P_LIGHTS_ST, HIGH);
}

////////////////////////////////////////////////////////////////////////
// Convert a 12 bit number into a pattern of lights
// Bit 0 is the lowest of the vertical strip, up to bit 7 which is the 
// leftmost of the horizontal strip, counting to right
void lightsSetLeft(unsigned int d)
{
  lightsStateLeft = 0;
  unsigned int mask = 1;
  for(int i=0; i<12; ++i)
  {
    if(d&mask)
    {
      switch(i)
      {
        case 0:  lightsStateLeft|=LIGHTS_L_V0; break;
        case 1:  lightsStateLeft|=LIGHTS_L_V1; break;
        case 2:  lightsStateLeft|=LIGHTS_L_V2; break;
        case 3:  lightsStateLeft|=LIGHTS_L_V3; break;
        case 4:  lightsStateLeft|=LIGHTS_L_V4; break;
        case 5:  lightsStateLeft|=LIGHTS_L_V5; break;
        case 6:  lightsStateLeft|=LIGHTS_L_V6; break;
        case 7:  lightsStateLeft|=LIGHTS_L_H0; break;
        case 8:  lightsStateLeft|=LIGHTS_L_H1; break;
        case 9:  lightsStateLeft|=LIGHTS_L_H2; break;
        case 10: lightsStateLeft|=LIGHTS_L_H3; break;
        case 11: lightsStateLeft|=LIGHTS_L_H4; break;
      }
    }    
    mask <<=1;
  }
}

////////////////////////////////////////////////////////////////////////
// Convert a 12 bit number into a pattern of lights
// Bit 0 is the lowest of the vertical strip, up to bit 7 which is the 
// rightmost of the horizontal strip, counting to left
void lightsSetRight(unsigned int d)
{
  lightsStateRight &= 0b111111111111;
  unsigned int mask = 1;
  for(int i=0; i<12; ++i)
  {
    if(d&mask)
    {
      switch(i)
      {
        case 0:  lightsStateRight|=LIGHTS_R_V0; break;
        case 1:  lightsStateRight|=LIGHTS_R_V1; break;
        case 2:  lightsStateRight|=LIGHTS_R_V2; break;
        case 3:  lightsStateRight|=LIGHTS_R_V3; break;
        case 4:  lightsStateRight|=LIGHTS_R_V4; break;
        case 5:  lightsStateRight|=LIGHTS_R_V5; break;
        case 6:  lightsStateRight|=LIGHTS_R_V6; break;
        case 7:  lightsStateRight|=LIGHTS_R_H0; break;
        case 8:  lightsStateRight|=LIGHTS_R_H1; break;
        case 9:  lightsStateRight|=LIGHTS_R_H2; break;
        case 10: lightsStateRight|=LIGHTS_R_H3; break;
        case 11: lightsStateRight|=LIGHTS_R_H4; break;
      }
    }    
    mask <<=1;
  }
}

////////////////////////////////////////////////////////////////////////
// One off setup
void lightsSetup()
{
  pinMode(P_LIGHTS_DAT_1, OUTPUT);
  pinMode(P_LIGHTS_DAT_2, OUTPUT);
  pinMode(P_LIGHTS_SH, OUTPUT);
  pinMode(P_LIGHTS_ST, OUTPUT);
  pinMode(P_LIGHTS_OE, OUTPUT);
  digitalWrite(P_LIGHTS_OE, LOW);
}

////////////////////////////////////////////////////////////////////////
// Set same state in both left and right light arrays
void lightsSetSymmetrical(unsigned int d)
{
  lightsSetLeft(d);
  lightsSetRight(d);
  lightsRefresh();
}

////////////////////////////////////////////////////////////////////////
// Set state of the LED behind the start button
void lightsSetButton(byte d)
{
  if(d)
    lightsStateRight |= LIGHTS_R_BUTTON;
  else
    lightsStateRight &= ~LIGHTS_R_BUTTON;
}

////////////////////////////////////////////////////////////////////////
void lightsSetBrightness(byte d)
{
  analogWrite(P_LIGHTS_OE, 255-d);
}

////////////////////////////////////////////////////////////////////////
void lightsSetStack(int len)
{
  switch(len)
  {
    case 1:  lightsSetSymmetrical(0b000000000001); break;
    case 2:  lightsSetSymmetrical(0b000000000011); break;
    case 3:  lightsSetSymmetrical(0b000000000111); break;
    case 4:  lightsSetSymmetrical(0b000000001111); break;
    case 5:  lightsSetSymmetrical(0b000000011111); break;
    case 6:  lightsSetSymmetrical(0b000000111111); break;
    case 7:  lightsSetSymmetrical(0b000001111111); break;
    case 8:  lightsSetSymmetrical(0b000011111111); break;
    case 9:  lightsSetSymmetrical(0b000111111111); break;
    case 10: lightsSetSymmetrical(0b001111111111); break;
    case 11: lightsSetSymmetrical(0b011111111111); break;
    case 12: lightsSetSymmetrical(0b111111111111); break;
    default: lightsSetSymmetrical(0);     
  }
}
