/*  Plays a fluctuating ambient wash using pairs
    of slightly detuned oscillators, following an example
    from Miller Puckette's Pure Data manual.
  
    The detune frequencies are modified by chance in
    updateControl(), and the outputs of 14 audio
    oscillators are summed in updateAudio().
  
    Demonstrates the use of fixed-point Q16n16
    format numbers, mtof() for converting midi note
    values to frequency, and xorshift96() for random numbers.
  
    Circuit: Audio output on digital pin 9 (on a Uno or similar), or 
    check the README or http://sensorium.github.com/Mozzi/
  
    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users
  
    Tim Barrass 2012.
    This example code is in the public domain.
*/

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/whitenoise8192_int8.h>
#include <tables/cos8192_int8.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h> // for Q16n16 fixed-point fractional number type
#include <StateVariable.h>

// Beating sines 1
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos1(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos2(COS8192_DATA);

// Beating sines 2
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos3(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos4(COS8192_DATA);

// Noise
Oscil <WHITENOISE8192_NUM_CELLS, AUDIO_RATE> aNoise1(WHITENOISE8192_DATA); // audio noise

StateVariable <LOWPASS> svf;


void handleControlChange(byte channel, byte number, byte value)
{
  if(1==number)
  {
    aCos3.setFreq(100.0f + 2*value);
    aCos4.setFreq(101.0f + 2*value);  
  }
  if(70==number)
  {
    aCos1.setFreq(50.0f + 2*value);
    aCos2.setFreq(50.0f + 2*value);  
  }
  
}

void setup(){
  startMozzi(64); // a literal control rate here

  // set Oscils with chosen frequencies
  aCos1.setFreq(50.0f);
  aCos2.setFreq(50.0f);
  aCos3.setFreq(100.0f);
  aCos4.setFreq(101.0f);
  
  
  aNoise1.setFreq(2);
  
  svf.setResonance(150);
  svf.setCentreFreq(1200);
  
  usbMIDI.setHandleControlChange(handleControlChange);
}


void loop(){
  audioHook();
}


unsigned int j=0;
void updateControl(){
  usbMIDI.read();
}


int updateAudio(){
  int asig = aCos1.next() + aCos2.next() + aCos3.next() + aCos4.next();
  return (asig>>2);// + svf.next(aNoise1.next()))>>1;
}
