/**
 * Blink example for MSP430
 *
 * Snatched from hackaday.com and rewritten to compile for a number of
 * different MCUs in the MSP430 series.
 *
 * @author Christopher Vagnetoft <noccylabs.info>
 * @license GNU General Public License (GPL) v2 or later
 */

#include <msp430.h>
#include <legacymsp430.h>
#include "ta0compat.h"

#define  LED10      BIT0
#define  LED11      BIT1
#define  LED12      BIT2
#define  LED13      BIT3
#define  LED20      BIT4
#define  LED21      BIT5
#define  LED22      BIT6
#define  LED23      BIT7
#define  LED_DIR    P1DIR
#define  LED_OUT    P1OUT

volatile unsigned char twink = 0;

void initLEDs(void) {

	LED_DIR = 0xFF; //Set LED pins as outputs
 	LED_OUT = 0xFF; //Turn on all LEDs (pool to GND)
}

int main(void) {

	// Halt the watchdog timer - According to the datasheet the watchdog timer
	// starts automatically after powerup. It must be configured or halted at
	// the beginning of code execution to avoid a system reset. Furthermore,
	// the watchdog timer register (WDTCTL) is password protected, and requires
	// the upper byte during write operations to be 0x5A, which is the value
	// associated with WDTPW.
	WDTCTL = WDTPW + WDTHOLD;

	//Setup LEDs
	initLEDs();

	//Set ACLK to use internal VLO (12 kHz clock)
	BCSCTL3 |= LFXT1S_2; 
	// SMCLK  = MCLK = VLO = 12Khz
	BCSCTL2 |= SELM_3 + SELS; 
	// Stop DCO
	_BIS_SR(SCG1 + SCG0);

	//Set TimerA to use auxiliary clock in UP mode
	TACTL = TASSEL_1 | MC_1;
	//Enable the interrupt for TACCR0 match
	TACCTL0 = CCIE;

	// Set TACCR0 which also starts the timer. At 12 kHz, counting to 300 
	// should produce an interrupt 100Hz : 12000/120=100Hz
	TACCR0 = 120;

	//Enable global interrupts
	WRITE_SR(GIE);

	while(1) {
		//Loop forever, interrupts take care of the rest
		__bis_SR_register(CPUOFF + GIE);
	}

}

unsigned char LEDS = 0;

char c1 = 0;
char powerLed = 0;

void powerLedsTick()
{
  if( ++c1 > 15 )
  {
    c1=0;
    if( powerLed == 0 )
      LEDS |= ( LED10|LED11|LED12|LED13 );      
    if( powerLed==1 )
      LEDS &= ~LED10;
    else if( powerLed==2 )
      LEDS &= ~LED11;
    else if( powerLed==3 )
      LEDS &= ~LED12;
    else if( powerLed==4 )
      LEDS &= ~LED13;    
    else if( powerLed==5 )
      ;//nochange
    else if( powerLed==6 )
      LEDS |= LED13;      
    else if( powerLed==7 )
      LEDS |= LED12;      
    else if( powerLed==8 )
      LEDS |= LED11;      
    else if( powerLed==9 )
      LEDS |= LED10;      
    else 
	powerLed=0;         
    ++powerLed ;
  }
}

char c2 = 0;
char cycloLed = 0;

void cycloLedsTick()
{
  if( ++c2 > 20 )
  {
    c2=0;
    ++cycloLed;
    if( cycloLed > 3 )
    {
      cycloLed = 0;    
    }

    LEDS |= ( LED20|LED21|LED22|LED23 );
    if( cycloLed==0 )
      LEDS &= ~LED20;
    else if( cycloLed==1 )
      LEDS &= ~LED21;
    else if( cycloLed==2 )
      LEDS &= ~LED22;
    else if( cycloLed==3 )
      LEDS &= ~LED23;    
  }
}

// Interrupt Service Routine for Timer A0. We need to use preprocessor
// directives in order to place the interrupt at the correct address for
// the current mcu.
#ifdef TIMER0_A0_VECTOR
interrupt(TIMER0_A0_VECTOR) TIMERA0_ISR(void) {
#else
interrupt(TIMERA0_VECTOR) TIMERA0_ISR(void) {
#endif
	powerLedsTick();
	cycloLedsTick();
	LED_OUT=LEDS;
}


