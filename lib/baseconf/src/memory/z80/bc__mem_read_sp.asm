; Return the current stack pointer in HL.
SECTION code_clib
PUBLIC _bc__mem_read_sp
; Getter: HL=entry SP, including this CALL's two-byte return address.
; No arguments; A/BC/DE/IX/IY preserved, HL and ADD arithmetic flags change.
; Does not return the caller's pre-CALL stack pointer.
_bc__mem_read_sp:
        ld hl,0
        add hl,sp
        ret
