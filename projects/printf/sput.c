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

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>


/**
 * TXD on P1.1
 */
#define TXD BIT1

/**
 * RXD on P1.2
 */
#define RXD BIT2

/**
 * Baudrate
 */
#define BAUDRATE                 9600

/**
 * Bit time in "loops" - must be tuned per MCU & clocks. 
 */
#define BIT_TIME_LOOPS        (220000/BAUDRATE) // 220000 loops per second (@ ~1MHZ SCLK)

void uart_init(void)
{
//     P1SEL |= TXD;
     P1DIR |= TXD;

     // P1IES |= RXD;                 // RXD Hi/lo edge interrupt
     // P1IFG &= ~RXD;                // Clear RXD (flag) before enabling interrupt
     // P1IE  |= RXD;                 // Enable RXD interrupt
}

bool uart_getc(uint8_t *c)
{
     return false;
}

static void bitout(int bit)
{
	// set the TX pin to the bit value
	if( bit )
		P1OUT |= TXD;
	else
		P1OUT &= ~TXD;
	// wait for bit_time
	register unsigned i = BIT_TIME_LOOPS;
	while(i--) 
		asm(""); // so the loop is not optimized out
}

void uart_putc(uint8_t c)
{
	// start bit
	bitout(0);
	// shift out 8 bits of data
	register int i = 8;
	while(i--)
	{
		bitout( c & 1 );
		c >>= 1;
	}
	// stop bits
	bitout(1);
	// second stop bit (not necessary)
	// bitout(1);
}

void uart_puts(const char *str)
{
     while(*str != 0) uart_putc(*str++);
}

