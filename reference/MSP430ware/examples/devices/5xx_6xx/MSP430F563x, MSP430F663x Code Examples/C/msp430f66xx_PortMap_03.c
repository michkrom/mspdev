//******************************************************************************
// MSP430F66x Demo - Port Map single function to multiple pins;
//		Multiple runtime configurations
//
// Description:  The port mapping of TIMERB0 is changed with each WDT interrupt.
//               TimerB0 provides 4 PWMs with 4 different duty cycles.
//               TimerB0 is sourced from ACLK.  ACLK is REFO, 32kHz
//                                                       
//                 MSP430x66x
//             ------------------------                        
//         /|\|                        |                       
//          | |                        |                       
//          --|RST                     |                                   
//            |     P2.0(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                 
//            |     P2.1(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                
//            |     P2.2(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                
//            |     P2.3(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                  
//            |     P2.4(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                  
//            |     P2.5(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                 
//            |     P2.6(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                  
//            |     P2.7(TB0CCR1,2,3,4)|--> 25,50,75,87.5%                  
//                 
//   Priya Thanigai
//   Texas Instruments Inc.
//   Nov 2009
//   Built with IAR Embedded Workbench Version: 4.20 & Code Composer Studio V4.0
//******************************************************************************
#include <msp430f6638.h>

#define PORT_MAP_RECFG                      // Multiple runtime Port Map configurations

const unsigned char PortSequence[4] = {   PM_TB0CCR1B,
                                          PM_TB0CCR2B,
                                          PM_TB0CCR3B,  
                                          PM_TB0CCR4B };

unsigned char count=0;
void Port_Mapping(void);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  Port_Mapping();
  // Setup Port Pins              
  P2DIR |= 0xFF;                            // P2.0 - P2.7 output
  P2SEL |= 0xFF;                            // P2.0 - P2.6 Port Map functions
  
  // Setup TB0 
  TB0CCR0 = 256;                            // PWM Period/2
  TB0CCTL1 = OUTMOD_6;                      // CCR1 toggle/set
  TB0CCR1 = 192;                            // CCR1 PWM duty cycle
  TB0CCTL2 = OUTMOD_6;                      // CCR2 toggle/set
  TB0CCR2 = 128;                            // CCR2 PWM duty cycle
  TB0CCTL3 = OUTMOD_6;                      // CCR3 toggle/set
  TB0CCR3 = 64;                             // CCR3 PWM duty cycle
  TB0CCTL4 = OUTMOD_6;                      // CCR4 toggle/set
  TB0CCR4 = 32;                             // CCR4 PWM duty cycle  
  TB0CTL = TBSSEL_1 + MC_3;                 // ACLK, up-down mode
  
// Setup WDT in interval mode                                                 
  WDTCTL = WDT_ADLY_1000;                   // WDT 1s, ACLK, interval timer
  SFRIE1 |= WDTIE;                          // Enable WDT interrupt

  while(1)
  {
    __bis_SR_register(LPM3_bits + GIE);       // Enter LPM3 w/interrupt
    __no_operation();                         // For debugger
    
    Port_Mapping();
    count++;
    if(count==4)
      count = 0; 
  }
}

// Watchdog Timer interrupt service routine
#pragma vector = WDT_VECTOR
__interrupt void watchdog_timer(void)
{
  __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3

}

void Port_Mapping(void)
{
  unsigned char i;
  volatile unsigned char *ptr;
  // Disable Interrupts before altering Port Mapping registers
  __disable_interrupt();                    
  // Enable Write-access to modify port mapping registers
  PMAPPWD = 0x02D52;                       
  
  #ifdef PORT_MAP_RECFG         
  // Allow reconfiguration during runtime
  PMAPCTL = PMAPRECFG;                      
  #endif  
  
  ptr = &P2MAP0;
  for(i=0;i<8;i++)
  {
    *ptr = PortSequence[count];
    ptr++;
  }
 
  // Disable Write-Access to modify port mapping registers
  PMAPPWD = 0;                              
  #ifdef PORT_MAP_EINT
  __enable_interrupt();                     // Re-enable all interrupts
  #endif  
  
}

