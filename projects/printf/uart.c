/* MSP430 UART RS232 (hw based)
 *
 ******************************************************************************/


/**
 * Initializes the UART for 9600 baud with a RX interrupt
 **/
void uart_init(void) {
    P1SEL = BIT1 + BIT2 ;				// P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;				// P1.1 = RXD, P1.2=TXD

    UCA0CTL1 |= UCSSEL_1;				// CLK = ACLK
    UCA0BR0 = 0x03;						// 32kHz/9600 = 3.41
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS1 + UCBRS0;			// Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;				// **Initialize USCI state machine**
    IE2 |= UCA0RXIE;					// Enable USCI_A0 RX interrupt
}

/**
 * puts() is used by printf() to display or send a string.. This function
 *     determines where printf prints to. For this case it sends a string
 *     out over UART, another option could be to display the string on an
 *     LCD display.
 **/
void puts(char *s) {
    char c;

    // Loops through each character in string 's'
    while (c = *s++) {
        sendByte(c);
    }
}
/**
 * puts() is used by printf() to display or send a character. This function
 *     determines where printf prints to. For this case it sends a character
 *     out over UART.
 **/
void putc(unsigned b) {
    sendByte(b);
}

/**
 * Sends a single byte out through UART
 **/
void sendByte(unsigned char byte )
{
    while (!(IFG2&UCA0TXIFG));			// USCI_A0 TX buffer ready?
    UCA0TXBUF = byte;					// TX -> RXed character
}

/**
 * Interrupt routine for receiving a character over UART
 **/
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    char r = UCA0RXBUF;					// Get the received character
    if (r == 't')						// 'u' received?
    {
        FLAGS |= TX;					// Set flag to transmit data
        __bic_SR_register_on_exit(LPM3_bits);	// Wake-up CPU
    }
}

