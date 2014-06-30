////////////////////////////////////////////////////////////////////
// 
// PLAYER BAT
// 
////////////////////////////////////////////////////////////////////
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
      CONTRACT,
      EXPLODED
    };
    
    int which;
    int colour;
    int bottomRow;        
    int state;
    float scale;
    float an;
    int topRow;
    unsigned long nextTick;
    
  public:
    // Identify a specific player
    enum {
      NEITHER,
      LEFT, 
      RIGHT
    };
    
    // Specify a single colour (in LED addressing order)
    enum {
      GREEN,
      RED,
      BLUE
    };
    
    ////////////////////////////////////////////////////////////////////    
    CPlayer(int w, int c, int r)
    {
      which = w;
      colour = c;
      bottomRow = r;
    }

    ////////////////////////////////////////////////////////////////////    
    void setup()
    {
      scale = 0;
      state = IDLE;
      nextTick = 0;
      an = 0;
    }

    ////////////////////////////////////////////////////////////////////    
    void run(unsigned long ticks)
    {
      if(!ticks || nextTick <= ticks)
      {
        nextTick = ticks + 10;
        switch(state)
        {
          case IDLE:
            an+=0.1;
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
    
    ////////////////////////////////////////////////////////////////////  
    // Render the bat into the strip buffer    
    void render(CStrip& Strip)
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
        topRow = 0;
        
      }
      else if(state != EXPLODED)
      {
        byte intensity = 0x2;
        float row = bottomRow;
        for(int i=0; i<NUM_ELEMENTS; ++i)
        {        
          if(row >= Strip.LENGTH)       
            break;
          topRow = (int)row;
          int index = 3 * topRow;
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
          row += scale;
        }
      }      
    }
    
    ////////////////////////////////////////////////////////////////////
    // Swing the bat if it is ready. Returns 1 if started swinging
    byte swing()
    {
      if(IDLE == state)
      {
        state = EXPAND;
        return 1;
      }
      return 0;
    }
    
    ////////////////////////////////////////////////////////////////////
    // Check if the bat is swinging and occupies a given strip row
    byte hitTest(int row)
    {
      if((EXPAND == state) && (row >= 0) && (row <= topRow))
      {
        Serial.println("hittest ok");        
        return 1;
      }
      return 0;
    }
    
    void explode(CSparks &Sparks)
    {
      byte red = (colour == RED)? 255:0;
      byte green = (colour == GREEN)? 255:0;
      byte blue = (colour == BLUE)? 255:0;
      int baseCol = (which == LEFT) ? CStrip::BASE_COL_LEFT : CStrip::BASE_COL_RIGHT;
      state = EXPLODED;
      for(int i=0; i<30; ++i)
         Sparks.add(baseCol + i%3, bottomRow, -(float)random(15)/20.0, red, green, blue, random(200));
    }
    
    void unexplode()
    {
      if(state == EXPLODED)
        state = IDLE;
    }
};

#endif // __PLAYER_H__
