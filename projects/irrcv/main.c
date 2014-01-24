
/**
 * Servo controller for MSP430
 *
 *
 * @author Michal Krombholz
 * @license GNU General Public License (GPL) v2 or later
 */

#include <msp430.h>

#define  LED1      BIT0
#define  LED2      BIT6

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
  do
  {
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
  //P1OUT ^= LED_1;
}


//------------------------------ IR decoder ----------------------------------

// times in n*100us
// start = 1600us
// long(0) = 800us
// short(0) = 400us
// repeats in ~170ms

unsigned long irCommand;
int irBitCount;
long irPreviousEdgeTimeStamp=0; // rounded to next 100us


// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{   
  P1OUT ^= LED_1;
  
  // protocol timing
  typedef enum { 
    IR_SYMBOL_START = 16, 
    IR_SYMBOL_LONG = 8,
    IR_SYMBOL_SHORT = 4,
    IR_SYMBOL_ERROR = 0 } SYMBOL;
    
  const int IR_BIT_COUNT = 32;    
  const int TOLERANCE = 2;
  
  // measure time from prev rise to this rise
  long now = (getTimeUs()+50)/100; // in 100us
  long deltaLong = now - irPreviousEdgeTimeStamp;
  irPreviousEdgeTimeStamp = now;
  
  // cap delta to 16 signed int
  int delta;
  if( deltaLong < 0 || deltaLong > 0x7FFF )
    delta = 0x7fff;
  else
    delta = deltaLong;  
  
  // classify received symbol based on timing since last edge
  SYMBOL symbol;
  if( delta > IR_SYMBOL_START-TOLERANCE && delta < IR_SYMBOL_START+TOLERANCE )
    symbol = IR_SYMBOL_START;
  else if( delta > IR_SYMBOL_LONG-TOLERANCE && delta < IR_SYMBOL_LONG+TOLERANCE )
    symbol = IR_SYMBOL_LONG;
  else if( delta > IR_SYMBOL_LONG-TOLERANCE && delta < IR_SYMBOL_LONG+TOLERANCE )
    symbol = IR_SYMBOL_SHORT;
  else
    symbol = IR_SYMBOL_ERROR;

  // act on received symbol
  switch(symbol)
  {
    case IR_SYMBOL_START:
      {
	// start interval
	irBitCount = 0;
	irCommand = 0;
	break;
      }
    case IR_SYMBOL_ERROR:
      {
	irBitCount = IR_BIT_COUNT+1; // mark invalid command
	P1OUT ^= LED_2;
	break;
      }
    default: // it's a good bit!
      {
	if( irBitCount < IR_BIT_COUNT )
	{
	  // shift command and count the bit
	  irCommand <<= 1;    
	  irBitCount ++;
	  if( symbol == IR_SYMBOL_SHORT )
	  {
	    // long interval (bit 1)
	    irCommand |= 1;
	  }
	}
	else // error in bit count (too many)
	{
	  irBitCount = IR_BIT_COUNT + 1;      
	}
	break;
      }
  }
  
  if( irBitCount == IR_BIT_COUNT )
  {
  }
  
  // P1.3 IFG cleared to enable next interrupt
  P1IFG = 0;
}


//--------------------------------------------------------------------------


int main(void) {

	// Disable the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Set ACLK to use internal VLO (12 kHz clock)
	BCSCTL3 |= LFXT1S_2;
	
	//BCSCTL1 = CALBC1_1MHZ;    // Set range
	//DCOCTL = CALDCO_1MHZ;     // Set DCO step and modulation	

	// confugure DCO to 16MHz and use it as SMCLK
	BCSCTL2 = SELM_0;          // DCO->SCLK, SMCLK
	BCSCTL1 = CALBC1_16MHZ;    // Set range
	DCOCTL = CALDCO_16MHZ;     // Set DCO step and modulation	
	
	//Set TimerA to use auxiliary clock in UP mode
	//TACTL = TASSEL_1 | MC_1;
	
	// configure SMCLK to 2MHz (SCLK/8)
	BCSCTL2 |= DIVS_3;
	
	//Set TimerA to SMCLK and CONTINOUS mode and DIV/2 ==> SMCLK/2=1MHz
	TACTL = TASSEL_2 | MC_2 | ID_1;	
	//Enable the interrupt for TACCR0 match
	TACCTL0 = CCIE;
	// Set TACCR0 which also starts the timer.
	TACCR0 = 0;

	// setup pins
	P1DIR |= LED_1 | LED_2;
	P1DIR &= ~IR_IN;

	// set the P1 pin interrupt to be triggered on the falling edge of the IR detector out
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
	}
}
