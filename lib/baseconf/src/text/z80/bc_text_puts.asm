        MODULE bc_text_puts
SECTION code_clib
PUBLIC _bc_text_puts
EXTERN bc__text_begin
EXTERN bc__text_put
EXTERN _bc__video_end

; SDCC text:16. One lease for the string. Preserve IX/IY.
;
; Processes a null-terminated string at the hardware text cursor under one
; video lease.
;
; Model:
;
; if (!text) return INVALID_ARGUMENT; if (text_begin()!=OK) return status;
; while (*text) text_put(*text++); return video_end();
;
; String storage stays readable during one lease. The next-character pointer
; is pushed around text_put, never stored in a callee-owned argument slot.
; AF/BC/DE/HL volatile; IX/IY preserved, status in L.
_bc_text_puts:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld a,d
        or e
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret z
        push de
        call bc__text_begin
        ld a,l
        or a
        jr nz,@failed
        pop hl
@next:
        ld a,(hl)
        or a
        jp z,_bc__video_end
        inc hl
        push hl
        call bc__text_put
        pop hl
        jr @next
@failed:
        pop bc
        ret
