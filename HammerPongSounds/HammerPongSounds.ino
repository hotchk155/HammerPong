#include "Arduino.h"
#define P_OUT 14
#define P_LED 11

enum 
{
  SOUND_NONE,
  SOUND_STRIKE
};
int whichSound = SOUND_NONE;
int count = 0;
unsigned long ms = 0;
unsigned long nextTick = 0;

void startSound(int which)
{
  whichSound = which;
  count = 0;
  nextTick = 0;
}

void runSounds(unsigned long ms)
{
  switch(whichSound)
  {
  case SOUND_NONE:    
    noTone(P_OUT);
    nextTick = ms + 1000;
    break;
  case SOUND_STRIKE:
      switch(count)
      {
        case 0:
          tone(P_OUT, 200);
          nextTick = ms + 100;
          break;        
        case 1:
          tone(P_OUT, 400);
          nextTick = ms + 100;
          break;        
        case 2:
          whichSound = SOUND_NONE;
          break;
      }      
  }
  count++;
}

void setup()
{
  Serial.begin(9600);
  pinMode(P_OUT,OUTPUT);
  pinMode(P_LED,OUTPUT);
  ms=0;
}

unsigned long q=0;
byte ledState=0;
void loop()
{
  unsigned long newMillis = millis();
  if(newMillis >= ms)
    ms = newMillis;
  else
    ms = 0;
    
  if(!ms || nextTick <= ms)
   runSounds(ms);

  
  if(q<ms)
  {
     digitalWrite(P_LED,ledState);
     q=ms+500;
     ledState=!ledState;
  }
  
  if(Serial.available())
  {
    char ch =Serial.read();
    switch(ch)
    {
      case 'a':
        startSound(SOUND_STRIKE);
        break;
    }
  }
}
