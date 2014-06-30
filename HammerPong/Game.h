////////////////////////////////////////////////////////////////////////////
//
// This class defines the state machine for the high level Hammer Pong
// behaviour including game play, idle mode etc
//
////////////////////////////////////////////////////////////////////////////

#ifndef __GAME_H__
#define __GAME_H__

//
// INCLUDE FILES
//
#include "Midi.h"
#include "Lights.h"
#include "Digits.h"
#include "Strip.h"
#include "Player.h"
#include "Puck.h"
#include "Trail.h"
#include "Animation.h"

//
// MACRO DEFS
//
#define INITIAL_SPEED 0.2  // initial speed of puck after serve (positions per tick)
#define REBOUND_SPEED 1.1  // how much the speed increases when the puck is returned
#define MAX_SCORE  9

//
// GAME STATE MACHINE CLASS
//
class CGame
{
  // STATES
  enum 
  {
    BEGIN_STATE,      // The state we start up in
    SERVING_STATE,    // Waiting for a player to serve
    PLAYING_STATE,    // Puck is in play
    SCORED_STATE,     // A player has scored
    GAMEOVER_STATE,   // A plater has won
    ATTRACT_STATE     // The game has been idle so show light show etc
  };
  
  int state;              // main state
  int servingPlayer;      // player next to serve
  int scoreLeft;          // left player score
  int scoreRight;         // right player score
  int rallyCount;         // length of current rally
  int tickPeriod;         // state machine run period (ticks) 
  unsigned long nextTick; // scheduled time of next run
  
  CMIDI       MIDI;        // MIDI comms handler
  CDigits     Digits;      // Handler for dual 7-seg score display
  CLights     Lights;      // Handler for console lights
  CStrip      Strip;       // Handler for LED strips
  CPlayer     PlayerLeft;  // Handler for left side player
  CPlayer     PlayerRight; // Handler for right side player
  CPuck       Puck;        // Handler for puck
  CTrail      Trail;       // Handler for trail of "sparks" left by puck
  
public:  

  ////////////////////////////////////////////////////////////////////////////
  CGame() :
    PlayerLeft(CPlayer::LEFT, CPlayer::BLUE, 0),
    PlayerRight(CPlayer::RIGHT, CPlayer::GREEN, 0)
  {
  }

  ////////////////////////////////////////////////////////////////////////////
  void setup()
  {
    MIDI.setup();
    Digits.setup();
    Lights.setup();
    Strip.setup();
    PlayerLeft.setup();
    PlayerRight.setup();
    Puck.setup();
    Trail.setup();
    Lights.setButton(1);
    transition(BEGIN_STATE);
  }

  ////////////////////////////////////////////////////////////////////////////
  void transition(int newState)
  {
    switch(newState)
    {
      case BEGIN_STATE:
        servingPlayer = random(2) ? CPlayer::LEFT : CPlayer::RIGHT;
        scoreLeft = 0;
        scoreRight = 0;
        tickPeriod = 200;
        break;
          
      case SERVING_STATE:
        Puck.readyToServe(servingPlayer);      
        Lights.sequence(CLights::SLOW_RISE_BOTH_SIDES);
        Digits.set(scoreLeft, scoreRight);
        if(servingPlayer == CPlayer::RIGHT)
          Digits.sequence(CDigits::BLINK_RIGHT);
        else          
          Digits.sequence(CDigits::BLINK_LEFT);
        tickPeriod = 1;
        break;

      case PLAYING_STATE:
        rallyCount = 0;
        Digits.set(rallyCount);
        Digits.sequence(CDigits::NO_SEQUENCE);
        Lights.sequence(CLights::FAST_FALL_BOTH_SIDES);
        Puck.startPlay(INITIAL_SPEED);
        tickPeriod = 1;
        break;
        
      case SCORED_STATE:
        Lights.sequence(CLights::SCORE);
        tickPeriod = 2000;
        break;
        
      case GAMEOVER_STATE:
        if(scoreLeft > scoreRight)
        {
          Lights.sequence(CLights::VICTORY_LEFT);
          Digits.sequence(CDigits::BLINK_LEFT_DIM_RIGHT);
        }
        else
        {
          Lights.sequence(CLights::VICTORY_RIGHT);
          Digits.sequence(CDigits::BLINK_RIGHT_DIM_LEFT);
        }
        tickPeriod = 100;
        break;

      case ATTRACT_STATE:
        Digits.sequence(CDigits::MEANDER);
        tickPeriod = 100;
        break;
    }
    state = newState;
  }
  
  ////////////////////////////////////////////////////////////////////////////
  void run(unsigned long ticks)
  {
    if(MIDI.read() == 0x90 && MIDI.param2())
    {
      switch(MIDI.param1())
      {
        case 60:
          PlayerLeft.swing();
          break;
        case 61:
          PlayerLeft.swing();
          break;
      }
    }
    
    PlayerLeft.run(ticks);
    PlayerRight.run(ticks);
    Puck.run(ticks, Trail, PlayerLeft, PlayerRight);  
    Trail.run(ticks);
    Digits.run(ticks);
    Lights.run(ticks);
      
    if(!ticks || nextTick <= ticks)
    {           
      switch(state)
      {
        //////////////////////////////////////////////////////////////////
        // Game has just started
        case BEGIN_STATE:
          transition(SERVING_STATE);
          break;
          
        //////////////////////////////////////////////////////////////////
        // Waiting for the serving player to make a serve
        case SERVING_STATE:
          if(Puck.hitTest(PlayerLeft, PlayerRight))
              transition(PLAYING_STATE);
          break;
          
        //////////////////////////////////////////////////////////////////
        // Game in progress
        case PLAYING_STATE:
          {          
            int losingPlayer = Puck.isOutOfPlay();
            if(CPlayer::LEFT == losingPlayer)
            {
                scoreRight++;
                servingPlayer = CPlayer::RIGHT;
                transition(SCORED_STATE);
            }
            else if(CPlayer::RIGHT == losingPlayer)
            {
                scoreLeft++;
                servingPlayer = CPlayer::LEFT;
                transition(SCORED_STATE);
            }
            else if(Puck.hitTest(PlayerLeft, PlayerRight))
            {
                Puck.reverse(REBOUND_SPEED);
                Digits.set(++rallyCount);
            }             
          }
          break;        
          
        //////////////////////////////////////////////////////////////////
        // A player has scored
        case SCORED_STATE:
          if(scoreLeft >= MAX_SCORE || scoreRight >= MAX_SCORE)
            transition(GAMEOVER_STATE);
          else
            transition(SERVING_STATE);
          break;
          
        //////////////////////////////////////////////////////////////////
        // Game over
        case GAMEOVER_STATE:
          break;
        
        //////////////////////////////////////////////////////////////////
        // Nobody playing, do some other stuff        
        case ATTRACT_STATE:
          break;
      }        
      nextTick = ticks + tickPeriod;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////
  void render()
  {
    Strip.clear();
    Trail.render(Strip);
    PlayerLeft.render(Strip);
    PlayerRight.render(Strip);
    Puck.render(Strip);
    Strip.refresh();
  }
};

#endif // __GAME_H__
