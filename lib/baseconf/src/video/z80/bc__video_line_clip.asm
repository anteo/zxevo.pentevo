        MODULE bc__video_line_clip
SECTION code_clib
PUBLIC bc__video_line_clip
PUBLIC bc__video_line_return

; Signed HL/DE endpoints, positive BC limit. Return sorted, clipped inclusive
; HL/DE endpoints, carry if empty. No overflow-prone signed subtraction.
;
; Orders and clips a pair of signed endpoints to the valid range of one axis
; and reports when the span is empty.
;
; Model:
;
; if (signed(first)>signed(last)) swap(first,last);
; first=max(first,0); last=min(last,limit-1); return empty=(first>last);
;
; The register endpoints and CF contract are above. Positive limit is required.
; Only AF/HL/DE change; BC/IX/IY preserved. Compare sign first, then unsigned
; bytes of same-sign values, so INT16_MIN/MAX never overflow a subtraction.
bc__video_line_clip:
        ld a,h
        xor d
        jp p,@same_sign
        bit 7,h
        jr nz,@ordered
        ex de,hl
        jr @ordered
@same_sign:
        ld a,h
        cp d
        jr c,@ordered
        jr nz,@swap
        ld a,l
        cp e
        jr c,@ordered
        jr z,@ordered
@swap:
        ex de,hl
@ordered:
        bit 7,d
        jr nz,@empty
        bit 7,h
        jr z,@lower_ready
        ld hl,0
@lower_ready:
        ld a,d
        cp b
        jr c,@upper_ready
        jr nz,@clip_upper
        ld a,e
        cp c
        jr c,@upper_ready
@clip_upper:
        ld d,b
        ld e,c
        dec de
@upper_ready:
        ld a,h
        cp d
        jr c,@valid
        jr nz,@empty
        ld a,l
        cp e
        jr c,@valid
        jr nz,@empty
@valid:
        or a
        ret
@empty:
        scf
        ret

; Every axis-line backend shares the caller's saved IX frame. No static scratch.
; Shared axis-line tail epilogue: discard all locals below the persistent IX
; frame, restore caller IX, return void. Requires the dispatcher saved-IX frame;
; this is not an independently callable C function. Other working registers
; remain volatile; IY was never changed by the axis backends.
bc__video_line_return:
        ld sp,ix
        pop ix
        ret
