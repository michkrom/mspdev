/******************************************************************************
 * Simple software serial UART (TX only).
 *
 * This code implements putc and puts with purely software bitbanging.
 * It uses busy loop to do the timing and hence does not use hardware TIMER.
 * it does require some tuning: the BIT_TIME_LOOPS may need to be adjusted
 *
 * (c) 2013, Broken Bit Labs, Michal Krombholz
 * GPL v2
 *
 * http://github.com/michkrom/mspdev/projects/printf
 ******************************************************************************/

#include "msp430.h"
#include "stdarg.h"

#include "sput.h"

// Flag register
volatile unsigned char FLAGS = 1;

void printf(char *, ...);

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;  // Stop WDT

	BCSCTL2 = SELM_0;          // DCO->SCLK, SMCLK

	// confugure DCO to 1MHz and use it as SMCLK
#if DCO16MHZ
	// the g series does not have this calibrated!
	BCSCTL1 = CALBC1_16MHZ;    // Set range
	DCOCTL = CALDCO_16MHZ;     // Set DCO step and modulation	
#else // choose 1MHz
	BCSCTL1 = CALBC1_1MHZ;     // Set range
	DCOCTL = CALDCO_1MHZ;      // Set DCO step and modulation	
#endif

	// LED
	P1DIR |= BIT0; 

	uart_init();

#if BASIC_TEST
	while(1)
	{ 
		P1OUT ^= BIT0;
#if 1
		puts("HELLO!\r\n");
#else
		putc(0xFF); 
		putc(0x00); 
		putc(0xAA); 
		putc(0x55); 
#endif
	// delay some for debuging
	register unsigned i = 250; // ~ 1ms delay @ 1MHz SCLK (takes 4 clocks for one loop)
	while(i--) asm(""); // so the loop does not get optimized out
	}

#else
	                                                                                                                                                                                                                                

	// Initialize values to display
	char *s = "BroBit's simple sput (RS232)";
	char c = '!';
	int i = -12345;
	unsigned u = 4321;
	long int l = -123456780;
	long unsigned n = 1098765432;
	unsigned x = 0xABCD;

	while(1) {
		P1OUT |= BIT0; 
		// Display all the different types of values
	    	printf("String         %s\r\n", s);
	    	printf("Char           %c\r\n", c);
	    	printf("Integer        %i\r\n", i);
	    	printf("Unsigned       %u\r\n", u);
	    	printf("Long           %l\r\n", l);
	    	printf("uNsigned loNg  %n\r\n", n);
	    	printf("heX            %x\r\n", x);
	    	printf("bin            %b\r\n", x);
		P1OUT &= ~BIT0; 
	}
#endif
}

