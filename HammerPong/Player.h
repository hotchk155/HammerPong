#ifndef __PLAYER_H__
#define __PLAYER_H__

class CPlayer
{
    enum {
      NUM_ELEMENTS = 7,
      MAX_SCALE = 5      
    };
    
    enum {
      IDLE,
      EXPAND,
      CONTRACT      
    };
    
    int which;
    int colour;
    int bottomRow;        
    int state;
    float scale;
    float an;
    unsigned long nextTick;
  public:
    enum {
      LEFT, 
      RIGHT
    };
    enum {
      GREEN,
      RED,
      BLUE
    };
    CPlayer(int w, int c, int r)
    {
      which = w;
      colour = c;
      bottomRow = r;
    }
    void setup()
    {
      scale = 0;
      state = IDLE;
      nextTick = 0;
      an = 0;
    }
    void run(unsigned long ticks)
    {
      if(!ticks || nextTick <= ticks)
      {
        nextTick = ticks + 10;
        switch(state)
        {
          case IDLE:
            an+=0.1;
            scale = 0;
            if(!random(100))
              state = EXPAND;
            break;
          case EXPAND:
            if(scale<MAX_SCALE)
            {
              scale += 0.6;
            }
            else
            {
              state = CONTRACT;
            }
            break;
          case CONTRACT:
            scale -= 0.2;
            if(scale<=0)
            {
              an=0;
              scale = 0;
              state = IDLE;
            }         
            break; 
        }
      }
    }
    void render()
    {
      int col;
      byte *p;
      if(which == RIGHT)
      {
        col = 3;
      }
      else
      {
        col = 0;
      }

      if(state == IDLE)
      {
        int index = 3 * bottomRow;
        byte intensity = 155.0+100.0*cos(an);
        p=&Strip.buffer[col][index];
        p[0]=p[1]=p[2] = 0; 
        p[colour] = intensity;

        p=&Strip.buffer[col+1][index];
        p[0]=p[1]=p[2] = 0; 
        p[colour] = intensity;

        p=&Strip.buffer[col+2][index];
        p[0]=p[1]=p[2] = 0; 
        p[colour] = intensity;
        
      }
      else
      {
        byte intensity = 0x2;
        for(int i=0; i<NUM_ELEMENTS; ++i)
        {        
          int row = bottomRow + scale*i;
          if(row < Strip.LENGTH)       
          {            
            int index = 3 * row;
            p=&Strip.buffer[col][index];
            p[0]=p[1]=p[2] = 0; 
            p[colour] = intensity;

            p=&Strip.buffer[col+1][index];
            p[0]=p[1]=p[2] = 0; 
            p[colour] = intensity;

            p=&Strip.buffer[col+2][index];
            p[0]=p[1]=p[2] = 0; 
            p[colour] = intensity;
            
            intensity <<= 1;
            intensity |= 1;
          }
        }
      }
    }
};

#endif // __PLAYER_H__
