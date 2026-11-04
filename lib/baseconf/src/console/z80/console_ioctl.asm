        MODULE baseconf_console_ioctl

PUBLIC generic_console_ioctl
EXTERN _bc_console_font

SECTION code_clib
; Handles the generic console request to change the raw font pointer and
; reports other requests as unsupported.
;
; Model:
;
; if (request!=3) return CF=1; raw_console_font=load_le16(argument); return CF=0;
;
; Native generic-console input A=request,DE=pointer to font pointer.
; AF/DE/HL volatile; BC/IX/IY preserved. This raw font-pointer IOCTL does not
; validate a bc_font_t descriptor or change the active descriptor-based font.
generic_console_ioctl:
        cp 3
        jr nz, @unsupported
        ld a, (de)
        ld l, a
        inc de
        ld a, (de)
        ld h, a
        ld (_bc_console_font), hl
        and a
        ret
@unsupported:
        scf
        ret
