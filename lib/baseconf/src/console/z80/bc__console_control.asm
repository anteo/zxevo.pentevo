        MODULE bc__console_control
SECTION code_clib
PUBLIC _bc__console_control
PUBLIC bc__console_control
EXTERN _bc__console_newline
EXTERN __console_x

; Applies a cursor-only control or ignores an unrecognized control byte.
;
; Model:
;
; if (c==LF) newline();
; else if (c==CR) cursor_x=0;
; else if (c==BS && cursor_x) --cursor_x;
; return OK;
;
; SDCC fastcall input L or native input A. Return status in L.
; The caller supplies current geometry and expands tabs through its renderer.
; No video lease is required. AF/BC/HL volatile, DE/IX/IY preserved.
_bc__console_control:
        ld a,l
bc__console_control:
        ld l,0
        cp 10
        jp z,_bc__console_newline
        cp 13
        jr z,@carriage_return
        cp 8
        ret nz
        ld a,(__console_x)
        or a
        ret z
        dec a
        jr @store
@carriage_return:
        xor a
@store:
        ld (__console_x),a
        ret
