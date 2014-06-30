#ifndef __ANIMATION_H__
#define __ANIMATION_H__

class CAnimation
{
  public:
    virtual void setup() = 0;
    virtual void run(unsigned long ticks) = 0;
    virtual void render(CStrip& Strip) = 0;
};


#define DROPS_MAX 40
class CRaindropAnimation : public CAnimation
{
  typedef struct
  {
    byte col;
    float row;
    float vel;
    byte intensity;  
  } DROP;
  DROP drops[DROPS_MAX];
  byte red;
  byte green;
  byte blue;
  float colour;
  
  unsigned long nextEvent;
public:  
  CRaindropAnimation()
  {
    setup();
  }
  void setup()
  {
    for(int i=0; i<DROPS_MAX; ++i)
      drops[i].row = -1;
    nextEvent = 0;
    colour = 0;
  }  
  void run(unsigned long ticks)
  {
    if(!ticks || nextEvent <= ticks)
    {
      byte WheelPos = colour;
        if(WheelPos < 85) 
        {
          red = WheelPos * 3;
          green = 255 - WheelPos * 3;
          blue = 0;
        } 
        else if(WheelPos < 170) 
        {
          WheelPos -= 85;
          red = 255 - WheelPos * 3;
          green = 0;
          blue = WheelPos * 3;
        } 
        else 
        {
         WheelPos -= 170;
          red = 0;
          green = WheelPos * 3;
          blue = 255 - WheelPos * 3;
        }
      colour += 0.01;
      
      for(int i=0; i<DROPS_MAX; ++i)
      {
        if(drops[i].row >= 0)
        {
          drops[i].row -= drops[i].vel;
          drops[i].vel += 0.01;
        }
        else if(random(100)==1)
        {
          drops[i].col = random(6);
          drops[i].row = 150;
          drops[i].vel = -random(20)/10.0;
          drops[i].intensity = random(100)+5;
        }
      }
      nextEvent = ticks + 10;
    }
  }
  void render(CStrip& Strip)
  {
    for(int i=0; i<DROPS_MAX; ++i)
    {
      if(drops[i].row>=0 && drops[i].row < 150)
      {      
        byte *p;
        p = &Strip.buffer[drops[i].col][3*(int)drops[i].row];
        p[0] = (float)drops[i].intensity * green/255.0;;
        p[1] = (float)drops[i].intensity * blue/255.0;
        p[2] = (float)drops[i].intensity * red/255.0;;
      }
    }
  }
};

#endif // __ANIMATION_H__

