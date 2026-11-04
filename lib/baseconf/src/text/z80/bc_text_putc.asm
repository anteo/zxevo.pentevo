        MODULE bc_text_putc
SECTION code_clib
PUBLIC _bc_text_putc
EXTERN bc__text_begin
EXTERN bc__text_put
EXTERN _bc__video_end

; SDCC character:8. One lease, including control characters. Preserve IX/IY.
;
; Processes one hardware text character or cursor control while holding a
; video lease.
;
; Model:
;
; status=text_begin(); if (status!=OK) return status;
; text_put(character); return video_end();
;
; Save the character across begin(); no local surface copy. Status in L,
; AF/BC/DE/HL volatile; IX/IY preserved. The function owns the lease.
_bc_text_putc:
        ld hl,2
        add hl,sp
        ld a,(hl)
        push af
        call bc__text_begin
        ld a,l
        or a
        jr nz,@failed
        pop af
        call bc__text_put
        jp _bc__video_end
@failed:
        pop bc
        ret
