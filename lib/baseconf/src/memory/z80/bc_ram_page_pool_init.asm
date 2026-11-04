        MODULE bc_ram_page_pool_init
SECTION code_clib
PUBLIC _bc_ram_page_pool_init

; Clear the eight allocation bytes and attach the caller's allowed mask.
;
; if (!pool || !page_mask) return BC_INVALID_ARGUMENT;
; clear(pool->allocated, 8);
; pool->page_mask = page_mask;
; return BC_OK;
;
; SDCC pool pointer at SP+2, mask pointer at SP+4, status in L.
; Preserve IX/IY, clobber AF/BC/DE/HL.
_bc_ram_page_pool_init:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        inc hl
        ld c,(hl)
        inc hl
        ld b,(hl)
        ld a,d
        or e
        jr z,@invalid
        ld a,b
        or c
        jr z,@invalid
        push de
        ex de,hl
        inc hl
        inc hl                       ; allocated follows the mask pointer
        ld e,8
        xor a
@clear:
        ld (hl),a
        inc hl
        dec e
        jr nz,@clear
        pop hl
        ld (hl),c
        inc hl
        ld (hl),b
        ld l,a                       ; BC_OK
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
