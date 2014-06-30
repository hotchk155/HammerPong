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
    MIN_STRIP_ROW_LEFT  = CStrip::MIN_LEFT,
    MAX_STRIP_ROW_LEFT  = CStrip::MAX_LEFT,
    MIN_STRIP_ROW_RIGHT = CStrip::MIN_RIGHT,
    MAX_STRIP_ROW_RIGHT = CStrip::MAX_RIGHT,
    
    VIRTUAL_GAP         = 50,
    
    MIN_POS_LEFT        = 0,  // min virtual positon will map to MIN_STRIP_ROW_LEFT
    MAX_POS_LEFT        = MAX_STRIP_ROW_LEFT - MIN_STRIP_ROW_LEFT, // height of left strip
    MIN_POS_RIGHT       = MAX_POS_LEFT + VIRTUAL_GAP, // min virtual position on right strip (at top) maps to MAX_STRIP_ROW_LEFT
    MAX_POS_RIGHT       = MIN_POS_RIGHT + MAX_STRIP_ROW_RIGHT - MIN_STRIP_ROW_RIGHT, // total virtual lenght of strip
    
    MIN_POS = 0,
    MAX_POS = MAX_POS_RIGHT
  };  
  float pos;
  int facing;  
  float vel;
  unsigned long nextTick;
  
  int stripCol;     // base strip
  int stripRow[3];
  
  enum {
    INIT,
    SERVE_LEFT,
    SERVE_RIGHT,
    IN_PLAY,
    MISSED_LEFT,
    MISSED_RIGHT,
    HIDDEN
  };
  enum {
    LEFT_TO_RIGHT = 1,
    RIGHT_TO_LEFT = -1
  };
  int state;
  
public:
  
  ////////////////////////////////////////////////////////////////////
  void setup()
  {
    pos = 0;
    vel = 0;
    facing = LEFT_TO_RIGHT;
    nextTick = 0;
    stripRow[0] = stripRow[1] = stripRow[2] = -1;
    state = INIT;
  }
    
  ////////////////////////////////////////////////////////////////////
  void run(unsigned long ticks, CSparks& Sparks)
  {
    int prob;
    int row;
    const float dropRate = 0.99;
    if(!ticks || nextTick <= ticks)
    {
      
      switch(state)
      {
        case INIT:
          break;
          
        // Left player needs to serve
        case SERVE_LEFT:
          facing = LEFT_TO_RIGHT;
          pos -= (MIN_POS_LEFT+1);
          pos *= dropRate;
          pos += (MIN_POS_LEFT+1);
          nextTick = ticks + 3;
          
          // Check if the player has served
//          row = pos - MIN_POS_LEFT;
//          if(PlayerLeft.hitTest(row))
//            state = IN_PLAY;
          break;

        // Right player needs to serve
        case SERVE_RIGHT:
          facing = RIGHT_TO_LEFT;
          pos = (MAX_POS_RIGHT-1) - pos;
          pos *= dropRate;
          pos = (MAX_POS_RIGHT-1) - pos;
          nextTick = ticks + 3;
          
          // Check if the player has served
//          row = MAX_STRIP_ROW_RIGHT - (pos - MIN_POS_RIGHT);
//          if(PlayerRight.hitTest(row))
//            state = IN_PLAY;
          break;
          
        // The game is in progress
        case IN_PLAY:
        
          // Add sparks to the trail
          prob = 0;
          if(facing == LEFT_TO_RIGHT)
          {
            if(pos > MIN_POS_LEFT + 5 && pos < MAX_POS_LEFT)
              prob = 10+pos/10;   
            pos+=vel;                      
          }
          else
          {
            if(pos < MAX_POS_RIGHT - 5 && pos > MIN_POS_RIGHT)
              prob = 10+(MAX_POS_RIGHT - pos)/10;
             pos-=vel;
          }
          if(prob>0) 
          {
            float v = -vel/(2+random(5));
            int i = random(100);
            switch(random(prob))
            {
              case 1:
                if(stripRow[0] > 0)
                  Sparks.add(stripCol, stripRow[0],v,255,0,0,i);
                break;
              case 2:
                if(stripRow[1] > 0)
                  Sparks.add(stripCol+1, stripRow[1],v,255,0,0,i);
                break;
              case 3:
                if(stripRow[2] > 0)
                  Sparks.add(stripCol+2, stripRow[2],v,255,0,0,i);
                break;
            }          
          }
          nextTick = ticks + 1;
          break;
          
      case MISSED_LEFT:
      case MISSED_RIGHT:  
      case HIDDEN:
          nextTick = ticks + 1000;             
          break;
      }

      // Pre-calculate information needed to render the puck        
      // Prepare the set of row indexes for each of the 
      // 3 strip columns displaying the puck
      stripRow[0] = stripRow[1] = stripRow[2] = -1;
      if(state == HIDDEN)
      {
        // wont get shown
      }
      else if(facing == LEFT_TO_RIGHT) 
      {
        // Puck is facing as if moving from the LEFT to the RIGHT
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
        else
        {
          Serial.print("MISSED_RIGHT");
          state = MISSED_RIGHT;
        }
      }
      else
      {
        // Puck is facing as if moving from the RIGHT to the LEFT
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
        else
        {
          Serial.print("MISSED_LEFT");
          state = MISSED_LEFT;
        }
      }      
    }  
  }  
  
  ////////////////////////////////////////////////////////////////////
  void render(CStrip& Strip)
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

  ////////////////////////////////////////////////////////////////////
  // Place the puck at the top of the indicated player's strip and
  // start it falling ready to serve
  void readyToServe(int player)
  {
    if(CPlayer::RIGHT == player)
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
  byte hitTest(CPlayer& PlayerLeft,   CPlayer& PlayerRight)
  {
    if(0 == stripCol)
    {
      // puck is on left side, so if it is moving toward 
      // left player then check if the player hit it
      if(facing==RIGHT_TO_LEFT||state==SERVE_LEFT)
        return PlayerLeft.hitTest(stripRow[1]);
    }
    else
    {
      // puck is on right side, yada yada
      if(facing==LEFT_TO_RIGHT||state==SERVE_RIGHT)
        return PlayerRight.hitTest(stripRow[1]);
    }    
    return 0;    
  }
  
  ////////////////////////////////////////////////////////////////////
  // kick off the game
  void startPlay(float v)
  {
    Serial.println("startPlay");
    vel = v;
    if(SERVE_RIGHT == state)
      facing = RIGHT_TO_LEFT;
    else
      facing = LEFT_TO_RIGHT;
    state = IN_PLAY;
  }

  ////////////////////////////////////////////////////////////////////
  void reverse(float v)  
  {
    Serial.println("reverse");
    if(facing==LEFT_TO_RIGHT)
      facing = RIGHT_TO_LEFT;
    else
      facing =LEFT_TO_RIGHT;
    vel*=v;
  }
  
  ////////////////////////////////////////////////////////////////////
  inline int isOutOfPlay()
  {
    switch(state)
    {
      case MISSED_LEFT:
        return CPlayer::LEFT;
      case MISSED_RIGHT:        
        return CPlayer::RIGHT;
      default:
        return CPlayer::NEITHER;      
    }
  }
  
  ////////////////////////////////////////////////////////////////////
  void hide()
  {
    state = HIDDEN;
  }  
};

#endif //  __PUCK_H__
