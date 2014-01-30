/**
 * IR receiver/decoder for MSP430
 * for PROTOCOL TracerJet RC codes
 *
 *
 * @author Michal Krombholz
 * @license GNU General Public License (GPL) v2 or later
 */

#include <msp430.h>

// simple printf
void simple_print_init();
void printf(char *, ...);


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


// "Protocol TracerJet Heli" RC remote protocol description.
//
// The remote pulses IR (@38kHz carrier) for a variable time of 400us
// the time between IR bursts is used to mean one of 3 SYMBOLs:
// START, LONG and SHORT.
// The command consist of sequence of 500us burst then START then 32 bits of data.
// the gap between IR bursts is 400us
// the gap between commands is about ~170ms
//
// start symbol + 32bit, IR burst off time is 400us
// symbol times from edge of IR to next edge of IR
// start    = 2000us
// long(1)  = 1200us
// short(0) =  800us
//
// Detection Algorithm
//
// The symbols are timed from falling edge of IR to the next fallling edge of IR (ie end of burst).
// However since the IR detector uses "0" level as "active" (ie IR present)
// the pin interrupts as active on "rising" edge of detector signal.
// The algoritm looks for the START symbol, and then expects the bits to follow.
// The first 500us burst is only used to time next symbol (ie START symbol).
// However, the alg is not perfect as it hinges on receiving and recognizing correct START symbol.
// A possibility exist that wrapping of time ticks would produce a delta time matching start by chance.
// This is however rather unlikely (more unlikely when timer wraparound is rare/longer).
// Also, the alg has no build in timeout and when not all bits are received in one IR command
// (due to interference) it will continue to read bits of next one and eventually err (too many bits).
// Hence it will faile receiving both commands (bad one and next potentially good one).
//

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
    case IR_SYMBOL_START: { // start symbol recognized
        // start interval
        irBitCount = 0;
        irCommand = 0;
        P1OUT &= ~LED_2;
        break;
    }
    case IR_SYMBOL_ERROR: { //  unrecognized timing, error!
        if( irBitCount <= IR_BIT_COUNT )
        {
            irBitCount = IR_BIT_COUNT+1; // mark invalid command
            P1OUT |= LED_2;
        }
        break;
    }
    default: { // it's a good bit - either 0 or 1 !
        if( irBitCount <= IR_BIT_COUNT )
        {
            // shift command and count the bit
            irCommand <<= 1;
            irBitCount ++;
            if( symbol == IR_SYMBOL_LONG ) {
                // long interval ==> bit=1
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

    // initialize serial-out printf support
    simple_print_init();
    printf("IR RECEIVER READY\r\n");

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
        // loop forever and decode IR codes
        if( irReceivedCommand != 0 ) {
            unsigned long tmp = irReceivedCommand;
            irReceivedCommand = 0;

            unsigned th = tmp >> 16;
            unsigned tl = tmp & 0xFFFF;
            printf("%x %x",th,tl);
//          printf(" %b %b ",th,tl);

            // PROTOCOL TRACERJET's IR decoding

            // power: 0-100 (127?); 128 - off; > 128 dynamic boost (seen 130ish to 228max only)
            unsigned pwr = th >> 8;
            unsigned lr = (th >> 4) & 0xF; // roll: left=15...right=1; neutral=8 (-7..+7)
            unsigned fb = (th >> 0) & 0xF; // pitch: forward=15..backward=1; neutral=8 (-7..+7)

            // channel 0-A; 2-B; 3-C; (value of 1 missing)
            unsigned ch =  (tl >> 14) & 0x3; // 2 bits
            // trim
            unsigned trm = (tl >>  8) & 0x3F; // 6 bits

            // 6 bits of 3 first bytes sum (sic!) - kinda checksum (ignores upper 2 bits)
            unsigned sum = tl & 0xFF;
            // potentially the 2 upper bits of last byte are .5 ch (for two pushbuttons?)
            // looks like sbdy did not think right as 2 upper bits of each byte are ignored this way

            // sum computed on 3 bytes then only lower 6 bit retained
            unsigned sum2 = ((th>>8)+(th&0xFF)+(tl>>8)) & 0x3f;

            printf(" pwr=%i",pwr);
            printf(" fb=%i lr=%i",fb-8, lr-8);
            printf(" ch=%i",ch);
            printf(" trm=%i",trm);
            printf(" sum=%X %X",sum,sum2);
            printf("\r\n");
            P1OUT ^= LED_1;
        }
    }
}
