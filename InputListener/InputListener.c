#include <system.h>
#include <memory.h>


#pragma DATA _CONFIG1, _FOSC_INTOSC & _WDTE_OFF & _MCLRE_OFF &_CLKOUTEN_OFF
#pragma DATA _CONFIG2, _WRT_OFF & _PLLEN_OFF & _STVREN_ON & _BORV_19 & _LVP_OFF
#pragma CLOCK_FREQ 16000000

#define ADC_AQUISITION_DELAY 10

#define TIMER_2_INIT_SCALAR		5	// Timer 2 is an 8 bit timer counting at 250kHz
									// using this init scalar means that rollover
									// interrupt fires once per ms

#define P_TRIG 		portc.1
#define P_HEARTBEAT 		portc.2
#define NUM_INPUT_PORTS 	1
#define TRIGGER_THRESHOLD 	200		// lowest ADC level that is considered a trigger
#define INPUT_SAMPLE_TIME	30			// milliseconds
#define INPUT_DEAD_TIME		300			// milliseconds

typedef unsigned char byte;

// ADC states
enum {
	ADC_CONNECT,
	ADC_ACQUIRE,
	ADC_CONVERT				
};

// Enumerate the control bit combinations for
// reading each analog input port 
enum {
	ANA_0=(byte)0b00000000,
	ANA_1=(byte)0b00000100,
	ANA_2=(byte)0b00001000,
	ANA_3=(byte)0b00001100,
	ANA_4=(byte)0b00010000,
	ANA_5=(byte)0b00010100,
	ANA_6=(byte)0b00011000,
	ANA_7=(byte)0b00011100
};

////////////////////////////////////////////////////////////
// INPUT TRIGGER STRUCTURE
typedef struct 
{
	byte adcInputMask;		// PIC analog input port mask
	byte midiChannel;		// MIDI channel on which to notify this trigger
	byte midiNote;			// MIDI note on which to notify this trigger
	int triggerThreshold;	// Lowest valid ADC level
	int maxLevel;			// Highest valid ADC level
	int thisMaxInput;
	unsigned long endOfSampleTime;	// End of sample time period
	unsigned long endOfDeadTime;	// End of dead time period
} INPUT_PORT;


INPUT_PORT inputPort[NUM_INPUT_PORTS];		// input port data
byte whichInputPort = 0;					// iterator of input ports
int adcState = ADC_CONNECT;					// master adc state machine state
volatile unsigned long milliseconds = 0;	// milliseconds counter
volatile byte millisecondsRollover = 0;		// flag to say if milliseconds have rolled over


////////////////////////////////////////////////////////////
// INTERRUPT HANDLER 
// Used to maintain a millisecond clock
void interrupt( void )
{
	// timer 0 rollover ISR. Counts ms ticks
	if(intcon.2)
	{
		tmr0 = TIMER_2_INIT_SCALAR;
		++milliseconds;
		if(!++milliseconds)
			millisecondsRollover = 1; // make sure we know if the ms counter overflows
		intcon.2 = 0;
	}
}

////////////////////////////////////////////////////////////
// INIT SERIAL PORT FOR MIDI XMIT
void init_usart()
{
	pir1.1 = 0;		//TXIF 		
	pir1.5 = 0;		//RCIF
	
	pie1.1 = 0;		//TXIE 		no interrupts
	pie1.5 = 0;		//RCIE 		enable
	
	baudcon.4 = 0;	// SCKP		synchronous bit polarity 
	baudcon.3 = 1;	// BRG16	enable 16 bit brg
	baudcon.1 = 0;	// WUE		wake up enable off
	baudcon.0 = 0;	// ABDEN	auto baud detect
		
	txsta.6 = 0;	// TX9		8 bit transmission
	txsta.5 = 1;	// TXEN		transmit enable
	txsta.4 = 0;	// SYNC		async mode
	txsta.3 = 0;	// SEDNB	break character
	txsta.2 = 0;	// BRGH		high baudrate 
	txsta.0 = 0;	// TX9D		bit 9

	rcsta.7 = 1;	// SPEN 	serial port enable
	rcsta.6 = 0;	// RX9 		8 bit operation
	rcsta.5 = 0;	// SREN 	enable receiver
	rcsta.4 = 0;	// CREN 	continuous receive enable
		
	spbrgh = 0;		// brg high byte
	spbrg = 31;		// brg low byte (31250)	
}
		
////////////////////////////////////////////////////////////
// FUNCTION TO SEND A CHARACTER TO UART
void send(unsigned char c)
{
	txreg = c;
	while(!txsta.1);
}

////////////////////////////////////////////////////////////
// SEND MIDI NOTE MESSAGE
void midiNote(byte chan, byte note, byte vel)
{
	send(0x90|(chan&0x0F));
	send(note&0x7f);
	send(vel&0x7f);
}

////////////////////////////////////////////////////////////
// INITIALISE PORT DATA
void initInput(INPUT_PORT *pInput, byte inputMask, byte chan, byte note)
{
	pInput->adcInputMask = inputMask;
	pInput->midiChannel = chan;
	pInput->midiNote = note;
	pInput->triggerThreshold = TRIGGER_THRESHOLD;
	pInput->maxLevel = 1023;
	pInput->endOfDeadTime = 0;
	pInput->endOfSampleTime = 0;
}

////////////////////////////////////////////////////////////
// INPUT PORT ADC DATA HANDLER
void handleAdcResult(INPUT_PORT *pInput, int adcResult)
{

	// are we waiting following previous trigger?
	if(pInput->endOfSampleTime) 
	{
		if(adcResult > pInput->thisMaxInput)
			pInput->thisMaxInput = adcResult;
		if(milliseconds >= pInput->endOfSampleTime)		
		{
			// calculate the MIDI velocity for the hit
			long velocity = 127 * (pInput->thisMaxInput - pInput->triggerThreshold);
			velocity /= (pInput->maxLevel - pInput->triggerThreshold);
			if(velocity > 127)
				velocity = 127;
			if(velocity > 0)
			{
				// play the MIDI note
				midiNote(pInput->midiChannel, pInput->midiNote, (byte)velocity);
				P_TRIG = 1;
			
			}			
			
			// set up dead time			
			pInput->endOfSampleTime = 0;
			pInput->endOfDeadTime = milliseconds + INPUT_DEAD_TIME;
		}		
	}
	else if(pInput->endOfDeadTime) 
	{
		// if we have waited long enough then clear the dead time
		if(milliseconds > pInput->endOfDeadTime)
		{
			midiNote(pInput->midiChannel, pInput->midiNote, 0);
			pInput->endOfDeadTime = 0;
			P_TRIG = 0;
		}
	}
	// otherwise see if the input is above the trigger threshold
	else if(adcResult >= pInput->triggerThreshold)
	{	
		pInput->endOfSampleTime = milliseconds + INPUT_SAMPLE_TIME;
		pInput->endOfDeadTime = 0;
		pInput->thisMaxInput = adcResult;		
	}	
}

////////////////////////////////////////////////////////////
// RUN THE INPUT STATE MACHINE
void runInputs()
{
	switch(adcState)
	{
		// Connect ADC to the next analog input
		case ADC_CONNECT:			
			adcon0=0b00000001 | inputPort[whichInputPort].adcInputMask;
			tmr2 = 0;
			adcState = ADC_ACQUIRE;
			break;
			
		// Waiting for a delay while the ADC input settles
		case ADC_ACQUIRE:
			if(tmr2 > ADC_AQUISITION_DELAY) // allowing settling time
			{
				// Start the conversion
				adcon0.1=1;
				adcState = ADC_CONVERT;				
			}
			break;
			
		// Waiting for the conversion to complete
		case ADC_CONVERT:
			if(!adcon0.1) // Conversion complete
			{
				// notify the result				
				handleAdcResult(&inputPort[whichInputPort], (((int)adresh)<<8)|adresl);				
				
				// and prepare for the next port
				if(++whichInputPort>=NUM_INPUT_PORTS)
					whichInputPort=0;
				adcState = ADC_CONNECT;				
			}
			break;
	}			
}


void main()
{ 
	int i;
	
	// osc control / 16MHz / internal
	osccon = 0b01111010;
	
	// configure io
	//apfcon0.7=0;  // usart rx on rc5
	//apfcon0.2=0;  // usart tx on rc4
	trisa =  0b00010111;              	
    trisc =  0b00111000;              
	ansela = 0b00010111;
	anselc = 0b00000000;
	porta=0;
	portc=0;

	
	// Configure timer 0 (controls millisecond counter)
	// 	timer 0 runs at 4MHz
	// 	prescaled 1/16 = 250kHz
	// 	rollover at 250 = 1kHz
	// 	1ms per rollover	
	option_reg.5 = 0; // timer 0 driven from instruction cycle clock
	option_reg.3 = 0; // timer 0 is prescaled
	option_reg.2 = 0; // }
	option_reg.1 = 1; // } 1/16 prescaler
	option_reg.0 = 1; // }
	intcon.5 = 1; 	  // enabled timer 0 interrrupt
	intcon.2 = 0;     // clear interrupt fired flag
	intcon.7 = 1;	  // GIE	
	intcon.6 = 1;	  // PEIE
	
	// Configure timer 2 (used to time ADC acquisition)
	t2con = 0b00000110;
	      	      
	// turn on the ADC
	adcon0=0b00000001; //AD on
	adcon1=0b10100000; //right justify, fosc/32, Vss
	
	// initialise MIDI comms
	init_usart();

	initInput(&inputPort[0], ANA_3, 0, 60);
	adcState = ADC_CONNECT;
	whichInputPort = 0;					

	P_TRIG=0;
	
	// main loop
	for(;;)
	{
		P_HEARTBEAT = !!(milliseconds & 0x100);

		if(millisecondsRollover)
		{			
			// ensure that if there is a roll over of the millisecond
			// counter then inputs cannot get jammed into their dead
			// time
			for(int i = 0; i<NUM_INPUT_PORTS; ++i)
			{
				inputPort[i].endOfSampleTime = 0;
				inputPort[i].endOfDeadTime = 0;
			}
			millisecondsRollover = 0;
		}
		
		// process ADC inputs
		runInputs();
	}
}




