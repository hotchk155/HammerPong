#include <system.h>
#include <memory.h>

#define ANA_0 0b00000000
#define ANA_1 0b00000100
#define ANA_2 0b00001000
#define ANA_3 0b00001100
#define ANA_4 0b00010000
#define ANA_5 0b00010100
#define ANA_6 0b00011000
#define ANA_7 0b00011100
#define ADC_MAX	6

#define P_HEARTBEAT 	porta.5

#pragma DATA _CONFIG, _MCLRE_OFF&_WDT_OFF&_INTRC_OSC_NOCLKOUT
#pragma CLOCK_FREQ 8000000
#define ADC_AQUISITION_DELAY 10

typedef unsigned char byte;

enum {
	ADC_CONNECT,
	ADC_ACQUIRE,
	ADC_CONVERT				
};

void init_usart()
{
	pir1.1 = 1;	//TXIF transmit enable
	pie1.1 = 0;	//TXIE no interrupts
	
	baudctl.4 = 0;		// synchronous bit polarity 
	baudctl.3 = 1;		// enable 16 bit brg
	baudctl.1 = 0;		// wake up enable off
	baudctl.0 = 0;		// disable auto baud detect
		
	txsta.6 = 0;	// 8 bit transmission
	txsta.5 = 1;	// transmit enable
	txsta.4 = 0;	// async mode
	txsta.2 = 0;	// high baudrate BRGH

	rcsta.7 = 1;	// serial port enable
	rcsta.6 = 0;	// 8 bit operation
	rcsta.4 = 0;	// enable receiver
		
	spbrgh = 0;		// brg high byte
	spbrg = 15;		// brg low byte (31250)	
}
		
void send(unsigned char c)
{
	txreg = c;
	while(!txsta.1);
}

void sendController(byte channel, byte controller, byte value)
{
	P_HEARTBEAT = 1;
	send(0xb0 | channel);
	send(controller&0x7f);
	send(value&0x7f);
	P_HEARTBEAT = 0;	
}

byte adcInput[ADC_MAX] = {ANA_2, ANA_3, ANA_4, ANA_5, ANA_6, ANA_7};
byte adcInitComplete = 0;
int adcResult[ADC_MAX] = {-1,-1,-1,-1,-1,-1};
int adcIndex = 0;
int adcState = ADC_CONNECT;

////////////////////////////////////////////////////////////////
// 
// doADC
//
// State machine for running the ADC and updating the adcResult
// array with the result from each analog input
//
void doADC()
{
	switch(adcState)
	{
		// Connect ADC to the correct analog input
		case ADC_CONNECT:			
			adcon0=0b10000001 | adcInput[adcIndex];
			tmr0 = 0;
			adcState = ADC_ACQUIRE;
			// fall through
			
		// Waiting for a delay while the ADC input settles
		case ADC_ACQUIRE:
			if(tmr0 > ADC_AQUISITION_DELAY)
			{
				// Start the conversion
				adcon0.1=1;
				adcState = ADC_CONVERT;				
			}
			break;
			
		// Waiting for the conversion to complete
		case ADC_CONVERT:
			if(!adcon0.1)
			{
				// store the result
				adcResult[adcIndex] = (((int)adresh)<<8)|adresl;				
				
				// and prepare for the next ADC
				if(++adcIndex>=ADC_MAX)
				{
					adcIndex = 0;
					adcInitComplete = 1;
				}
				adcState = ADC_CONNECT;				
			}
			break;
	}			
}


#define BUFLEN	8
typedef struct 	
{
  char midiChannel;
  char midiController;  
  int minADC;
  int maxADC;
  char currentValue;  
  char history[BUFLEN];
} CONTROLLER;

CONTROLLER controllers[ADC_MAX] = {0};

void initInput(int which, byte channel, byte controller)
{
	controllers[which].midiChannel = channel;
	controllers[which].midiController = controller;
	controllers[which].minADC = -1;
	controllers[which].maxADC = -1;
	controllers[which].currentValue = -1;
}

void checkInput(int which)
{
   // pointer to the controllers
  CONTROLLER *p = &controllers[which];
  
  // read the raw analog value 0-1023
  int adc = adcResult[which];

  // remember highest and lowest values
  if((p->minADC == -1) || (p->minADC > adc))
      p->minADC = adc;
  if((p->maxADC == -1) || (p->maxADC < adc))
      p->maxADC = adc;
         
  // get the range of known readings
  int range = p->maxADC - p->minADC;     
  if(range < 1)
     range = 1;
         
  // scale the current value into the range
  // NB no floating point support...
  int newValue = (127*(adc - p->minADC))/range;
              
  // add the value into the history buffer
  long smoothed = 0;
  for(int j=0; j<BUFLEN-1;++j)
  {
    p->history[j] = p->history[j+1];
    smoothed += p->history[j];
  }
  p->history[BUFLEN-1] = newValue;
  smoothed += newValue;
  smoothed /= BUFLEN;
              
  // has the value changed?
  if(smoothed != p->currentValue)
  {
	 sendController(p->midiChannel, p->midiController, smoothed);
     p->currentValue= smoothed;
  }
}

void main()
{ 
	int i;
	
	// osc control / 8MHz / internal
	osccon = 0b01110001;
	
	// timer0... configure source and prescaler
	option_reg = 0b10000011;
	cmcon0 = 7;                      
	
	// configure io
	trisa = 0b00001010;              	
    trisc = 0b00001111;              
	ansel = 0b11111100;
      
	// turn on the ADC
	adcon1=0b00100000; //fOSC/32
	adcon0=0b10000001; // Right justify / Vdd / AD on
	
	// initialise MIDI comms
	init_usart();
	
	// Initialise the controllers
	initInput(0, 0, 1);
	initInput(1, 0, 2);
	initInput(2, 0, 4);
	initInput(3, 0, 7);
	initInput(4, 0, 11);
	initInput(5, 0, 74);

	adcInitComplete = 0;
	for(;;)
	{
		doADC();
		if(adcInitComplete)
		{			
			for(i=0;i<ADC_MAX;++i)
				checkInput(i);
			adcInitComplete = 0;			
			delay_ms(20);
		}
	}
}




