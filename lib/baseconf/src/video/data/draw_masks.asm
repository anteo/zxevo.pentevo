        MODULE video_draw_masks
SECTION rodata_graphics
PUBLIC bc_draw_masks

; Bitmap masks shared by the ZX and ATM640 native pixel writers.
bc_draw_masks:
        defb 080h,040h,020h,010h,008h,004h,002h,001h
