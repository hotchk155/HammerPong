////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//
// LOW LEVEL MIDI HANDLING
//
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef __MIDI_H__
#define __MIDI_H__

//
// INCLUDE FILES
//
#include "Arduino.h"
#include "Midi.h"

class CMIDI
{
  // state variables
  byte inRunningStatus;
  byte outRunningStatus;
  byte numParams;
  char paramIndex;
  byte params[2];
public:
  void setup()
  {
    // init the serial port
    Serial.begin(9600);
    Serial1.begin(31250);
    Serial1.flush();
  
    inRunningStatus = 0;
    outRunningStatus = 0;
    numParams = 0;
    paramIndex = 0;
  }

  byte read()
  {
    // loop while we have incoming MIDI serial data
    while(Serial1.available())
    {    
      // fetch the next byte
      byte ch = Serial1.read();

      if(!!(ch & 0x80)) // is a status
      {
        paramIndex = 0;
        inRunningStatus = ch; 
        switch(ch & 0xF0)
        {
          case 0xD0: //  Channel Pressure  1  pressure  
            numParams = 1;
            break;    
          case 0x80: //  Note-off  2  key  velocity  
          case 0x90: //  Note-on  2  key  veolcity  
          case 0xA0: //  Aftertouch  2  key  touch  
          case 0xB0: //  Continuous controller  2  controller #  controller value  
          case 0xC0: //  Patch change  2  instrument #   
          case 0xE0: //  Pitch bend  2  lsb (7 bits)  msb (7 bits)  
          default:
            numParams = 2;
            break;        
        }
      }    
      else if(inRunningStatus)
      {
        // gathering parameters
        params[paramIndex++] = ch;
        if(paramIndex >= numParams)
        {
          paramIndex = 0;
          return inRunningStatus;        
        }
      }
    }
    return 0;
  }
  
  inline byte param1()
  {
      return params[0];
  }
  inline byte param2()
  {
      return params[1];
  }
};

#endif
