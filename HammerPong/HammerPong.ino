/*
  Don't rely on millis... gets slowed when interrupts are turned off during strip updates
*/
#include <Arduino.h>
#include "Lights.h"
#include "Digits.h"
#include "Strip.h"


// Pin for the hearbeat LED
#define P_HEARTBEAT 13



/*

/////////////////////////////////////////////////////////////////////
#define HALF_STRIP_JOIN 32 // represents a virtual "out of view" section at top of each strip
#define LEFT_STRIP_MIN (PIX_LENGTH-HALF_STRIP_JOIN)
#define LEFT_STRIP_MAX (-HALF_STRIP_JOIN)
#define RIGHT_STRIP_MAX HALF_STRIP_JOIN
#define RIGHT_STRIP_MIN (PIX_LENGTH+HALF_STRIP_JOIN)

typedef struct
{
  int x;         // which column (strip)
  float y;         // row position (0 is at the top of the strip)
  float intensity; // spark intensity, which ranges from bright white to dull red
  float dy;
} SPARK;

#define NUM_SPARKS 20
SPARK sparks[NUM_SPARKS];
void sparksInit()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {
    sparks[i].x=0;
    sparks[i].y=0;
    sparks[i].intensity=0;
    sparks[i].dy=0;
  }
}
void sparksRender()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {    
    if(sparks[i].intensity>5)
    {
      SPARK *pSpark = &sparks[i];
      byte *pCell = NULL;
      if(pSpark->x < 3)
      {
        // LEFT SIDE
        int y = pSpark->y - LEFT_STRIP_MIN;
        if(y >= 0 && y < PIX_LENGTH)
          pCell = &pixBuffer[pSpark->x][3*y];
      }
      else 
      {
        // RIGHT SIDE
        int y = pSpark->y - LEFT_STRIP_MIN;
        if(y >= 0 && y < PIX_LENGTH)
          pCell = &pixBuffer[pSpark->x-3][3*(PIX_LENGTH - y)];
      }
      if(pCell)
      {
#define THR        50
        pCell[1] = sparks[i].intensity;
        if(sparks[i].intensity > THR)
          pCell[0] = pCell[2] = THR + sparks[i].intensity/2;
      }
    }
  }
}
void sparksExplode()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {
    sparks[i].x=random(3);
    sparks[i].y=LEFT_STRIP_MIN;
    sparks[i].intensity=(float)random(150)+50;
    sparks[i].dy=(float)random(300)/100;
  }
  
}
void sparksRun()
{
  for(int i=0; i<NUM_SPARKS; ++i)
  {
    if(sparks[i].intensity)
    {
      sparks[i].y+=sparks[i].dy;
      sparks[i].dy*=0.93;
      sparks[i].intensity*=0.95;
    }
  }
  
}

void puckRender(int y)
{

  int q=y;
  byte *p;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &pixBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &pixBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
}
*/
void setup() {  
  pinMode(P_HEARTBEAT, OUTPUT);
  digitalWrite(P_HEARTBEAT, LOW);
  //sparksInit();
//  pixInit();
}
//int puckY=0;
//int q=0;
//byte c=0;
//unsigned long nextTick = 0;
void loop() 
{
  //unsigned long m = millis();
//  if(m > nextTick)
  //{
    //nextTick = m + 200;
//    digitalWrite(P_HEARTBEAT, c);
  //  c=!c;
    //sparksExplode();
//  }
  //sparksRun();
  
  //runBlobs();
//  pixClear();
//  sparksRender();
//  pixUpdate();
  
}

