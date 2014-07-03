/////////////////////////////////////////////////////////////////////
//
// CLASS TO CONTROL FRONT PANEL LIGHTS
//
/////////////////////////////////////////////////////////////////////
#ifndef __LIGHTS_H__
#define __LIGHTS_H__

class CLights
{
  // Pin definitions
  enum {
    P_DAT_2  = 12,
    P_SH     = 11,
    P_ST     = 10,
    P_OE     = 9,
    P_DAT_1  = 8
  };

  // Define the bits in the output data that correspond to each light
  enum {
    L_V0 = 0x0002,  // Left side vertical strip, numbered from bottom
    L_V1 = 0x0004,
    L_V2 = 0x0008,
    L_V3 = 0x0010,
    L_V4 = 0x0020,
    L_V5 = 0x0040,
    L_V6 = 0x0080,
    
    L_H0 = 0x1000,  // Left side horizontal strip, numbered from left to right
    L_H1 = 0x0800,
    L_H2 = 0x0400,
    L_H3 = 0x0200,
    L_H4 = 0x0100,
    
    R_V6 = 0x0100,  // Right side vertical strip, numbered from bottom
    R_V5 = 0x0200,
    R_V4 = 0x0400,
    R_V3 = 0x0800,
    R_V2 = 0x1000,
    R_V1 = 0x2000,
    R_V0 = 0x4000,
    
    R_H0 = 0x0008,  // Right side horizontal strip, numbered from right to left
    R_H1 = 0x0010,
    R_H2 = 0x0020,
    R_H3 = 0x0040,
    R_H4 = 0x0080,
    
    R_BUTTON = 0x0004  // Reset button (addressed via right hand strip)
  
  };

  // States of the left and right light arrays
  unsigned int stateLeft;
  unsigned int stateRight;
  
  // Running light sequence (if any) and additonal state variables for sequencing
  int sequenceType;
  unsigned long ticksPeriod;
  unsigned long nextTick;
  int counter;
  int fade;
  
  ////////////////////////////////////////////////////////////////////////
  // Send data to the lights
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
  // Set of defined light sequences
  enum 
  {
    NO_SEQUENCE,
    TESTING,
    SLOW_RISE_BOTH_SIDES,
    FAST_FALL_BOTH_SIDES,
    SCORE,
    VICTORY_LEFT,
    VICTORY_RIGHT,
    BLINK_RESET_BUTTON,
    MEANDER
  };

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
    sequence(NO_SEQUENCE);
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Set same state in both left and right light arrays
  void setSymmetrical(unsigned int d)
  {
    setLeft(d);
    setRight(d);
    refresh();
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Set left and right states individually
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
  // Set brightness of all the yellow lights (0-255)
  void setBrightness(byte d)
  {
    analogWrite(P_OE, 255-d);
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Set a "stack" of N lights starting from bottom of the vertical strip
  // and extending along the horizontal strip to the centre. The state of
  // left and right side are mirrored
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
  
  ////////////////////////////////////////////////////////////////////////
  // Start a new sequence of lights
  void sequence(int s)
  {
    sequenceType = s;
    nextTick=0;
    switch(sequenceType)
    {
      case NO_SEQUENCE:
        ticksPeriod = 1000;
        break;         
      case SLOW_RISE_BOTH_SIDES:
        counter = 0;
        setBrightness(200);      
        ticksPeriod = 100;
        break;
      case FAST_FALL_BOTH_SIDES:
        if(counter > 12 || counter < 0)
          counter = 12;
        setBrightness(200);      
        ticksPeriod = 35;
        break;
      case SCORE:
        counter = 0;
        fade=255;
        setBrightness(255);      
        ticksPeriod = 20;
        break;
      case VICTORY_LEFT:
      case VICTORY_RIGHT:
        counter = 0;
        setBrightness(200);
        ticksPeriod = 50;
        break;
      case BLINK_RESET_BUTTON:
        counter = 0;
        setBrightness(255);
        ticksPeriod = 50;
        break;
      case TESTING:
        counter = 0;
        setBrightness(255);
        ticksPeriod = 50;
        break;
      case MEANDER:
        counter = 0;
        setBrightness(100);
        ticksPeriod = 50;
        break;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////
  // Run the sequence
  void run(unsigned long ticks)
  {
    unsigned int b;
//    Serial.print(ticks, DEC);
//    Serial.print(" ");
//    Serial.print(nextTick, DEC);
//    Serial.println(" ");
    if(!ticks || nextTick <= ticks)
    {
//    Serial.println("lightsrun");
      switch(sequenceType)
      {
      case SLOW_RISE_BOTH_SIDES:
        if(counter++ < 12)
          setStack(counter);        
        else
          sequence(NO_SEQUENCE);
        break;
      case FAST_FALL_BOTH_SIDES:
        if(counter-- > 0)
          setStack(counter);        
        else
          sequence(NO_SEQUENCE);
        break;
      case SCORE:
        if(counter < 10)
        {
          if(counter&1)
            setStack(12);
          else
            setStack(0);          
          ++counter;
        }
        else if(fade > 1)
        {
            setStack(12);
            setBrightness(fade);        
            fade/=2.0;
        }
        else
        {
          setBrightness(0);        
          sequence(NO_SEQUENCE);
        }
        break;
      case VICTORY_LEFT:
      case VICTORY_RIGHT:
        switch(counter % 7)        
        {          
          case 0: b=0b000001000000; break;
          case 1: b=0b000000100000; break;
          case 2: b=0b000000010000; break;
          case 3: b=0b000000001000; break;
          case 4: b=0b000000000100; break;
          case 5: b=0b000000000010; break;
          case 6: b=0b000000000001; break;
        }
        switch(counter % 5)        
        {          
          case 0: b|=0b100000000000; break;
          case 1: b|=0b010000000000; break;
          case 2: b|=0b001000000000; break;
          case 3: b|=0b000100000000; break;
          case 4: b|=0b000010000000; break;
        }
        if(VICTORY_LEFT==sequenceType)
          setBoth(b,0);
        else
          setBoth(0,b);
        setButton(counter&0x8);
        counter++;
        break;
      case BLINK_RESET_BUTTON:
        setButton(counter&0x8);
        counter++;
        break;
      case TESTING:
        setLeft((unsigned int)1<<(11-counter%12));
        setRight((unsigned int)1<<(counter%12));
        setButton(counter&0x8);
        setBrightness(3+28*(counter%10));
        counter++;
        break;
      case MEANDER:
        if(counter<12)
        {
          setLeft(1<<(counter));
          setRight(0);
          refresh();
        }
        else
        if(counter<24)
        {
          setLeft(0);
          setRight(1<<(23-counter));
          refresh();
        }
        else
        if(counter<35)
        {
          setLeft(0);
          setRight(1<<(counter-23));
          refresh();
        }
        else
        if(counter<46)
        {
          setLeft(1<<(46-counter));
          setRight(0);
          refresh();
        }
        if(++counter >=46)
          counter=0;
        setButton(counter&0x4);
      }    
      nextTick = ticks + ticksPeriod;
    }
  }  
};  
  
#endif // __LIGHTS_H__
