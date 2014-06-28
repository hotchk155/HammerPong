#ifndef __PUCK_H__
#define  __PUCK_H__

#include "Trail.h"

////////////////////////////////////////////////////////////////////
// 
// The puck uses its own coordinate system
// which starts at 0 from the bottom of the LEFT
// side of the strip then counts up locations 
// ascending the left side, includes a "virtual"
// gap on the cross piece them counts up as 
// we descend the RIGHT hand side.
// It treats the strips as a continuous length
// 
//
class CPuck 
{
  enum {
    
    // Limits on physically addressable/visible strip rows
    MIN_STRIP_ROW_LEFT  = 0,
    MAX_STRIP_ROW_LEFT  = 150,
    MIN_STRIP_ROW_RIGHT = 0,
    MAX_STRIP_ROW_RIGHT = 150,
    
    VIRTUAL_GAP         = 50,
    
    MIN_POS_LEFT        = 0,  // min virtual positon will map to MIN_STRIP_ROW_LEFT
    MAX_POS_LEFT        = MAX_STRIP_ROW_LEFT - MIN_STRIP_ROW_LEFT, // height of left strip
    MIN_POS_RIGHT       = MAX_POS_LEFT + VIRTUAL_GAP, // min virtual position on right strip (at top) maps to MAX_STRIP_ROW_LEFT
    MAX_POS_RIGHT       = MIN_POS_RIGHT + MAX_STRIP_ROW_RIGHT - MIN_STRIP_ROW_RIGHT, // total virtual lenght of strip
    
    MIN_POS = 0,
    MAX_POS = MAX_POS_RIGHT
  };  
  float pos;
  int dir;  
  float vel;
  unsigned long nextTick;
  
  int stripCol;     // base strip
  int stripRow[3];
  
  enum {
    SERVE_LEFT,
    SERVE_RIGHT,
    IN_PLAY
  };
  int state;
  
  enum {
    LEFT,
    RIGHT,
  };
public:
  
  void setup()
  {
    pos = 0;
    vel = 0.9;
    dir = 1;
    nextTick = 0;
    stripRow[0] = stripRow[1] = stripRow[2] = -1;
    state = IN_PLAY;
//    newServe(LEFT);
  }
  
  ////////////////////////////////////////////////////////////////////
  void newServe(int which)
  {
    if(RIGHT == which)
    {
      state = SERVE_RIGHT;
      pos = MIN_POS_RIGHT;
    }
    else
    {
      state = SERVE_LEFT;
      pos = MAX_POS_LEFT;
    }
  }
  
  ////////////////////////////////////////////////////////////////////
  void run(unsigned long ticks, CTrail& Trail)
  {
    int prob;
    const float dropRate = 0.99;
    if(!ticks || nextTick <= ticks)
    {
      
      switch(state)
      {
        case SERVE_LEFT:
          dir = 1;
          pos -= (MIN_POS_LEFT+1);
          pos *= dropRate;
          pos += (MIN_POS_LEFT+1);
          nextTick = ticks + 3;
          break;
        case SERVE_RIGHT:
          dir = 0;
          pos = (MAX_POS_RIGHT-1) - pos;
          pos *= dropRate;
          pos = (MAX_POS_RIGHT-1) - pos;
          nextTick = ticks + 3;
          break;
        case IN_PLAY:
          prob = 0;
          if(dir > 0)
          {
            if(pos > MIN_POS_LEFT + 5 && pos < MAX_POS_LEFT)
              prob = 10+pos/10;   
            pos+=vel;          
            if(pos >= MAX_POS - 1)
              dir=-1;
            
          }
          else
          {
            if(pos < MAX_POS_RIGHT - 5 && pos > MIN_POS_RIGHT)
              prob = 10+(MAX_POS_RIGHT - pos)/10;
             pos-=vel;
            if(pos < 1)
              dir=1;
          }
          if(prob>0) 
          {
            switch(random(prob))
            {
              case 1:
                if(stripRow[0] > 0)
                  Trail.add(stripCol, stripRow[0],vel);
                break;
              case 2:
                if(stripRow[1] > 0)
                  Trail.add(stripCol+1, stripRow[1],vel);
                break;
              case 3:
                if(stripRow[2] > 0)
                  Trail.add(stripCol+2, stripRow[2],vel);
                break;
            }          
          }
          nextTick = ticks + 1;
          break;
      }

      // Pre-calculate information needed to render the puck        
      // Prepare the set of row indexes for each of the 
      // 3 strip columns displaying the puck
      stripRow[0] = stripRow[1] = stripRow[2] = -1;
      if(dir > 0) 
      {
        // Puck is moving from the LEFT to the RIGHT
        if(pos < MIN_POS_LEFT)
        {
          // off strip
        }
        else if(pos < MAX_POS_LEFT)
        {          
          // on the left strip       
          stripCol = 0;
          
          // All pixel are in the same row of the strip
          int row = (pos + MIN_STRIP_ROW_LEFT);
          if(row >= MIN_STRIP_ROW_LEFT && MAX_STRIP_ROW_LEFT)
          {
            stripRow[0] = stripRow[1] = stripRow[2] = row;
          }
        }      
        else if(pos < MIN_POS_RIGHT)
        {
          // in the gap
        }
        else if(pos < MAX_POS_RIGHT)
        {
          // on the right strip
          stripCol = 3; 
          
          // middle pixel is on 1 strip row lower
          int row = MAX_STRIP_ROW_RIGHT - (pos - MIN_POS_RIGHT);
          if(row >= MIN_STRIP_ROW_RIGHT && row < MAX_STRIP_ROW_RIGHT)
          {
            stripRow[0] = stripRow[2] = row;
            if(row > MIN_STRIP_ROW_RIGHT)
              stripRow[1] = row - 1;
          }
        }
      }
      else
      {
        // Puck is moving from the RIGHT to the LEFT
        if(pos >= MAX_POS_RIGHT)
        {
          // off end of strip
        }
        else if(pos > MIN_POS_RIGHT)
        {          
          // on the right strip       
          stripCol = 3;
          
          // All pixel are in the same row of the strip
          int row = MAX_STRIP_ROW_RIGHT - (pos - MIN_POS_RIGHT);
          if(row >= MIN_STRIP_ROW_RIGHT && row < MAX_STRIP_ROW_RIGHT)
          {
            stripRow[0] = stripRow[1] = stripRow[2] = row;
          }
        }      
        else if(pos > MAX_POS_LEFT)
        {
          // in the gap
        }
        else if(pos >= MIN_POS_LEFT)
        {
          // on the left strip
          stripCol = 0; 
          
          // middle pixel is on 1 strip row lower
          int row = (pos + MIN_STRIP_ROW_LEFT);
          if(row >= MIN_STRIP_ROW_LEFT && MAX_STRIP_ROW_LEFT)
          {
            stripRow[0] = stripRow[2] = row;
            if(row > MIN_STRIP_ROW_LEFT)
              stripRow[1] = row - 1;
          }
        }
      }      
    }  
  }  
  
  ////////////////////////////////////////////////////////////////////
  void render()
  {
    byte *p;
    int col = stripCol;
    for(int i=0; i<3; ++i)
    {
      if(stripRow[i] >= 0)
      {
        int ofs = 3 * stripRow[i];
        byte *p = &Strip.buffer[col][ofs];
        p[0]=p[1]=p[2]=0xFF;
      }
      ++col;
    }    
  }
};

#endif //  __PUCK_H__
