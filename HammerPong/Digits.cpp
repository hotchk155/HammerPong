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
byte digitsStateLeft;
byte digitsStateRight;

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
// Show a single digit number on the left or the right display
void digitsSet(byte pos, int n)
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
}

////////////////////////////////////////////////////////////////////////
// One-off setup
void digitsSetup()
{
  pinMode(P_DIGITS_DAT1, OUTPUT);
  pinMode(P_DIGITS_DAT2, OUTPUT);
  pinMode(P_DIGITS_SH, OUTPUT);
  pinMode(P_DIGITS_ST, OUTPUT);
  
  digitalWrite(P_DIGITS_DAT1, LOW);
  digitalWrite(P_DIGITS_DAT2, LOW);
  digitalWrite(P_DIGITS_SH, LOW);
  digitalWrite(P_DIGITS_ST, LOW);

  digitsStateLeft = 0;
  digitsStateRight = 0;
  digitsRefresh();
}

////////////////////////////////////////////////////////////////////////
// Show a 2 digit number
void digitsSetCounter(int n)
{
  n%=100;
  digitsSetLeft(n/10);
  digitsSetRight(n%10);
}
void digitsSetLeft(int left)
{
  digitsSet(0,left);
  digitsRefresh();
}
void digitsSetRight(int right)
{
  digitsSet(1,right);
  digitsRefresh();
}
void digitsSetBoth(int left, int right)
{
  digitsSet(0,left);
  digitsSet(1,right);
  digitsRefresh();
}
void digitsSetRaw(int left, int right)
{
  digitsStateLeft = left;
  digitsStateRight = right;
  digitsRefresh();
}
void digitsSetBrightness(byte left, byte right)
{
  analogWrite(P_DIGITS_OE1, 255-left);
  analogWrite(P_DIGITS_OE2, 255-right);
}
