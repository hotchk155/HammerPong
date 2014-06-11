#include <Arduino.h>

#define PIX_LENGTH 150
#define PIX_BUFSSIZE (3 * PIX_LENGTH)
#define PIX_NUMBUFFERS 3
byte pixBuffer[PIX_NUMBUFFERS][PIX_BUFSSIZE];

#define P_STRIP0 5
#define P_STRIP1 6
#define P_STRIP2 7
#define P_STRIP3 36
#define P_STRIP4 37
#define P_STRIP5 38
#define P_HEARTBEAT 13

#define SCALE      VARIANT_MCK / 2UL / 1000000UL
#define INST       (2UL * F_CPU / VARIANT_MCK)
#define TIME_800_0 ((int)(0.40 * SCALE + 0.5) - (5 * INST))
#define TIME_800_1 ((int)(0.80 * SCALE + 0.5) - (5 * INST))
#define PERIOD_800 ((int)(1.25 * SCALE + 0.5) - (5 * INST))


uint32_t endTime;
void pixClear()
{
  memset(pixBuffer, 0, PIX_NUMBUFFERS * PIX_BUFSSIZE);
}
void pixInit()
{
  pinMode(P_STRIP0, OUTPUT);
  pinMode(P_STRIP1, OUTPUT);
  pinMode(P_STRIP2, OUTPUT);
  pinMode(P_STRIP3, OUTPUT);
  pinMode(P_STRIP4, OUTPUT);
  pinMode(P_STRIP5, OUTPUT);

  pinMode(P_HEARTBEAT, OUTPUT);
  
  digitalWrite(P_STRIP0, LOW);
  digitalWrite(P_STRIP1, LOW);
  digitalWrite(P_STRIP2, LOW);
  digitalWrite(P_STRIP3, LOW);
  digitalWrite(P_STRIP4, LOW);
  digitalWrite(P_STRIP5, LOW);
  digitalWrite(P_HEARTBEAT, LOW);
  
  endTime = 0;
  pixClear();

}
void pixUpdate()
{

  while ((micros() - endTime) < 50L);
  noInterrupts(); // Need 100% focus on instruction timing



  pmc_set_writeprotect(false);
  pmc_enable_periph_clk((uint32_t)TC3_IRQn);
  TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP | TC_CMR_TCCLKS_TIMER_CLOCK1);
  TC_Start(TC1, 0);
int t0 = TIME_800_0;
int t1 = TIME_800_1;
int period = PERIOD_800;

  // get port address...
  // Note: all the pins need to be on same port reg
  Pio *port      = g_APinDescription[P_STRIP0].pPort; 
  
  // Get register addresses
  volatile WoReg *portSet   = &(port->PIO_SODR);            
  volatile WoReg *portClear = &(port->PIO_CODR);            
  volatile WoReg *timeValue = &(TC1->TC_CHANNEL[0].TC_CV);  
  volatile WoReg *timeReset = &(TC1->TC_CHANNEL[0].TC_CCR);


  int index = 0;
  int pinMask0 = g_APinDescription[P_STRIP0].ulPin; 
  int pinMask1 = g_APinDescription[P_STRIP1].ulPin; 
  int pinMask2 = g_APinDescription[P_STRIP2].ulPin; 
  int pinMask3 = g_APinDescription[P_STRIP3].ulPin; 
  int pinMask4 = g_APinDescription[P_STRIP4].ulPin; 
  int pinMask5 = g_APinDescription[P_STRIP5].ulPin; 

  int phase0 = pinMask0|pinMask1|pinMask2|pinMask3|pinMask4|pinMask5;
  while(index < PIX_BUFSSIZE)  
  {          
    // Fetch all six colour byte values
    byte colourValue0 = pixBuffer[0][index];
    byte colourValue1 = pixBuffer[1][index];
    byte colourValue2 = pixBuffer[2][index];
    byte colourValue3 = pixBuffer[3][index];
    byte colourValue4 = pixBuffer[4][index];
    byte colourValue5 = pixBuffer[5][index];

    uint8_t bitMask = 0x80;
    while(bitMask)
    {    
      int phase1 = phase0; // 111111
      if(colourValue0 & bitMask) phase1 ^= pinMask0; 
      if(colourValue1 & bitMask) phase1 ^= pinMask1; 
      if(colourValue2 & bitMask) phase1 ^= pinMask2; 
      if(colourValue3 & bitMask) phase1 ^= pinMask3; 
      if(colourValue4 & bitMask) phase1 ^= pinMask4; 
      if(colourValue5 & bitMask) phase1 ^= pinMask5; 
      int phase2 = phase0 ^ phase1; 
      
      while (*timeValue < period);    
      
      // set all outputs high
      *portSet   = phase0;
      *timeReset = TC_CCR_CLKEN | TC_CCR_SWTRG;
      while (*timeValue < t0);
      *portClear = phase1;
      while (*timeValue < t1);
      *portClear = phase2;
  
      // shift the bit mask
      bitMask >>= 1;
    }
    
    // next byte;
    ++index;
    
  }
  while (*timeValue < period);    
  TC_Stop(TC1, 0);

  interrupts();
  endTime = micros(); // Save EOD time for latch on next call
}

typedef struct 
{
  byte x;
  int  c;
  byte i;
  float y;
  float dy;
} BLOB;
#define MAX_BLOB 100
BLOB blobs[MAX_BLOB];
void initBlobs()
{
  memset(blobs,0,sizeof(blobs));
}
void runBlobs()
{
  for(int i=0; i<MAX_BLOB; ++i)
  {
    BLOB *p = &blobs[i];
    if(p->y <=0)
    {
      p->y = 149;
      p->dy = 0.1;
      p->x = i%3;
      p->c = random(1000);
      p->i = 20+random(235);
    }
    else if(!p->c)
    {
      p->y -= p->dy;
      p->dy += 0.0005;
//      if(p->i>0)
//        p->i--;
    }
    else
    {
        --p->c;
    }
  }
}

void renderBlobs()
{
  for(int i=0; i<MAX_BLOB; ++i)
  {
    BLOB *p = &blobs[i];
    if(p->y >= 0 && p->y < 149 )
      pixBuffer[p->x][1+(3*(int)p->y)] = p->i;
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

void puckRender(int y)
{

  int q=y;
  byte *p;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &pixBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[0][3*q];
    p[0] = p[1] = p[2] = 255;
    p = &pixBuffer[2][3*q];
    p[0] = p[1] = p[2] = 255;
  }
  q++;
  if(q>=0 && q<150)
  {
    p = &pixBuffer[1][3*q];
    p[0] = p[1] = p[2] = 255;
  }
}

void setup() {  
  initBlobs();
  pixInit();
}
//int puckY=0;
int q=0;
void loop() 
{
  //runBlobs();
  pixClear();
  puckRender(q);
  if(++q>=150) q=0;
//  renderBlobs();
  pixUpdate();
}

