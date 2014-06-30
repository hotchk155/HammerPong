/////////////////////////////////////////////////////////////////////
// Manages a "trail of sparks" particle system
#ifndef __SPARKS_H__
#define __SPARKS_H__

#define MAX_SPARKS 100

class CSparks
{
  typedef struct
  {
    byte red;
    byte green;
    byte blue;
    byte col;    
    float row;
    float vel;
    float intensity;
    byte ired;
    byte igreen;
    byte iblue;
  } SPARK;

  enum 
  {
    TICK_PERIOD = 10
  };  
  SPARK sparks[MAX_SPARKS];
  unsigned long nextTicks;
  int count;
public:
  void setup()
  {
    count = 0;
    nextTicks = 0;
  }

  void add(byte col, int row, float vel, byte red, byte green, byte blue, int intensity) // velocity is per tick
  {
    for(int i=0; i<count; ++i)
    {
      SPARK *p = &sparks[i];
      if(!((int)p->intensity))
      {
        p->ired = 0;
        p->igreen = 0;
        p->iblue = 0;
        p->row = row;
        p->col = col;
        p->vel = vel;
        p->red = red;
        p->green = green;
        p->blue = blue;
        p->intensity = intensity;
        break;
      }
    }    
  }
  void run(unsigned long ticks)
  {
    if(!ticks || nextTicks < ticks)
    {
      nextTicks = ticks + TICK_PERIOD;
      for(int i=0; i<MAX_SPARKS; ++i)
      {
        SPARK *s = &sparks[i];
        if((int)s->intensity)
        {
          s->ired = (float)s->red*s->intensity/255.0;
          s->igreen = (float)s->green*s->intensity/255.0;
          s->iblue = (float)s->blue*s->intensity/255.0;
          s->row -= s->vel;
          s->vel += 0.009;
          s->intensity *= 0.95;          
        }
      }          
    }
  }
  void render(CStrip& Strip)
  {
    for(int i=0; i<MAX_SPARKS; ++i)
    {
      SPARK *s = &sparks[i];
      int row = s->row;
      if((int)s->intensity && row < 150 && row > 0)
      {
        byte *p = &Strip.buffer[s->col][3*row];
        p[0]=s->igreen;
        p[1]=s->ired;
        p[2]=s->iblue;
      }
    }
  }              
  void setCount(int newCount)
  {
    if(newCount > MAX_SPARKS)
      newCount = MAX_SPARKS;
    if(newCount < 0)
      newCount = 0;
    for(int i=count; i<newCount; ++i)
      sparks[i].intensity = 0;
    count = newCount;    
  }
};

#endif //  __SPARKS_H__
