;*******************************************************************************
;   MSP430F20xx Demo - Software Poll P1.4, Set P1.0 if P1.4 = 1
;
;   Description: Poll P1.4 in a loop, if hi P1.0 is set, if low, P1.0 reset.
;   ACLK = n/a, MCLK = SMCLK = default DCO
;
;                MSP430F20xx
;             -----------------
;         /|\|              XIN|-
;          | |                 |
;          --|RST          XOUT|-
;      /|\   |                 |
;       --o--|P1.4        P1.0|-->LED
;      \|/
;
;   P.Thanigai
;   Texas Instruments Inc.
;   May 2007
;   Built with Code Composer Essentials Version: 2.0
;*******************************************************************************
 .cdecls C,LIST,  "msp430x20x3.h"
;------------------------------------------------------------------------------
            .text                           ; Program Start
;------------------------------------------------------------------------------
RESET       mov.w   #0280h,SP               ; Initialize stackpointer
StopWDT     mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; Stop WDT
SetupP1     bis.b   #001h,&P1DIR            ; P1.0 output
                                            ;				          			
Mainloop    bit.b   #010h,&P1IN             ; P1.4 hi/low?
            jc      ON                      ; jmp--> P1.4 is set
                                            ;
OFF         bic.b   #001h,&P1OUT            ; P1.0 = 0 / LED OFF
            jmp     Mainloop                ;
ON          bis.b   #001h,&P1OUT            ; P1.0 = 1 / LED ON
            jmp     Mainloop                ;
                                            ;
;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET                   ;
            .end