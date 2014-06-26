#include "Arduino.h"
#include "Game.h"
#include "Lights.h"
#include "Strip.h"
#include "Digits.h"

enum 
{
  GAME_STATE_BEGIN,
  GAME_STATE_SERVE,
  GAME_STATE_PLAY,
  GAME_STATE_SCORE,
  GAME_STATE_GAMEOVER,
  GAME_STATE_ATTRACT
};

int gameState;  // From GAME_STATE_xxx enum
int gameScoreLeft; // Left hand player
int gameScoreRight; // Right hand player
int gameServer; // Who won last / serves next (0 = Left, 1 = Right)
int gameLightCounter;
float gameLightFade;
unsigned long gameNextLightEvent;

int gameDigitCounter;
unsigned long gameNextDigitEvent;
float gameDigitFade;

void gameBeginState(int state)
{
  gameState = state;
  gameNextLightEvent = 0;
  gameNextDigitEvent = 0;
  switch(state)
  {
  case GAME_STATE_BEGIN:
    gameScoreLeft = 0;
    gameScoreRight = 0;
    gameServer = random(2);
    gameBeginState(GAME_STATE_SERVE);
    break;
    
  case GAME_STATE_SERVE:
    Lights.setBrightness(200);        
    gameLightCounter = 0;
    Digits.setBoth(gameScoreLeft, gameScoreRight);
    gameDigitFade = 0;
    break;
  case GAME_STATE_PLAY:
    break;
  case GAME_STATE_SCORE:
    gameLightCounter = 0;
    gameLightFade=255;
    break;
  case GAME_STATE_GAMEOVER:
    Lights.setBrightness(200);        
    gameLightCounter = 0;
    Digits.setBoth(gameScoreLeft, gameScoreRight);
    gameDigitFade = 0;
    break;
  case GAME_STATE_ATTRACT:    
    Digits.setBrightness(100,100);
    Lights.setStack(0);
    gameDigitCounter = 0;
    gameLightCounter = 0;
    break;
  }
}

void gameRunDigits(unsigned long ticks)
{
    float f;
    int d = 200;
    switch(gameState)
    {
    case GAME_STATE_GAMEOVER:
      d=20; // dim the losing player
    case GAME_STATE_SERVE:
       f = 127+125*cos(gameDigitFade);
      if(gameServer==0)
        Digits.setBrightness(f,d);
      else
        Digits.setBrightness(d,f);
      gameDigitFade += 0.1;
      gameNextDigitEvent = ticks +10;
      break;
      
    case GAME_STATE_ATTRACT:
      switch(gameDigitCounter%8)
      {
        case 0: Digits.setRaw(Digits.SEG_A,Digits.SEG_F); break;
        case 1: Digits.setRaw(Digits.SEG_B,Digits.SEG_G); break;
        case 2: Digits.setRaw(Digits.SEG_G,Digits.SEG_C); break;
        case 3: Digits.setRaw(Digits.SEG_E,Digits.SEG_D); break;
        case 4: Digits.setRaw(Digits.SEG_D,Digits.SEG_E); break;
        case 5: Digits.setRaw(Digits.SEG_C,Digits.SEG_G); break;
        case 6: Digits.setRaw(Digits.SEG_G,Digits.SEG_B); break;
        case 7: Digits.setRaw(Digits.SEG_F,Digits.SEG_A); break;
      }
      gameNextDigitEvent = ticks +100;
      gameDigitCounter++;
      break;
    }
}
void gameRunLights(unsigned long ticks)
{
    switch(gameState)
    {
    case GAME_STATE_SERVE:
      if(gameLightCounter < 12)
      {
        Lights.setStack(++gameLightCounter);        
        gameNextLightEvent = ticks +100;
      }
      else
      {
    gameBeginState(GAME_STATE_ATTRACT);
        
      }
      break;
    case GAME_STATE_PLAY:
      if(gameLightCounter > 0)
      {
        Lights.setStack(--gameLightCounter);
        gameNextLightEvent = ticks + 20;
      }
      break;
    case GAME_STATE_SCORE:
      if(gameLightCounter < 10)
      {
        if(gameLightCounter&1)
          Lights.setStack(12);
        else
          Lights.setStack(0);          
        gameNextLightEvent = ticks + 20;
      }
      else if(gameLightCounter < 100)
      {
          Lights.setStack(12);
          Lights.setBrightness(gameLightFade);        
          gameLightFade/=2.0;
          gameNextLightEvent = ticks + 50;
      }
      else
      {
          Lights.setStack(0);
      }
      ++gameLightCounter;
      break;
    case GAME_STATE_GAMEOVER:
      {
        unsigned int b= 0;
        Lights.setBrightness(200);
        switch(gameLightCounter % 7)        
        {          
          case 0: b=0b000001000000; break;
          case 1: b=0b000000100000; break;
          case 2: b=0b000000010000; break;
          case 3: b=0b000000001000; break;
          case 4: b=0b000000000100; break;
          case 5: b=0b000000000010; break;
          case 6: b=0b000000000001; break;
        }
        switch(gameLightCounter % 5)        
        {          
          case 0: b|=0b100000000000; break;
          case 1: b|=0b010000000000; break;
          case 2: b|=0b001000000000; break;
          case 3: b|=0b000100000000; break;
          case 4: b|=0b000010000000; break;
        }
        if(0==gameServer)
          Lights.setBoth(b,0);
        else
          Lights.setBoth(0,b);
        gameLightCounter++;
        Lights.setButton(gameLightCounter&0x8);
      }      
      gameNextLightEvent = ticks + 50;
      break;
      
      
    ////////////////////////////////////////  
    case GAME_STATE_ATTRACT:    
      Lights.setButton(gameLightCounter&0x8);
      gameLightCounter++;
      gameNextLightEvent = ticks + 50;
      break;
    }
}



typedef struct 
{
  byte x;
  int y;
  
} SPARK;




int puckY;


///////////////////////////////////////////////////////////////////////////
void renderPuck(int y)
{

  int q=y;
  byte *p;
  if(q>=0 && q<150)
  {
    p = &Strip.buffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &Strip.buffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &Strip.buffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &Strip.buffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &Strip.buffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &Strip.buffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
}

void gameSetup()
{
  gameBeginState(GAME_STATE_BEGIN);
}
void gameRun(unsigned long ticks)
{
  if(gameNextLightEvent!=NO_TICKS && (!ticks || gameNextLightEvent < ticks))
  {
    gameNextLightEvent = NO_TICKS;
    gameRunLights(ticks);
  }
  if(gameNextDigitEvent!=NO_TICKS && (!ticks || gameNextDigitEvent < ticks))
  {
    gameNextDigitEvent = NO_TICKS;
    gameRunDigits(ticks);
  }
}
