#ifndef __LIGHTS_H__
#define __LIGHTS_H__

////////////////////////////////////////////////////////////////////////
#define P_LIGHTS_DAT_2  12
#define P_LIGHTS_SH     11
#define P_LIGHTS_ST     10
#define P_LIGHTS_OE     9
#define P_LIGHTS_DAT_1  8

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
void lightsSetup();
void lightsSetSymmetrical(unsigned int d);
void lightsSetButton(byte d);
void lightsSetBrightness(byte d);
void lightsSetStack(int len);


#endif // __LIGHTS_H__
