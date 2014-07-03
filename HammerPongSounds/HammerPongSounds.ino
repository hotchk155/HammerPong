#include "Arduino.h"

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#define P_OUT 14
#define P_LED 11

enum 
{
  SOUND_NONE,
  SOUND_BEGIN,
  SOUND_SERVE,
  SOUND_RETURN,
  SOUND_SCORE,
  SOUND_TUNE
};
int whichSound = SOUND_NONE;
int count = 0;
unsigned long ms = 0;
unsigned long nextTick = 0;

char * tune = 
//1---2---3---4---
"c.e.gag...g-...."
"a.agage...e-....";

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
  case SOUND_BEGIN:    
      if(count<30)
      {
          tone(P_OUT, 200+10*count);
          nextTick = ms + 25;
      }
      else
      {      
          whichSound = SOUND_NONE;
      }      
      break;
  case SOUND_SERVE:
      switch(count)
      {
        case 0:
          tone(P_OUT, 400);
          nextTick = ms + 50;
          break;        
        case 1:
          whichSound = SOUND_NONE;
          break;
      }      
      break;
  case SOUND_RETURN:
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
      break; 
  case SOUND_SCORE:
      switch(count)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
          tone(P_OUT, 400-50*count);
          nextTick = ms + 100;
          break;        
        case 6:
          whichSound = SOUND_NONE;
          break;
      }      
      break;
  case SOUND_TUNE:
      switch(tune[count])
      {
         case 'c': tone(P_OUT, NOTE_C4); break;
         case 'e': tone(P_OUT, NOTE_E4); break;
         case 'g': tone(P_OUT, NOTE_G4); break;
         case 'a': tone(P_OUT, NOTE_A4); break;
         case '.': noTone(P_OUT); break;
         case '-': break;
         case '\0': whichSound = SOUND_NONE; break;
      }  
      nextTick = ms + 100;
      break;
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
      case 'n':
        startSound(SOUND_NONE);
        break;
      case 's':
        startSound(SOUND_SERVE);
        break;
      case 'b':
        startSound(SOUND_BEGIN);
        break;
      case 'r':
        startSound(SOUND_RETURN);
        break;
      case 'w':
        startSound(SOUND_SCORE);
        break;
      case 't':
        startSound(SOUND_TUNE);
        break;
    }
  }
}

