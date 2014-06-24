#include "Arduino.h"
#include "Game.h"
#include "Lights.h"
#include "Strip.h"

enum 
{
  GAME_STATE_SERVE,
  GAME_STATE_PLAY,
  GAME_STATE_SCORE,
  GAME_STATE_GAMEOVER,
  GAME_STATE_ATTRACT
};

int gameState;  // From GAME_STATE_xxx enum
int gameScoreLeft; // Left hand player
int gameScoreRight; // Right hand player
int gameServer; // Who serves (0 = Left, 1 = Right)
int gameLightStackLength;
unsigned long gameNextLightEvent;

void gameBeginState(int state)
{
  gameState = state;
  switch(state)
  {
  case GAME_STATE_SERVE:
    gameNextLightEvent = 0;
    gameLightStackLength = 0;
    break;
  case GAME_STATE_PLAY:
    break;
  case GAME_STATE_SCORE:
    break;
  case GAME_STATE_GAMEOVER:
    break;
  case GAME_STATE_ATTRACT:    
    break;
  }
}

void gameReset()
{
  
  gameScoreLeft = 0;
  gameScoreRight = 0;
  gameServer = random(2);
  gameBeginState(GAME_STATE_SERVE);
}


void gameRunLights(unsigned long ticks)
{
    switch(gameState)
    {
    case GAME_STATE_SERVE:
      if(gameLightStackLength < 12)
      {
        lightsSetStack(++gameLightStackLength);
        gameNextLightEvent = ticks + 500;
        gameState = GAME_STATE_PLAY;
      }
      break;
    case GAME_STATE_PLAY:
      if(gameLightStackLength > 0)
      {
        lightsSetStack(--gameLightStackLength);
        gameNextLightEvent = ticks + 100;
        gameState = GAME_STATE_SERVE;
      }
      break;
    case GAME_STATE_SCORE:
      break;
    case GAME_STATE_GAMEOVER:
      break;
    case GAME_STATE_ATTRACT:    
      break;
    }
}





void gameRunConsoleAttract(unsigned long ticks)
{
}
void gameRunConsole(unsigned long ticks)
{
  
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
    p = &stripBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &stripBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &stripBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &stripBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &stripBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &stripBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
}

void gameSetup()
{
  gameReset();
}
void gameRun(unsigned long ticks)
{
  if(!ticks || gameNextLightEvent < ticks)
    gameRunLights(ticks);
}
