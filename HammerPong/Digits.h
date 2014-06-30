#ifndef __DIGITS_H__
#define __DIGITS_H__

class CDigits
{
  ////////////////////////////////////////////////////////////////////////
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
  int counter;
  int sequenceType;
  int ticksPeriod;
  unsigned long nextTick;
  
public:
  ////////////////////////////////////////////////////////////////////////
  // Segment bits
  //
  //    AAA
  //   F   B
  //    GGG
  //   E   C
  //    DDD
  //
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
  // Character definitions
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

  ////////////////////////////////////////////////////////////////////////
  // Behaviour sequences
  enum
  {
      NO_SEQUENCE,
      TESTING,
      BLINK_LEFT,       
      BLINK_RIGHT,
      BLINK_LEFT_DIM_RIGHT,
      BLINK_RIGHT_DIM_LEFT,
      MEANDER
  };

public:
  
  ////////////////////////////////////////////////////////////////////////
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
  byte mapDigit(byte digit)
  {
    switch(digit)
    {
      case 0: return CH_0; 
      case 1: return CH_1; 
      case 2: return CH_2; 
      case 3: return CH_3; 
      case 4: return CH_4; 
      case 5: return CH_5; 
      case 6: return CH_6; 
      case 7: return CH_7; 
      case 8: return CH_8; 
      case 9: return CH_9; 
    }
    return 0;
  }
  
  ////////////////////////////////////////////////////////////////////////
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

    sequence(NO_SEQUENCE);  
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Show a 2 digit number
  void set(int n)
  {
    n%=100;
    stateLeft = mapDigit(n/10);
    stateRight = mapDigit(n%10);
    refresh();
    
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Show 2 single digit numbers
  void set(int left, int right)
  {
    stateLeft = mapDigit(left);
    stateRight = mapDigit(right);
    refresh();
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Set any combination of segments
  void setRaw(int left, int right)
  {
    stateLeft = left;
    stateRight = right;
    refresh();
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Set brightness of each display
  void setBrightness(byte left, byte right)
  {
    analogWrite(P_OE1, 255-left);
    analogWrite(P_OE2, 255-right);
  }

  ////////////////////////////////////////////////////////////////////////
  // Start a new sequence of lights
  void sequence(int s)
  {
    counter = 0;
    sequenceType = s;
    switch(sequenceType)
    {
      case NO_SEQUENCE:
        setBrightness(255,255);      
        setRaw(0,0);
        ticksPeriod = 1000;
        break;         
      case BLINK_LEFT:
      case BLINK_RIGHT:
      case BLINK_LEFT_DIM_RIGHT:
      case BLINK_RIGHT_DIM_LEFT:
        ticksPeriod = 10;
      break;
      
      case MEANDER:
        ticksPeriod = 100;
        break;        
      case TESTING:
        ticksPeriod = 10;
        break;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Run the sequence
  void run(unsigned long ticks)
  {
    if(!ticks || ticks >= nextTick)
    {
      byte br0;
      byte br1=255;
      switch(sequenceType)
      {
        case BLINK_LEFT_DIM_RIGHT:
        case BLINK_RIGHT_DIM_LEFT:
          br1 = 20;
        case BLINK_LEFT:
        case BLINK_RIGHT:
          br0 = 127+125.0*cos(counter/10.0);
          if(sequenceType == BLINK_LEFT || sequenceType == BLINK_LEFT_DIM_RIGHT)
            setBrightness(br0,br1);
          else
            setBrightness(br1,br0);
          break;
        
        case MEANDER:
          switch(counter%8)
          {  
           case 0: setRaw(SEG_A,SEG_F); break;
           case 1: setRaw(SEG_B,SEG_G); break;
           case 2: setRaw(SEG_G,SEG_C); break;
           case 3: setRaw(SEG_E,SEG_D); break;
           case 4: setRaw(SEG_D,SEG_E); break;
           case 5: setRaw(SEG_C,SEG_G); break;
           case 6: setRaw(SEG_G,SEG_B); break;
           case 7: setRaw(SEG_F,SEG_A); break;
          }  
          break;        
        case TESTING:
          set(counter/10);
          setBrightness(255-28*(counter%10),3+28*(counter%10));
          break;          
      }
      ++counter;
      nextTick = ticks + ticksPeriod;
    }  
  }
};

#endif //__DIGITS_H__
