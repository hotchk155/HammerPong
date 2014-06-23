////////////////////////////////////////////////////////////////////////
//
// DIGITS
//
// Code for controlling the "7 segment displays"
//
////////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "digits.h"

// The content of the display
byte digitsStateLeft = 0;
byte digitsStateRight = 0;

// dirty flag says if the states have changed
byte digitsNeedUpdate = 1;

// timer for next event for display animations
unsigned int digitsNextEvent = 0;

////////////////////////////////////////////////////////////////////////
// Implementation function to refresh the displays
void digitsRefresh()
{
  byte mask = 0x80;
  digitalWrite(P_DIGITS_ST, LOW);
  while(mask)
  {
    digitalWrite(P_DIGITS_SH, LOW);
    digitalWrite(P_DIGITS_DAT1, (digitsStateLeft & mask)?HIGH:LOW);
    digitalWrite(P_DIGITS_DAT2, (digitsStateRight & mask)?HIGH:LOW);    
    delay(1);
    digitalWrite(P_DIGITS_SH, HIGH);
    mask>>=1;
  }
  digitalWrite(P_DIGITS_ST, HIGH);
}

////////////////////////////////////////////////////////////////////////
// One-off setup
void digitsSetup()
{
  pinMode(P_DIGITS_DAT1, OUTPUT);
  pinMode(P_DIGITS_DAT2, OUTPUT);
  pinMode(P_DIGITS_OE1, OUTPUT);  
  pinMode(P_DIGITS_OE2, OUTPUT);
  pinMode(P_DIGITS_SH, OUTPUT);
  pinMode(P_DIGITS_ST, OUTPUT);
  
  digitalWrite(P_DIGITS_DAT1, LOW);
  digitalWrite(P_DIGITS_DAT2, LOW);
  digitalWrite(P_DIGITS_SH, LOW);
  digitalWrite(P_DIGITS_ST, LOW);
  digitalWrite(P_DIGITS_OE1, LOW);
  digitalWrite(P_DIGITS_OE2, LOW);
}

////////////////////////////////////////////////////////////////////////
// Directly set the content of left and right displays
void digitsSet(int left, int right)
{
  if(left>=0)
    digitsStateLeft = left;
  if(right>=0)
    digitsStateRight = right;
  digitsNeedUpdate = 1;
}


////////////////////////////////////////////////////////////////////////
// Show a single digit number on the left or the right display
void digitsShowNumber(byte pos, int n)
{
  byte &buf=pos? digitsStateRight : digitsStateLeft;
  switch(n)
  {
    case 0: buf=DIGIT_0; break;
    case 1: buf=DIGIT_1; break;
    case 2: buf=DIGIT_2; break;
    case 3: buf=DIGIT_3; break;
    case 4: buf=DIGIT_4; break;
    case 5: buf=DIGIT_5; break;
    case 6: buf=DIGIT_6; break;
    case 7: buf=DIGIT_7; break;
    case 8: buf=DIGIT_8; break;
    case 9: buf=DIGIT_9; break;
    default: buf=0; break;
  }
  digitsNeedUpdate=1;
}

////////////////////////////////////////////////////////////////////////
// Show a 2 digit number
void digitsShowNumber(int n)
{
  n%=100;
  digitsShowNumber(0, n/10);
  digitsShowNumber(1, n%10);
}

////////////////////////////////////////////////////////////////////////
// Run the digits state machine
void digitsRun(unsigned long milliseconds)
{
  if(digitsNeedUpdate)
  {
    digitsRefresh();
    digitsNeedUpdate=0;
  }
}


