#include "Adafruit_NeoPixel_Mod.h"

#define PIN 6
#define P_STRIP_A0 3
#define P_STRIP_A1 4
#define P_STRIP_A2 5

byte *pixels;
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define STRIP_MASK 0b00111000
#define STRIP0_MASK (1<<3)
#define STRIP1_MASK (1<<4)
#define STRIP2_MASK 0

void selectStrip(byte which)
{
  switch(which)
  {
    case 0:
    PORTD &= ~STRIP_MASK;
    PORTD |= STRIP0_MASK;
    break;
//      digitalWrite(P_STRIP_A0, HIGH);
//      digitalWrite(P_STRIP_A1, LOW);
//      digitalWrite(P_STRIP_A2, LOW);
      break;
    case 1:
    PORTD &= ~STRIP_MASK;
    PORTD |= STRIP1_MASK;
    break;
    
//      digitalWrite(P_STRIP_A0, LOW);
//      digitalWrite(P_STRIP_A1, HIGH);
//      digitalWrite(P_STRIP_A2, LOW);
      break;
    case 2:
    PORTD &= ~STRIP_MASK;
    PORTD |= STRIP2_MASK;
//      digitalWrite(P_STRIP_A0, LOW);
//      digitalWrite(P_STRIP_A1, LOW);
//      digitalWrite(P_STRIP_A2, LOW);
      break;
  }
}

typedef struct 
{
  byte x;
  byte c;
  byte i;
  float y;
  float dy;
} BLOB;
#define MAX_BLOB 10
BLOB blobs[MAX_BLOB];
void initBlobs()
{
  memset(blobs,0,sizeof(blobs));
}
void runBlobs(int puckY)
{
  for(int i=0; i<MAX_BLOB; ++i)
  {
    BLOB *p = &blobs[i];
    if(p->y <=0 && !p->i)
    {
      p->y = puckY;
      p->dy = 0.1;
      p->x = i%3;
//      p->c = random(10);
      p->i = 20+random(235);
    }
    else if(!p->c)
    {
      p->y -= p->dy;
      p->dy += 0.0005;
      if(p->i>0)
        p->i--;
    }
    else
    {
        --p->c;
    }
  }
}

void renderBlobs(int col, byte *pixels)
{
  for(int i=0; i<MAX_BLOB; ++i)
  {
    BLOB *p = &blobs[i];
    if(p->x == col)
    {

      if(p->y >= 0 && p->y < 149 )
      {
       
       pixels[1+(3*(int)p->y)] = p->i;
      }
    }
  }
}
/*

            .     .
               X
            X     X
               .
            X     X
               X
*/

void puckRender(int col, int y, byte *pixels)
{
  byte *p = &pixels[3*y];
  switch(col)
  {
    case 0:
    case 2:
      p[0] = p[1] = p[2] = 255;
      p-=3;
      p[0] = p[1] = p[2] = 255;
      break;
    case 1:
      p[0] = p[1] = p[2] = 255;
      p-=6;
      p[0] = p[1] = p[2] = 255;
      break;
  }
}

void setup() {
  pinMode(P_STRIP_A0, OUTPUT);
  pinMode(P_STRIP_A1, OUTPUT);
  pinMode(P_STRIP_A2, OUTPUT);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
 initBlobs();
}
int puckY=0;
void loop() {
  
  //runBlobs(puckY);
  pixels = strip.getPixels();
  for(int col = 0; col < 1; ++col)
  {
    selectStrip(col);
    memset(pixels, 0, 150*3);
    //renderBlobs(col, pixels);
    puckRender(col, puckY, pixels);
    strip.show();
  }
//  ++puckY;
  ++puckY;
  if(puckY>149)puckY=0;
//  delay(4);
}
  /*
  for(int i=0; i<3; ++i)
  {
    selectStrip(i);
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  // Send a theater pixel chase in...
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127,   0,   0), 50); // Red
  theaterChase(strip.Color(  0,   0, 127), 50); // Blue

  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
  }
}*/

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

