;*******************************************************************************
;   MSP-FET430P440 Demo - FLL+, Runs Internal DCO at 2.45MHz
;
;   Description: This program demonstrates setting the internal DCO to run at
;   2.45MHz with auto-calibration by the FLL+.
;   ACLK = LFXT1 = 32768Hz, MCLK = SMCLK = default DCO = 32 x ACLK = 1048576Hz
;   ;* An external watch crystal between XIN & XOUT is required for ACLK *//	
;
;                MSP430F449
;             -----------------
;         /|\|              XIN|-
;          | |                 | 32kHz
;          --|RST          XOUT|-
;            |                 |
;            |        P1.1/MCLK|--> MCLK = 2.4576MHz
;            |                 |
;            |        P1.5/ACLK|--> ACLK = 32kHz
;            |                 |
;
;   M. Buccini / A. Dannenberg
;   Texas Instruments Inc.
;   May 2005
;   Built with Code Composer Essentials Version: 1.0
;******************************************************************************
 .cdecls C,LIST, "msp430x44x.h"
;-------------------------------------------------------------------------------
            .text                  			; Program Start
;-------------------------------------------------------------------------------
RESET       mov.w   #0A00h,SP               ; Initialize stackpointer
SetupWDT    mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; Stop WDT
SetupFLL2   bis.b   #FN_2,&SCFI0            ; x2 DCO, 4MHz nominal DCO
            bis.b   #XCAP18PF,&FLL_CTL0     ; set load capacitance for xtal
            mov.b   #74,&SCFQCTL            ; (74+1) x 32768 = 2.4576MHz
                                            ;
SetupPorts  bis.b   #022h,&P1DIR            ; P1.1 & P1.5 to output direction
            bis.b   #022h,&P1SEL            ; P1.1 & P1.5 to output MCLK & ACLK
                                            ;
Mainloop    jmp     Mainloop                ; Loop with CPU running
                                            ;
;-------------------------------------------------------------------------------
;           Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; RESET Vector
            .short  RESET                   ;
            .end







