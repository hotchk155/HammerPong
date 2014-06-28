#ifndef __TRAILS_H__
#define __TRAILS_H__

#define MAX_SPARKS 50

class CTrail
{
  typedef struct
  {
    byte col;
    float row;
    float vel;
    float intensity;
  } SPARK;

  enum 
  {
    TICK_PERIOD = 10
  };  
  SPARK sparks[MAX_SPARKS];
  unsigned long nextTicks;
public:
  void setup()
  {
    nextTicks = 0;
    for(int i=0; i<MAX_SPARKS; ++i)
      sparks[i].intensity = 0;
  }

  void add(byte c, int r, float v) // velocity is per tick
  {
    for(int i=0; i<MAX_SPARKS; ++i)
    {
      SPARK *p = &sparks[i];
      if(!((int)p->intensity))
      {
        p->row = r;
        p->col = c;
        p->vel = -v/(2+random(5));;
        p->intensity = random(100);
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
          s->row -= s->vel;
          s->vel += 0.009;
          s->intensity *= 0.95;          
        }
      }          
    }
  }
  void render()
  {
      for(int i=0; i<MAX_SPARKS; ++i)
      {
        SPARK *s = &sparks[i];
        int row = s->row;
        if((int)s->intensity && row < 150 && row > 0)
        {
          byte *p = &Strip.buffer[s->col][3*row];
          p[0]=0;
          p[1]=s->intensity;
          p[2]=0;
        }
      }
    }              
};

#endif // __TRAILS_H__
