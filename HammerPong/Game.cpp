#include "Arduino.h"
#include "Game.h"
#include "Strip.h"


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

void gameReset()
{
  puckY = 0;
}
void gameSetup()
{
  gameReset();
}
void gameRun(unsigned long milliseconds)
{
  stripClear();
  renderPuck(puckY);
  puckY++;
  if(puckY > 200)
    puckY = 0;
}
