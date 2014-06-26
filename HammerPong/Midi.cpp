#include "Arduino.h"
#include "Midi.h"

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

// state variables
byte midiInRunningStatus;
byte midiOutRunningStatus;
byte midiNumParams;
byte midiParams[2];
char midiParamIndex;

////////////////////////////////////////////////////////////////////////////////
// MIDI INIT
void midiSetup()
{
  // init the serial port
  Serial.begin(31250);
  Serial.flush();

  midiInRunningStatus = 0;
  midiOutRunningStatus = 0;
  midiNumParams = 0;
  midiParamIndex = 0;
}


////////////////////////////////////////////////////////////////////////////////
// MIDI READ
byte midiRead()
{
  // loop while we have incoming MIDI serial data
  while(Serial.available())
  {    
    // fetch the next byte
    byte ch = Serial.read();

    if(!!(ch & 0x80))
    {
      midiParamIndex = 0;
      midiInRunningStatus = ch; 
      switch(ch & 0xF0)
      {
        case 0xD0: //  Channel Pressure  1  pressure  
          midiNumParams = 1;
          break;    
        case 0x80: //  Note-off  2  key  velocity  
        case 0x90: //  Note-on  2  key  veolcity  
        case 0xA0: //  Aftertouch  2  key  touch  
        case 0xB0: //  Continuous controller  2  controller #  controller value  
        case 0xC0: //  Patch change  2  instrument #   
        case 0xE0: //  Pitch bend  2  lsb (7 bits)  msb (7 bits)  
        default:
          midiNumParams = 2;
          break;        
      }
    }    
    else if(midiInRunningStatus)
    {
      // gathering parameters
      midiParams[midiParamIndex++] = ch;
      if(midiParamIndex >= midiNumParams)
      {
        midiParamIndex = 0;
        return midiInRunningStatus;        
      }
    }
  }
  return 0;
}

