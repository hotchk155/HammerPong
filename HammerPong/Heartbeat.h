#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

class CHeartbeat
{
  enum 
  {
    P_HEARTBEAT = 13
  };
  unsigned long nextTick;
  byte state;
public:
  void setup()
  {
    nextTick = 0;
    state = 0;
    pinMode(P_HEARTBEAT, OUTPUT);
    digitalWrite(P_HEARTBEAT, LOW);
  }
  void run(unsigned long ticks)
  {
    if(!ticks || ticks >= nextTick)
    {
      nextTick = ticks + 500;
      state = !state;
      digitalWrite(P_HEARTBEAT, state? HIGH:LOW);      
    }
  }

};

#endif // __HEARTBEAT_H__
