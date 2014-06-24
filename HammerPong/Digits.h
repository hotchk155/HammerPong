#ifndef __DIGITS_H__
#define __DIGITS_H__

////////////////////////////////////////////////////////////////////////
#define P_DIGITS_DAT1  2
#define P_DIGITS_DAT2  6
#define P_DIGITS_OE1   3
#define P_DIGITS_OE2   7 
#define P_DIGITS_SH    5
#define P_DIGITS_ST    4

////////////////////////////////////////////////////////////////////////
enum {
  SEG_A = 0x01,
  SEG_B = 0x02,
  SEG_C = 0x04,
  SEG_D = 0x08,
  SEG_E = 0x10,
  SEG_F = 0x20,
  SEG_G = 0x40
};

////////////////////////////////////////////////////////////////////////
//
//    AAA
//   F   B
//    GGG
//   E   C
//    DDD
//
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

////////////////////////////////////////////////////////////////////////
void digitsSetup();
void digitsSetCounter(int n);
void digitsSetLeft(int left);
void digitsSetRight(int right);
void digitsSetBoth(int left, int right);
void digitsSetBrightness(byte left, byte right);
void digitsSetRaw(int left, int right);


#endif //__DIGITS_H__
