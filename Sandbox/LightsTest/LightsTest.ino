#define P_DIGITS_DAT1  2
#define P_DIGITS_DAT2  6
#define P_DIGITS_OE1   3
#define P_DIGITS_OE2   7 
#define P_DIGITS_SH    5
#define P_DIGITS_ST    4

enum {
  SEG_A = 0x01,
  SEG_B = 0x02,
  SEG_C = 0x04,
  SEG_D = 0x08,
  SEG_E = 0x10,
  SEG_F = 0x20,
  SEG_G = 0x40
};

//    AAA
//   F   B
//    GGG
//   E   C
//    DDD
enum {
  DIGIT_0 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F,
  DIGIT_1 = SEG_B|SEG_C,
  DIGIT_2 = SEG_A|SEG_B|SEG_D|SEG_E|SEG_G,
  DIGIT_3 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_G,
  DIGIT_4 = SEG_B|SEG_C|SEG_F|SEG_G,
  DIGIT_5 = SEG_A|SEG_C|SEG_D|SEG_F|SEG_G,
  DIGIT_6 = SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
  DIGIT_7 = SEG_A|SEG_B|SEG_C,
  DIGIT_8 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
  DIGIT_9 = SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G
};

byte digitsStateLeft = 0;
byte digitsStateRight = 0;
byte digitsNeedUpdate = 1;

void digitsSetup()
{
  pinMode(P_DIGITS_DAT1, OUTPUT);
  pinMode(P_DIGITS_DAT2, OUTPUT);
  pinMode(P_DIGITS_OE1, OUTPUT);  
  pinMode(P_DIGITS_OE2, OUTPUT);
  pinMode(P_DIGITS_SH, OUTPUT);
  pinMode(P_DIGITS_ST, OUTPUT);
  
  digitalWrite(P_DIGITS_DAT1, LOW);
  digitalWrite(P_DIGITS_DAT2, LOW);
  digitalWrite(P_DIGITS_SH, LOW);
  digitalWrite(P_DIGITS_ST, LOW);
  digitalWrite(P_DIGITS_OE1, LOW);
  digitalWrite(P_DIGITS_OE2, LOW);
}
void digitsRefresh()
{
  byte mask = 0x80;
  digitalWrite(P_DIGITS_ST, LOW);
  while(mask)
  {
    digitalWrite(P_DIGITS_SH, LOW);
    digitalWrite(P_DIGITS_DAT1, (digitsStateLeft & mask)?HIGH:LOW);
    digitalWrite(P_DIGITS_DAT2, (digitsStateRight & mask)?HIGH:LOW);    
    delay(1);
    digitalWrite(P_DIGITS_SH, HIGH);
    mask>>=1;
  }
  digitalWrite(P_DIGITS_ST, HIGH);
}
void digitsSet(unsigned int left, unsigned int right)
{
  digitsStateLeft = left;
  digitsStateRight = right;
  digitsNeedUpdate = 1;
}
void digitsShow(byte pos, int n)
{
  byte &buf=pos? digitsStateRight : digitsStateLeft;
  switch(n)
  {
    case 0: buf=DIGIT_0; break;
    case 1: buf=DIGIT_1; break;
    case 2: buf=DIGIT_2; break;
    case 3: buf=DIGIT_3; break;
    case 4: buf=DIGIT_4; break;
    case 5: buf=DIGIT_5; break;
    case 6: buf=DIGIT_6; break;
    case 7: buf=DIGIT_7; break;
    case 8: buf=DIGIT_8; break;
    case 9: buf=DIGIT_9; break;
    default: buf=0; break;
  }
  digitsNeedUpdate=1;
}
void digitsShowNumber(int n)
{
  digitsShow(0, n/10);
  digitsShow(1, n%10);
}
unsigned int digitsNextEvent = 0;
int c=0;
void digitsRun(unsigned long milliseconds)
{
  if(milliseconds > digitsNextEvent)
  {
    digitsShowNumber(c++);
    digitsNextEvent = milliseconds + 1000;
  }
  if(digitsNeedUpdate)
  {
    digitsRefresh();
    digitsNeedUpdate=0;
  }
}





#define P_LIGHTS_DAT_2  12
#define P_LIGHTS_SH     11
#define P_LIGHTS_ST     10
#define P_LIGHTS_OE     9
#define P_LIGHTS_DAT_1  8
enum {
  LIGHTS_L_V0 = 0x0002,
  LIGHTS_L_V1 = 0x0004,
  LIGHTS_L_V2 = 0x0008,
  LIGHTS_L_V3 = 0x0010,
  LIGHTS_L_V4 = 0x0020,
  LIGHTS_L_V5 = 0x0040,
  LIGHTS_L_V6 = 0x0080,
  
  LIGHTS_L_H0 = 0x1000,
  LIGHTS_L_H1 = 0x0800,
  LIGHTS_L_H2 = 0x0400,
  LIGHTS_L_H3 = 0x0200,
  LIGHTS_L_H4 = 0x0100,
  
  LIGHTS_R_V6 = 0x0100,
  LIGHTS_R_V5 = 0x0200,
  LIGHTS_R_V4 = 0x0400,
  LIGHTS_R_V3 = 0x0800,
  LIGHTS_R_V2 = 0x1000,
  LIGHTS_R_V1 = 0x2000,
  LIGHTS_R_V0 = 0x4000,
  
  LIGHTS_R_H0 = 0x0008,
  LIGHTS_R_H1 = 0x0010,
  LIGHTS_R_H2 = 0x0020,
  LIGHTS_R_H3 = 0x0040,
  LIGHTS_R_H4 = 0x0080,
  
  LIGHTS_R_BUTTON = 0x0004

};

void lightsSetup()
{
  pinMode(P_LIGHTS_DAT_1, OUTPUT);
  pinMode(P_LIGHTS_DAT_2, OUTPUT);
  pinMode(P_LIGHTS_SH, OUTPUT);
  pinMode(P_LIGHTS_ST, OUTPUT);
  pinMode(P_LIGHTS_OE, OUTPUT);
  digitalWrite(P_LIGHTS_OE, LOW);
}
unsigned int lightsStateLeft = 0;
unsigned int lightsStateRight = 0;
byte lightsNeedUpdate = 0;
byte lightsBrightness = 255;
int lightsAnimState = 0;
unsigned long lightsNextAnim = 0;
void lightsRefresh()
{
  unsigned int mask = 0x8000;
  digitalWrite(P_LIGHTS_ST, LOW);
  while(mask)
  {
    digitalWrite(P_LIGHTS_SH, LOW);
    digitalWrite(P_LIGHTS_DAT_1, (lightsStateLeft & mask)?HIGH:LOW);
    digitalWrite(P_LIGHTS_DAT_2, (lightsStateRight & mask)?HIGH:LOW);    
    delay(1);
    digitalWrite(P_LIGHTS_SH, HIGH);
    mask>>=1;
  }
  digitalWrite(P_LIGHTS_ST, HIGH);
}
void lightsSet(unsigned int left, unsigned int right)
{
  lightsStateLeft = left;
  lightsStateRight = right;
  lightsNeedUpdate = 1;
}
void lightsSetSymmetrical(unsigned int d)
{
  unsigned int left = 0;
  unsigned int right = 0;
  unsigned int mask = 1;
  for(int i=0; i<=12; ++i)
  {
    if(d&mask)
    {
      switch(i)
      {
        case 0:  left|=LIGHTS_L_V0; right|=LIGHTS_R_V0; break;
        case 1:  left|=LIGHTS_L_V1; right|=LIGHTS_R_V1; break;
        case 2:  left|=LIGHTS_L_V2; right|=LIGHTS_R_V2; break;
        case 3:  left|=LIGHTS_L_V3; right|=LIGHTS_R_V3; break;
        case 4:  left|=LIGHTS_L_V4; right|=LIGHTS_R_V4; break;
        case 5:  left|=LIGHTS_L_V5; right|=LIGHTS_R_V5; break;
        case 6:  left|=LIGHTS_L_V6; right|=LIGHTS_R_V6; break;
        case 7:  left|=LIGHTS_L_H0; right|=LIGHTS_R_H0; break;
        case 8:  left|=LIGHTS_L_H1; right|=LIGHTS_R_H1; break;
        case 9:  left|=LIGHTS_L_H2; right|=LIGHTS_R_H2; break;
        case 10: left|=LIGHTS_L_H3; right|=LIGHTS_R_H3; break;
        case 11: left|=LIGHTS_L_H4; right|=LIGHTS_R_H4; break;
        case 12: right|=LIGHTS_R_BUTTON; break;
      }
    }
    
    mask <<=1;
  }
  lightsSet(left,right);
}
void lightsAnim1()
{
  switch(lightsAnimState%3)
  {
    case 0: lightsSetSymmetrical(0b1001001001001); break;
    case 1: lightsSetSymmetrical(0b1010010010010); break;
    case 2: lightsSetSymmetrical(0b1100100100100); break;
  } 
  lightsAnimState++;
}
void lightsRun(unsigned long milliseconds)
{
//  lightsSet(LIGHTS_L_V0,LIGHTS_R_V0);
  if(milliseconds >= lightsNextAnim)
  {
    lightsAnim1();
//  lightsSetSymmetrical(4096);
    lightsNextAnim = milliseconds + 100;
  }
  if(lightsNeedUpdate)
  {
    lightsRefresh();
    lightsNeedUpdate=0;
  }
  analogWrite(P_LIGHTS_OE, 255-lightsBrightness);
}
void setup() {
  lightsSetup();
  digitsSetup();

}


void loop() {
  unsigned int milliseconds = millis();
  lightsRun(milliseconds);
  digitsRun(milliseconds);
}
