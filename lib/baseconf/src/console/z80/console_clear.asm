        MODULE bc_console_clear
PUBLIC generic_console_cls
EXTERN _bc__console_clear
SECTION code_clib
; Generic-console adapter: call console_clear(), then return A=0, Z=1, CF=0.
; No arguments; AF/BC/DE/HL volatile, IX/IY preserved by the backend.
; The backend owns its lease; its status is not reported through A or carry.
generic_console_cls:
        call _bc__console_clear
        xor a
        ret
