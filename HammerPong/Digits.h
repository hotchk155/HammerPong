#ifndef __DIGITS_H__
#define __DIGITS_H__

class CDigits
{
  enum {
    P_DAT1  = 2,
    P_DAT2  = 6,
    P_OE1   = 3,
    P_OE2   = 7,
    P_SH    = 5,
    P_ST    = 4
  };
  
  // The content of the display
  byte stateLeft;
  byte stateRight;

public:
  ////////////////////////////////////////////////////////////////////////
  enum {
    SEG_A = 0x01,
    SEG_B = 0x02,
    SEG_C = 0x04,
    SEG_D = 0x08,
    SEG_E = 0x10,
    SEG_F = 0x20,
    SEG_G = 0x40
  };
  
  ////////////////////////////////////////////////////////////////////////
  //
  //    AAA
  //   F   B
  //    GGG
  //   E   C
  //    DDD
  //
  enum {
    CH_0 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F,
    CH_1 = SEG_B|SEG_C,
    CH_2 = SEG_A|SEG_B|SEG_D|SEG_E|SEG_G,
    CH_3 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_G,
    CH_4 = SEG_B|SEG_C|SEG_F|SEG_G,
    CH_5 = SEG_A|SEG_C|SEG_D|SEG_F|SEG_G,
    CH_6 = SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
    CH_7 = SEG_A|SEG_B|SEG_C,
    CH_8 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
    CH_9 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G
  };

public:
  ////////////////////////////////////////////////////////////////////////
  // Implementation function to refresh the displays
  void refresh()
  {
    byte mask = 0x80;
    digitalWrite(P_ST, LOW);
    while(mask)
    {
      digitalWrite(P_SH, LOW);
      digitalWrite(P_DAT1, (stateLeft & mask)?HIGH:LOW);
      digitalWrite(P_DAT2, (stateRight & mask)?HIGH:LOW);    
      delay(1);
      digitalWrite(P_SH, HIGH);
      mask>>=1;
    }
    digitalWrite(P_ST, HIGH);
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Show a single digit number on the left or the right display
  void set(byte pos, int n)
  {
    byte &buf=pos? stateRight : stateLeft;
    switch(n)
    {
      case 0: buf=CH_0; break;
      case 1: buf=CH_1; break;
      case 2: buf=CH_2; break;
      case 3: buf=CH_3; break;
      case 4: buf=CH_4; break;
      case 5: buf=CH_5; break;
      case 6: buf=CH_6; break;
      case 7: buf=CH_7; break;
      case 8: buf=CH_8; break;
      case 9: buf=CH_9; break;
      default: buf=0; break;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////
  // One-off setup
  void setup()
  {
    pinMode(P_DAT1, OUTPUT);
    pinMode(P_DAT2, OUTPUT);
    pinMode(P_SH, OUTPUT);
    pinMode(P_ST, OUTPUT);
    
    digitalWrite(P_DAT1, LOW);
    digitalWrite(P_DAT2, LOW);
    digitalWrite(P_SH, LOW);
    digitalWrite(P_ST, LOW);
  
    stateLeft = 0;
    stateRight = 0;
    refresh();
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Show a 2 digit number
  void setCounter(int n)
  {
    n%=100;
    setLeft(n/10);
    setRight(n%10);
  }
  void setLeft(int left)
  {
    set(0,left);
    refresh();
  }
  void setRight(int right)
  {
    set(1,right);
    refresh();
  }
  void setBoth(int left, int right)
  {
    set(0,left);
    set(1,right);
    refresh();
  }
  void setRaw(int left, int right)
  {
    stateLeft = left;
    stateRight = right;
    refresh();
  }
  void setBrightness(byte left, byte right)
  {
    analogWrite(P_OE1, 255-left);
    analogWrite(P_OE2, 255-right);
  }
};

extern CDigits Digits;

#endif //__DIGITS_H__
