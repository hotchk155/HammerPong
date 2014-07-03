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
#include "Lights.h"
#include "Digits.h"
#include "Strip.h"
#include "Sparks.h"
#include "Player.h"
#include "Puck.h"

//
// MACRO DEFS
//
#define INITIAL_SPEED 0.7  // initial speed of puck after serve (positions per tick)
#define REBOUND_SPEED 1.05  // how much the speed increases when the puck is returned
#define MAX_SCORE  5
#define IDLE_TIMEOUT  25000

#define SOUND_BEGIN 'b'
#define SOUND_SCORE 'w'
#define SOUND_GAMEOVER 't'
#define SOUND_RETURN 's'
#define SOUND_SERVE 'r'


//
// GAME STATE MACHINE CLASS
//
class CGame
{
  enum 
  {
    P_SWING_LEFT = 22,
    P_SWING_RIGHT = 24
  };
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
  int serveTimeout;
  float wheelPos;
  
  CDigits     Digits;      // Handler for dual 7-seg score display
  CLights     Lights;      // Handler for console lights
  CStrip      Strip;       // Handler for LED strips
  CPlayer     PlayerLeft;  // Handler for left side player
  CPlayer     PlayerRight; // Handler for right side player
  CPuck       Puck;        // Handler for puck
  CSparks     Sparks;       // Handler for trail of "sparks" left by puck
  
public:  

  ////////////////////////////////////////////////////////////////////////////
  CGame() :
    PlayerLeft(CPlayer::LEFT, CPlayer::BLUE, CStrip::MIN_LEFT),
    PlayerRight(CPlayer::RIGHT, CPlayer::GREEN, CStrip::MIN_RIGHT)
  {
    
  }

  ////////////////////////////////////////////////////////////////////////////
  void setup()
  {
    Serial.begin(9600);
    pinMode(P_SWING_LEFT, INPUT);
    pinMode(P_SWING_RIGHT, INPUT);
    Digits.setup();
    Lights.setup();
    Strip.setup();
    PlayerLeft.setup();
    PlayerRight.setup();
    Puck.setup();
    Sparks.setup();
    Lights.setButton(1);
    transition(BEGIN_STATE);
  }

  ////////////////////////////////////////////////////////////////////////////
  void sound(char s)
  {
    Serial.write(s);
  }
  
  ////////////////////////////////////////////////////////////////////////////
  void transition(int newState)
  {
    switch(newState)
    {
      case BEGIN_STATE:
        servingPlayer = (analogRead(0)&1) ? CPlayer::LEFT : CPlayer::RIGHT;
        scoreLeft = 0;
        scoreRight = 0;
        tickPeriod = 200;
        break;
          
      case SERVING_STATE:
        PlayerLeft.unexplode();
        PlayerRight.unexplode();
        Puck.readyToServe(servingPlayer);      
        Lights.sequence(CLights::SLOW_RISE_BOTH_SIDES);
        Digits.set(scoreLeft, scoreRight);
        if(servingPlayer == CPlayer::RIGHT)
          Digits.sequence(CDigits::BLINK_RIGHT);
        else          
          Digits.sequence(CDigits::BLINK_LEFT);
        tickPeriod = 1;
        serveTimeout = IDLE_TIMEOUT; 
        break;

      case PLAYING_STATE:
        rallyCount = 0;
        Digits.set(rallyCount);
        Digits.sequence(CDigits::NO_SEQUENCE);
        Lights.sequence(CLights::FAST_FALL_BOTH_SIDES);
        Puck.startPlay(INITIAL_SPEED);
        sound(SOUND_SERVE);
        tickPeriod = 1;
        break;
        
      case SCORED_STATE:
        sound(SOUND_SCORE);
        Lights.sequence(CLights::SCORE);
        tickPeriod = 3000;
        break;
        
      case GAMEOVER_STATE:
        sound(SOUND_GAMEOVER);
        Digits.set(scoreLeft, scoreRight);
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
        tickPeriod = IDLE_TIMEOUT; 
        break;

      case ATTRACT_STATE:
        PlayerLeft.hide();
        PlayerRight.hide();
        Puck.hide();
        Digits.sequence(CDigits::MEANDER);
        Lights.sequence(CLights::MEANDER);
        wheelPos = random(255);
        tickPeriod = 30;
        break;
    }
    state = newState;
    nextTick = 0;
  }
  
  ////////////////////////////////////////////////////////////////////////////
  void run(unsigned long ticks)
  {
    if(digitalRead(P_SWING_LEFT))
      PlayerLeft.swing();
    if(digitalRead(P_SWING_RIGHT))
      PlayerRight.swing();
    
    PlayerLeft.run(ticks);
    PlayerRight.run(ticks);
    Puck.run(ticks, Sparks); 
    Sparks.run(ticks);
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
          if(--serveTimeout <= 0)
              transition(ATTRACT_STATE);
          break;
          
        //////////////////////////////////////////////////////////////////
        // Game in progress
        case PLAYING_STATE:
          {          
            int losingPlayer = Puck.isOutOfPlay();
            if(CPlayer::LEFT == losingPlayer)
            {
                PlayerLeft.explode(Sparks);
                scoreRight++;
                servingPlayer = CPlayer::RIGHT;
                transition(SCORED_STATE);
            }
            else if(CPlayer::RIGHT == losingPlayer)
            {
                PlayerRight.explode(Sparks);
                scoreLeft++;
                servingPlayer = CPlayer::LEFT;
                transition(SCORED_STATE);
            }
            else if(Puck.hitTest(PlayerLeft, PlayerRight))
            {              
                sound(SOUND_RETURN);
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
          transition(ATTRACT_STATE);
          break;
        
        //////////////////////////////////////////////////////////////////
        // Nobody playing, do some other stuff        
        case ATTRACT_STATE:
          Sparks.add(random(6), 150, (float)random(50)/50.0, wheelPos, random(255));
          wheelPos+=0.05;
          if(wheelPos >= 255)
            wheelPos = 0;
          break;
      }        
      nextTick = ticks + tickPeriod;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////
  void render()
  {
    Strip.clear();
    Sparks.render(Strip);
    PlayerLeft.render(Strip);
    PlayerRight.render(Strip);
    Puck.render(Strip);
    Strip.refresh();
  }
};

#endif // __GAME_H__
