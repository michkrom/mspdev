/* MSP430 printf() Tests
 *
 * Description: A modified version of the test code for testing oPossum's
 *              tiny printf() function. More information on the printf()
 *              function can be found at the following link.
 *
 *              http://www.43oh.com/forum/viewtopic.php?f=10&t=1732
 *
 *              This specific code tests the printf() function using
 *              the hardware UART on the MSP430G2553 with a baud rate
 *              of 9600. Once the character 't' is received, the test
 *              sequence is started.
 *
 *              This code was originally created for "NJC's MSP430
 *              LaunchPad Blog".
 *
 * Author:  Nicholas J. Conn - http://msp430launchpad.com
 * Email:   webmaster at msp430launchpad.com
 * Date:    06-07-12
 ******************************************************************************/

#include "msp430.h"
#include "stdarg.h"

// Define flags used by the interrupt routines
#define TX	BIT0

// Flag register
volatile unsigned char FLAGS = 1;

void sendByte(unsigned char);
void printf(char *, ...);
void uart_init(void);
void putc(char c);

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;  // Stop WDT

	BCSCTL2 = SELM_0;          // DCO->SCLK, SMCLK

	// confugure DCO to and use it as SMCLK
#if DCO16MHZ
	BCSCTL1 = CALBC1_16MHZ;    // Set range
	DCOCTL = CALDCO_16MHZ;     // Set DCO step and modulation	
#else
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
	char *s = "NJC's MSP430 LaunchPad Blog";
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

