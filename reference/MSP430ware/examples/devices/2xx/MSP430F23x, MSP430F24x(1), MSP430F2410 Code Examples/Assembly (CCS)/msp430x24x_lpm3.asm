;*******************************************************************************
;   MSP430x24x Demo - Basic Clock, LPM3 Using WDT ISR, 32kHz ACLK
;
;   Description: This program operates MSP430 normally in LPM3, pulsing P1.0
;   at 4 second intervals. WDT ISR used to wake-up system. All I/O configured
;   as low outputs to eliminate floating inputs. Current consumption does
;   increase when LED is powered on P1.0. Demo for measuring LPM3 current.
;   ACLK = LFXT1/4 = 32768/4, MCLK = SMCLK = default DCO ~1.045MHz
;   //* External watch crystal on XIN XOUT is required for ACLK *//
;
;                MSP430F249
;             -----------------
;         /|\|              XIN|-
;          | |                 | 32kHz
;          --|RST          XOUT|-
;            |                 |
;            |             P1.0|-->LED
;
;  JL Bile
;  Texas Instruments Inc.
;  May 2008
;  Built Code Composer Essentials: v3 FET
;*******************************************************************************
 .cdecls C,LIST, "msp430x24x.h"
;-------------------------------------------------------------------------------
			.text	;Program Start
;-------------------------------------------------------------------------------
RESET       mov.w   #0500h,SP         ; Initialize stackpointer
SetupWDT    mov.w   #WDT_ADLY_1000,&WDTCTL  ; WDT 1s*4 interval timer
SetupBC     bis.b   #DIVA_1,&BCSCTL1        ; ACLK/4
            bis.b   #WDTIE,&IE1             ; Enable WDT interrupt
SetupP1     mov.b   #0FFh,&P1DIR            ; All P1.x outputs
            clr.b   &P1OUT                  ; All P1.x reset
SetupP2     mov.b   #0FFh,&P2DIR            ; All P2.x outputs
            clr.b   &P2OUT                  ; All P2.x reset
SetupP3     mov.b   #0FFh,&P3DIR            ; All P3.x outputs
            clr.b   &P3OUT                  ; All P3.x reset
SetupP4     mov.b   #0FFh,&P4DIR            ; All P4.x outputs
            clr.b   &P4OUT                  ; All P4.x reset
SetupP5     mov.b   #0FFh,&P5DIR            ; All P4.x outputs
            clr.b   &P5OUT 
SetupP6     mov.b   #0FFh,&P6DIR            ; All P4.x outputs
            clr.b   &P6OUT  
            
Mainloop    bis.w   #LPM3+GIE,SR            ; Enter LPM3, enable interrupts
            bis.b   #001h,&P1OUT            ; Set P1.0
            push.w  #20000                  ; Delay to TOS
Delay       dec.w   0(SP)                   ; Decrement TOS
            jnz     Delay                   ; Delay over?
            incd.w  SP                      ; Clean stack
            bic.b   #001h,&P1OUT            ; Reset P1.0
            jmp     Mainloop                ; Again
                                            ;
;-------------------------------------------------------------------------------
WDT_ISR  ;  Exit LPM3 on reti
;-------------------------------------------------------------------------------
            bic.w   #LPM3,0(SP)             ; Clear LPM3 from TOS
            reti                            ;
                                            ;
;-------------------------------------------------------------------------------
;			Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect	".int26"              ; WDT Vector
            .short	WDT_ISR
            .sect	".reset"            ; POR, ext. Reset
            .short    RESET
            .end
