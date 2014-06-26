#ifndef __LIGHTS_H__
#define __LIGHTS_H__

class CLights
{
  enum {
    P_DAT_2  = 12,
    P_SH     = 11,
    P_ST     = 10,
    P_OE     = 9,
    P_DAT_1  = 8
  };

  enum {
    L_V0 = 0x0002,
    L_V1 = 0x0004,
    L_V2 = 0x0008,
    L_V3 = 0x0010,
    L_V4 = 0x0020,
    L_V5 = 0x0040,
    L_V6 = 0x0080,
    
    L_H0 = 0x1000,
    L_H1 = 0x0800,
    L_H2 = 0x0400,
    L_H3 = 0x0200,
    L_H4 = 0x0100,
    
    R_V6 = 0x0100,
    R_V5 = 0x0200,
    R_V4 = 0x0400,
    R_V3 = 0x0800,
    R_V2 = 0x1000,
    R_V1 = 0x2000,
    R_V0 = 0x4000,
    
    R_H0 = 0x0008,
    R_H1 = 0x0010,
    R_H2 = 0x0020,
    R_H3 = 0x0040,
    R_H4 = 0x0080,
    
    R_BUTTON = 0x0004
  
  };

  // States of the left and right light arrays
  unsigned int stateLeft;
  unsigned int stateRight;
  
  ////////////////////////////////////////////////////////////////////////
  // Internal refresh function
  void refresh()
  {
    unsigned int mask = 0x8000;
    digitalWrite(P_ST, LOW);
    while(mask)
    {
      digitalWrite(P_SH, LOW);
      digitalWrite(P_DAT_1, (stateLeft & mask)?HIGH:LOW);
      digitalWrite(P_DAT_2, (stateRight & mask)?HIGH:LOW);    
      delay(1);
      digitalWrite(P_SH, HIGH);
      mask>>=1;
    }
    digitalWrite(P_ST, HIGH);
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Convert a 12 bit number into a pattern of lights
  // Bit 0 is the lowest of the vertical strip, up to bit 7 which is the 
  // leftmost of the horizontal strip, counting to right
  void setLeft(unsigned int d)
  {
    stateLeft = 0;
    unsigned int mask = 1;
    for(int i=0; i<12; ++i)
    {
      if(d&mask)
      {
        switch(i)
        {
          case 0:  stateLeft|=L_V0; break;
          case 1:  stateLeft|=L_V1; break;
          case 2:  stateLeft|=L_V2; break;
          case 3:  stateLeft|=L_V3; break;
          case 4:  stateLeft|=L_V4; break;
          case 5:  stateLeft|=L_V5; break;
          case 6:  stateLeft|=L_V6; break;
          case 7:  stateLeft|=L_H0; break;
          case 8:  stateLeft|=L_H1; break;
          case 9:  stateLeft|=L_H2; break;
          case 10: stateLeft|=L_H3; break;
          case 11: stateLeft|=L_H4; break;
        }
      }    
      mask <<=1;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Convert a 12 bit number into a pattern of lights
  // Bit 0 is the lowest of the vertical strip, up to bit 7 which is the 
  // rightmost of the horizontal strip, counting to left
  void setRight(unsigned int d)
  {
    stateRight &= R_BUTTON;
    unsigned int mask = 1;
    for(int i=0; i<12; ++i)
    {
      if(d&mask)
      {
        switch(i)
        {
          case 0:  stateRight|=R_V0; break;
          case 1:  stateRight|=R_V1; break;
          case 2:  stateRight|=R_V2; break;
          case 3:  stateRight|=R_V3; break;
          case 4:  stateRight|=R_V4; break;
          case 5:  stateRight|=R_V5; break;
          case 6:  stateRight|=R_V6; break;
          case 7:  stateRight|=R_H0; break;
          case 8:  stateRight|=R_H1; break;
          case 9:  stateRight|=R_H2; break;
          case 10: stateRight|=R_H3; break;
          case 11: stateRight|=R_H4; break;
        }
      }    
      mask <<=1;
    }
  }

public:  
  ////////////////////////////////////////////////////////////////////////
  // One off setup
  void setup()
  {
    pinMode(P_DAT_1, OUTPUT);
    pinMode(P_DAT_2, OUTPUT);
    pinMode(P_SH, OUTPUT);
    pinMode(P_ST, OUTPUT);
    pinMode(P_OE, OUTPUT);
    digitalWrite(P_OE, LOW);
    stateLeft = 0;
    stateRight = 0;
    refresh();
    setBrightness(255);
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Set same state in both left and right light arrays
  void setSymmetrical(unsigned int d)
  {
    setLeft(d);
    setRight(d);
    refresh();
  }
  
  void setBoth(unsigned int left, unsigned int right)
  {
    setLeft(left);
    setRight(right);
    refresh();
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Set state of the LED behind the start button
  void setButton(int d)
  {
    if(d)
      stateRight |= R_BUTTON;
    else
      stateRight &= ~R_BUTTON;
    refresh();
  }
  
  ////////////////////////////////////////////////////////////////////////
  void setBrightness(byte d)
  {
    analogWrite(P_OE, 255-d);
  }
  
  ////////////////////////////////////////////////////////////////////////
  void setStack(int len)
  {
    switch(len)
    {
      case 1:  setSymmetrical(0b000000000001); break;
      case 2:  setSymmetrical(0b000000000011); break;
      case 3:  setSymmetrical(0b000000000111); break;
      case 4:  setSymmetrical(0b000000001111); break;
      case 5:  setSymmetrical(0b000000011111); break;
      case 6:  setSymmetrical(0b000000111111); break;
      case 7:  setSymmetrical(0b000001111111); break;
      case 8:  setSymmetrical(0b000011111111); break;
      case 9:  setSymmetrical(0b000111111111); break;
      case 10: setSymmetrical(0b001111111111); break;
      case 11: setSymmetrical(0b011111111111); break;
      case 12: setSymmetrical(0b111111111111); break;
      default: setSymmetrical(0);     
    }    
  }
};  
  
extern CLights Lights;

#endif // __LIGHTS_H__
