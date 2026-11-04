        MODULE bc_wait_frame

SECTION code_clib
PUBLIC _bc_wait_frame

; Return BC_OK=0 after one frame INT, BC_UNSAFE_CONTEXT=6 if IFF2 is clear.
; LD A,I reports IFF2 in P/V. Do not execute EI or DI here; HALT leaves the
; caller's maskable-interrupt state unchanged. NMI can wake HALT, so the caller
; must exclude NMI. An INT between LD A,I and HALT is handled first; HALT then
; waits for the next frame request. Preserve IX/IY.
;
; Waits for a frame interrupt when maskable interrupts are enabled and
; reports an unsafe context otherwise.
;
; Model:
;
; if (!IFF2) return UNSAFE_CONTEXT; HALT(); return OK;
;
; No arguments; HL=status, AF volatile, BC/DE/IX/IY preserved by this routine
; (the interrupt handler must preserve the caller's context). A frame INT must
; be configured and NMI excluded; this is not a free-running time delay.
_bc_wait_frame:
        ld a,i
        jp po,@interrupts_disabled
        halt
        ld hl,0
        ret
@interrupts_disabled:
        ld hl,6
        ret
