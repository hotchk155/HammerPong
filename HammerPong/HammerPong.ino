 #include <Arduino.h>



// 'type' flags for LED pixels (third parameter to constructor):
#define NEO_RGB     0x00 // Wired for RGB data order
#define NEO_GRB     0x01 // Wired for GRB data order
#define NEO_COLMASK 0x01
#define NEO_KHZ800  0x02 // 800 KHz datastream
#define NEO_SPDMASK 0x02
// Trinket flash space is tight, v1 NeoPixels aren't handled by default.
// Remove the ifndef/endif to add support -- but code will be bigger.
// Conversely, can comment out the #defines to save space on other MCUs.
#ifndef __AVR_ATtiny85__
#define NEO_KHZ400  0x00 // 400 KHz datastream
#endif

class Adafruit_NeoPixel {

 public:

  // Constructor: number of LEDs, pin number, LED type
  Adafruit_NeoPixel(uint16_t n, uint8_t p=6, uint8_t t=NEO_GRB + NEO_KHZ800);
  ~Adafruit_NeoPixel();

  void
    begin(void),
    show(void),
    setPin(uint8_t p),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    setBrightness(uint8_t);
  uint8_t
   *getPixels(void) const;
  uint16_t
    numPixels(void) const;
  static uint32_t
    Color(uint8_t r, uint8_t g, uint8_t b);
  uint32_t
    getPixelColor(uint16_t n) const;

 private:

  const uint16_t
    numLEDs,       // Number of RGB LEDs in strip
    numBytes;      // Size of 'pixels' buffer below
  uint8_t
    pin,           // Output pin number
    brightness,
   *pixels,        // Holds LED color values (3 bytes each)
    rOffset,       // Index of red byte within each 3-byte pixel
    gOffset,       // Index of green byte
    bOffset;       // Index of blue byte
  const uint8_t
    type;          // Pixel flags (400 vs 800 KHz, RGB vs GRB color)
  uint32_t
    endTime;       // Latch timing reference
#ifdef __AVR__
  const volatile uint8_t
    *port;         // Output PORT register
  uint8_t
    pinMask;       // Output PORT bitmask
#endif

};


Adafruit_NeoPixel::Adafruit_NeoPixel(uint16_t n, uint8_t p, uint8_t t) : numLEDs(n), numBytes(n * 3), pin(p), pixels(NULL)
  , type(t)
{
  if ((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
  }
  if (t & NEO_GRB) { // GRB vs RGB; might add others if needed
    rOffset = 1;
    gOffset = 0;
  } else {
    rOffset = 0;
    gOffset = 1;
  }
  bOffset = 2;
}

Adafruit_NeoPixel::~Adafruit_NeoPixel() {
  if (pixels) free(pixels);
  pinMode(pin, INPUT);
}

void Adafruit_NeoPixel::begin(void) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void Adafruit_NeoPixel::show(void) {

  if (!pixels) return;

  // Data latch = 50+ microsecond pause in the output stream.  Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed.  This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while ((micros() - endTime) < 50L);
  // endTime is a private member (rather than global var) so that mutliple
  // instances on different pins can be quickly issued in succession (each
  // instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with any pin,
  // SBI/CBI instructions are eschewed in favor of full PORT writes via the
  // OUT or ST instructions.  It relies on two facts: that peripheral
  // functions (such as PWM) take precedence on output pins, so our PORT-
  // wide writes won't interfere, and that interrupts are globally disabled
  // while data is being issued to the LEDs, so no other code will be
  // accessing the PORT.  The code takes an initial 'snapshot' of the PORT
  // state, computes 'pin high' and 'pin low' values, and writes these back
  // to the PORT register as needed.

  noInterrupts(); // Need 100% focus on instruction timing



#define SCALE      VARIANT_MCK / 2UL / 1000000UL
#define INST       (2UL * F_CPU / VARIANT_MCK)
#define TIME_800_0 ((int)(0.40 * SCALE + 0.5) - (5 * INST))
#define TIME_800_1 ((int)(0.80 * SCALE + 0.5) - (5 * INST))
#define PERIOD_800 ((int)(1.25 * SCALE + 0.5) - (5 * INST))

  int             pinMaskA, pinMaskB, pinMaskC, time0, time1, period, t;
  Pio            *port;
  volatile WoReg *portSet, *portClear, *timeValue, *timeReset;
  uint8_t        *p, *end, pix, mask;

  pmc_set_writeprotect(false);
  pmc_enable_periph_clk((uint32_t)TC3_IRQn);
  TC_Configure(TC1, 0,
               TC_CMR_WAVE | TC_CMR_WAVSEL_UP | TC_CMR_TCCLKS_TIMER_CLOCK1);
  TC_Start(TC1, 0);

  pinMaskA   = g_APinDescription[5].ulPin; // Don't 'optimize' these into
  pinMaskB   = g_APinDescription[6].ulPin; // Don't 'optimize' these into
  pinMaskC   = g_APinDescription[7].ulPin; // Don't 'optimize' these into
  port      = g_APinDescription[pin].pPort; // declarations above.  Want to
  portSet   = &(port->PIO_SODR);            // burn a few cycles after
  portClear = &(port->PIO_CODR);            // starting timer to minimize
  timeValue = &(TC1->TC_CHANNEL[0].TC_CV);  // the initial 'while'.
  timeReset = &(TC1->TC_CHANNEL[0].TC_CCR);
  p         =  pixels;
  end       =  p + numBytes;
  pix       = *p++;
  mask      = 0x80;

  time0 = TIME_800_0;
  time1 = TIME_800_1;
  period = PERIOD_800;

  int index = 0;
  while(index < 450)  
  {              
    uint8_t colourValue = pixels[index];
    uint8_t bitMask = 0x80;
    while(bitMask)
    {    
      // set outputs high
      *portSet   = pinMaskA;
      *portSet   = pinMaskB;
      *portSet   = pinMaskC;
  
      // reset the timer
      *timeReset = TC_CCR_CLKEN | TC_CCR_SWTRG;
  
      // get the high cycle time
      t = time0;
      if (colourValue & bitMask) 
        t = time1;    
      
      // Wait until the high time period has elapsed
      while (*timeValue < t);
      
      // Set outputs low
      *portClear = pinMaskA;
      *portClear = pinMaskB;
      *portClear = pinMaskC;
      
      // shift the bit mask
      bitMask >>= 1;

      // Wait until the low time has elapsed
      while (*timeValue < period);    
    }
    
    // next byte;
    ++index;
    
  }
  TC_Stop(TC1, 0);

  interrupts();
  endTime = micros(); // Save EOD time for latch on next call
}

// Set the output pin number
void Adafruit_NeoPixel::setPin(uint8_t p) {
  pinMode(pin, INPUT);
  pin = p;
  pinMode(p, OUTPUT);
  digitalWrite(p, LOW);
}

// Set pixel color from separate R,G,B components:
void Adafruit_NeoPixel::setPixelColor(
  uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if (n < numLEDs) {
    if (brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p = &pixels[n * 3];
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void Adafruit_NeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if (n < numLEDs) {
    uint8_t
    r = (uint8_t)(c >> 16),
    g = (uint8_t)(c >>  8),
    b = (uint8_t)c;
    if (brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p = &pixels[n * 3];
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t Adafruit_NeoPixel::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Adafruit_NeoPixel::getPixelColor(uint16_t n) const {

  if (n < numLEDs) {
    uint8_t *p = &pixels[n * 3];
    return ((uint32_t)p[rOffset] << 16) |
           ((uint32_t)p[gOffset] <<  8) |
           (uint32_t)p[bOffset];
  }

  return 0; // Pixel # is out of bounds
}

// Returns pointer to pixels[] array.  Pixel data is stored in device-
// native format and is not translated here.  Application will need to be
// aware whether pixels are RGB vs. GRB and handle colors appropriately.
uint8_t *Adafruit_NeoPixel::getPixels(void) const {
  return pixels;
}

uint16_t Adafruit_NeoPixel::numPixels(void) const {
  return numLEDs;
}

// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.  C'est la vie.
void Adafruit_NeoPixel::setBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  if (newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
             *ptr           = pixels,
              oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if (oldBrightness == 0) scale = 0; // Avoid /0
    else if (b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for (uint16_t i = 0; i < numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}




#define PIN 6
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
       
//       pixels[1+(3*(int)p->y)] = p->i;
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
  pinMode(5,OUTPUT);
  pinMode(7,OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
// initBlobs();
}
//int puckY=0;
void loop() {
  
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

