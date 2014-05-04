//////////////////////////////////////////////////////////////////////////
//
// Input Listener
// Code for PIC16F1825
// SourceBoost C
// hotchk155/2014
//
// Monitors two analog inputs, sending MIDI note triggers with velocity
// depending on highest input level after a threshold is crossed.
//
//////////////////////////////////////////////////////////////////////////

#include <system.h>

typedef unsigned char byte;

// PIC CONFIG
#pragma DATA _CONFIG1, _FOSC_INTOSC & _WDTE_OFF & _MCLRE_OFF &_CLKOUTEN_OFF
#pragma DATA _CONFIG2, _WRT_OFF & _PLLEN_OFF & _STVREN_ON & _BORV_19 & _LVP_OFF
#pragma CLOCK_FREQ 16000000

#define ADC_AQUISITION_DELAY 		20		// clock ticks on timer2 to allow for input settling
											// after switching to a different analog input port

#define TIMER_0_INIT_SCALAR			5		// Timer 0 is an 8 bit timer counting at 250kHz
											// using this init scalar means that rollover
											// interrupt fires once per ms

#define P_HEARTBEAT 				portc.0	// Hearbeat LED connection
#define HEARTBEAT_PERIOD			500		// Milliseconds between toggles of heartbeat LED

#define INPUT_TRIGGER_THRESHOLD 	100		// lowest ADC level that is considered a trigger
#define INPUT_SAMPLE_TIME			50		// milliseconds period after triggering where we sample for highest level
#define INPUT_DEAD_TIME				300		// milliseconds period to wait after sending note message before listening again
#define INPUT_MAX_LEVEL				750		// LM358 op amp max output on 5V = ~3.8V
											// 1024 * (3.8 / 5.0) - 1 = 777. Reduced to ensure highest reading is possible
										
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

// ADC states
enum {
	ADC_CONNECT,							// connect to a new analog input
	ADC_ACQUIRE,							// connected to a new analog input, allowing settling time
	ADC_CONVERT								// waiting for conversion to complete
};

// Input state machine states
enum {
	INPUT_LISTENING,	// Ready to be triggered
	INPUT_SAMPLING,		// Triggered, reading max input level
	INPUT_BLOCKED		// Event has been reported, now we hold off for a bit
};

// Information associated with each input
typedef struct 
{
	byte adcInputMask;			// PIC analog input port mask
	byte midiChannel;			// MIDI channel on which to notify this trigger
	byte midiNote;				// MIDI note on which to notify this trigger
	byte ledBit;				// LED bit on port C
	byte state;					// Input state
	unsigned int thisMaxInput;	// Max ADC input on this trigger cycle
	unsigned long timeout;		// Timeout at which to move to next state
} INPUT_PORT;

// Set of inputs
#define NUM_INPUT_PORTS 2					// number of inputs to define
INPUT_PORT inputPort[NUM_INPUT_PORTS];		// input port data
byte whichInputPort = 0;					// iterator of input ports
int adcState = ADC_CONNECT;					// master adc state machine state
volatile unsigned long milliseconds = 0;	// milliseconds counter updated by timer 0
volatile byte millisecondsRollover = 0;		// flag to say if milliseconds have rolled over

////////////////////////////////////////////////////////////
// INTERRUPT HANDLER 
// Used to maintain a millisecond clock
void interrupt( void )
{
	// timer 0 rollover ISR. Counts ms ticks
	if(intcon.2)
	{
		tmr0 = TIMER_0_INIT_SCALAR;
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
void initInput(INPUT_PORT *pInput, byte inputMask, byte chan, byte note, byte ledBit)
{
	pInput->adcInputMask = inputMask;
	pInput->midiChannel = chan;
	pInput->midiNote = note;
	pInput->ledBit = ledBit;
	pInput->state = INPUT_LISTENING;
	pInput->timeout = 0;
}

////////////////////////////////////////////////////////////
// INPUT PORT ADC DATA HANDLER
void handleAdcResult(INPUT_PORT *pInput, unsigned int adcResult)
{
	switch(pInput->state)
	{
	// waiting for another trigger
	case INPUT_LISTENING:	
		if(adcResult >= INPUT_TRIGGER_THRESHOLD)
		{	
			// crossed threshold, start the trigger cycle
			pInput->thisMaxInput = adcResult;		
			pInput->timeout = milliseconds + INPUT_SAMPLE_TIME;
			pInput->state = INPUT_SAMPLING;
		}	
		break;
		
	// input has been triggered, now we listen for the sampling period,
	// taking and using the highest input reading
	case INPUT_SAMPLING:
		if(adcResult > pInput->thisMaxInput)
			pInput->thisMaxInput = adcResult;
		if(milliseconds >= pInput->timeout)		
		{
			// end of sampling - calculate the MIDI velocity for the trigger
			int velocity = (127 * (long)pInput->thisMaxInput) / INPUT_MAX_LEVEL;
			if(velocity > 127)
				velocity = 127;
			if(velocity <= 0)
			{
				// too low, ignore it
				pInput->state = INPUT_LISTENING;				
			}
			else
			{			
				// play the MIDI note
				midiNote(pInput->midiChannel, pInput->midiNote, (byte)velocity);
				
				// turn on the LED
				portc |= pInput->ledBit;
				
				// set up dead time			
				pInput->timeout = milliseconds + INPUT_DEAD_TIME;
				pInput->state = INPUT_BLOCKED;
			}
		}		
		break;
		
	// Holding off after a trigger, we block for a specified period of time
	// before going back to listening for another trigger
	case INPUT_BLOCKED:
		if(milliseconds >= pInput->timeout)		
		{
			midiNote(pInput->midiChannel, pInput->midiNote, 0);
			pInput->state = INPUT_LISTENING;
			portc &= ~pInput->ledBit;
		}
		break;
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

////////////////////////////////////////////////////////////
// MAIN 
void main()
{ 
	int i;
	unsigned long nextHeartbeat = 0;
	
	// osc control / 16MHz / internal
	osccon = 0b01111010;
	
	// configure io
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
	t2con = 0b00000110; //fosc/16
	      	      
	// turn on the ADC
	adcon0=0b00000001; //AD on
	adcon1=0b11010000; //right justify, fosc/16, Vss
	
	// initialise MIDI comms
	init_usart();

	// initialise the inputs
	initInput(&inputPort[0], ANA_3, 0, 60, (1<<1));
	initInput(&inputPort[1], ANA_7, 0, 61, (1<<2));
	adcState = ADC_CONNECT;
	whichInputPort = 0;					
	
	// forever...
	for(;;)
	{
		// Toggle the heartbeat
		if(milliseconds >= nextHeartbeat)
		{
			P_HEARTBEAT = !P_HEARTBEAT;
			nextHeartbeat = milliseconds + HEARTBEAT_PERIOD;
		}

		// Check if the millisecond counter has rolled over
		if(millisecondsRollover)
		{			
			// Ensure that we are not waiting on a ms timeout
			// since this could block until the timer rolls again!
			for(int i = 0; i<NUM_INPUT_PORTS; ++i)
				inputPort[i].state = INPUT_LISTENING;
			nextHeartbeat = 0;
			millisecondsRollover = 0;
		}
		
		// Run the input state machine
		runInputs();
	}
}




