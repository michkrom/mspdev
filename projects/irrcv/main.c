/**
 * IR receiver/decoder for MSP430
 * for PROTOCOL TracerJet RC codes
 *
 *
 * @author Michal Krombholz
 * @license GNU General Public License (GPL) v2 or later
 */

#include <msp430.h>

#define DCO1MHZ

#define  P_DIR     P1DIR
#define  P_OUT     P1OUT

// Input, this is the signal from the IR detector
#define IR_IN BIT4 // port1.4 is input

// Outputs, this is led 1 and 2 on the Launchpad
#define LED_1 BIT6 // port1.0 is our trigger led
#define LED_2 BIT0 // port1.6 is our status led	


//-------------------------- TIMER and TIMEBASE -------------------------------

volatile unsigned int timeCountH;

// getTimeUs returns 32bit time in [us] since start
// this would overflow every 4294.967296 seconds ~ about every 71.58 minutes (2^32*1E-6)
unsigned long getTimeUs()
{
	register unsigned int th;
	register unsigned int tl;
#if 1
	// read h & l and repeat if h changed (due to interrupt)
	do {
		th = timeCountH;
		tl = TAR;
	} while ( timeCountH != th );
#else
	// diable interrupts so we do not get interrupted when readting H & L part of timer
	__disable_interrupt();
	th = timeCountH;
	tl = TAR; // TAR = Timer A counter register
	__enable_interrupt();
#endif
	return (((unsigned long)th)<<16)|tl;
}

// Interrupt Service Routine for Timer A0. We need to use preprocessor
// directives in order to place the interrupt at the correct address for
// the current mcu.
// this interrupt fires every 65536us (ie then TA overflows)
// TA clock is 1MHz : from SMCLK = SCLK/8 and SCLK=16MHz and TA input divider = 2
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA()
{
	timeCountH ++;
	// show that we are alive - wiggle the LED 1 (green)
	// P1OUT ^= LED_1;
}


//------------------------------ IR decoder ----------------------------------

volatile unsigned long irReceivedCommand;

// start symbol + 32bit, IR burst is fixed @ 200us, space variable as per below
// symbol times
// start = 1600us
// long(0) = 800us
// short(1) = 400us
// repeats in ~170ms

// the symbols are timed from edge of IR to next edge of IR
// the timing is using just timer A counter (TAR 16bit@1us)
// the alg hinges on receiving and recognizing correct start signal
// but since TAR wraps around (every ~65.5ms) it is not ideal
// a possibility exist that wrapping would produce a delta time matching start by chance
// ideally I would use longer counter or reset TAR everytime IR is received
// another problem is when the train of symbols (bits) is not finished the alg
// would not know about it and could potentially join two separate trains (commands)
// again it is a (small) possiblity as the wrapping of timer may end up being a valid time delta
// ideally I would time out on receiving remaining bits and restart to wait for next start

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	// locals
	// temp to accumulate command bits
	static unsigned long irCommand;
	// current bit count - 
	// 0 marks reception of start; 
	// > IR_BIT_COUNT means not started or after error
	static int irBitCount;
	// previous edge timestamp
	static unsigned prev=0; // rounded to next 100us

	const int IR_BIT_COUNT = 32;
	// protocol timing
	typedef enum {
	    IR_SYMBOL_START = 2000,
	    IR_SYMBOL_LONG  = 1200,
	    IR_SYMBOL_SHORT = 800,
	    IR_SYMBOL_ERROR = 0
	} SYMBOL;
	const int TOLERANCE = 200;

	// measure time from prev rise to this rise
	unsigned now = TAR;
	unsigned delta = now > prev ? now-prev : 0xFFFF - (prev-now);
	prev = now;

	// classify received symbol based on timing since last edge
	SYMBOL symbol;
	if( delta > IR_SYMBOL_START-TOLERANCE && delta < IR_SYMBOL_START+TOLERANCE )
		symbol = IR_SYMBOL_START;
	else if( delta > IR_SYMBOL_LONG-TOLERANCE && delta < IR_SYMBOL_LONG+TOLERANCE )
		symbol = IR_SYMBOL_LONG;
	else if( delta > IR_SYMBOL_SHORT-TOLERANCE && delta < IR_SYMBOL_SHORT+TOLERANCE )
		symbol = IR_SYMBOL_SHORT;
	else
		symbol = IR_SYMBOL_ERROR;

	// act on received symbol
	switch(symbol) {
	case IR_SYMBOL_START: {
		// start interval
		irBitCount = 0;
		irCommand = 0;
		P1OUT &= ~LED_2;
		break;
	}
	case IR_SYMBOL_ERROR: {
		if( irBitCount <= IR_BIT_COUNT )
		{
			irBitCount = IR_BIT_COUNT+1; // mark invalid command
			P1OUT |= LED_2;
		}
		break;
	}
	default: { // it's a good bit!
		if( irBitCount <= IR_BIT_COUNT )
		{
			// shift command and count the bit
			irCommand <<= 1;
			irBitCount ++;
			if( symbol == IR_SYMBOL_SHORT ) {
				// short interval (bit 1)
				irCommand |= 1;
			}
		}
		break;
	}
	} // switch

	if( irBitCount == IR_BIT_COUNT ) {
		irReceivedCommand = irCommand;
		// mark end of command
		irBitCount = IR_BIT_COUNT+1;
	}

	// P1.3 IFG cleared to enable next interrupt
	P1IFG = 0;

}


//--------------------------------------------------------------------------


int main(void)
{

	// Disable the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Set ACLK to use internal VLO (12 kHz clock)
	BCSCTL3 |= LFXT1S_2;

#ifdef DCO1MHZ
	// confugure DCO to 1MHz
	BCSCTL1 = CALBC1_1MHZ;    // Set range
	DCOCTL = CALDCO_1MHZ;     // Set DCO step and modulation
#else
	// confugure DCO to 16MHz
	BCSCTL1 = CALBC1_16MHZ;    // Set range
	DCOCTL = CALDCO_16MHZ;     // Set DCO step and modulation
#endif

	BCSCTL2 = SELM_0;          // DCO->SCLK, SMCLK

	//Set TimerA to use auxiliary clock in UP mode
	//TACTL = TASSEL_1 | MC_1;

#ifndef DCO1MHZ
	// configure SMCLK to 2MHz (SCLK/8)
	BCSCTL2 |= DIVS_3;
#endif

	// TimerA

	// to SMCLK and CONTINOUS mode
	TACTL = TASSEL_2 | MC_2;

#ifndef DCO1MHZ
	// DIV/2 ==> SMCLK/2=1MHz
	TACL |= ID_1;
#endif

	//Enable the interrupt for TACCR0 match
	TACCTL0 = CCIE;
	// Set TACCR0 which also starts the timer.
	TACCR0 = 0;

	// setup pins
	P1DIR |= LED_1 | LED_2;
	P1DIR &= ~IR_IN;

	// set the P1 pin interrupt to be triggered on the edge of the IR detector out
	P1IE |= IR_IN;
	// P1IES |= IR_IN; // Falling edge
	P1IES &= ~IR_IN; // Rising edge
	P1IFG = 0;

	//Enable global interrupts
	//WRITE_SR(GIE);
	__enable_interrupt();

	while(1) {
		//Loop forever, interrupts take care of the rest
		//__bis_SR_register(CPUOFF + GIE);
		if( irReceivedCommand != 0 ) {
			irReceivedCommand = 0;
			P1OUT ^= LED_1;
		}
	}
}
