;
; A configurable CRT for bare-metal targets
;
;

        MODULE baseconf_crt0

;-------
; Include zcc_opt.def to find out information about us
;-------

defc    crt0 = 1
INCLUDE "zcc_opt.def"

;-------
; Some general scope declarations
;-------

EXTERN    _main           ;main() is always external to crt0 code
PUBLIC    __Exit         ;jp'd to by exit()
PUBLIC    l_dcal          ;jp(hl)
PUBLIC    call_rom3       ;ZX ROM interposer used by z88dk ZX routines
PUBLIC    __SYSVAR_BORDCR
EXTERN	  asm_im1_handler
EXTERN	  asm_nmi_handler

defc    __SYSVAR_BORDCR = 23624

IF DEFINED_CRT_ORG_BSS
defc    __crt_org_bss = CRT_ORG_BSS
ENDIF

IFNDEF      CRT_ORG_CODE
defc    CRT_ORG_CODE = 0x0000
ENDIF

IF CRT_ORG_CODE = 0x0000
    ; By default we don't have any rst handlers
defc    TAR__crt_enable_rst = $0000
ENDIF

    ; Default, don't change the stack pointer
defc    TAR__register_sp = -1
    ; Default, 32 functions can be registered for atexit()
defc    TAR__clib_exit_stack_size = 32
    ; Default, halt loop
defc    TAR__crt_on_exit = 0x10001

    ; Keep the classic malloc heap disabled unless the application opts in.
    ; CRT_STACK_SIZE > 0 asks crt_init_heap to use memory between BSS and SP.
IFNDEF CRT_STACK_SIZE
IFNDEF TAR__crt_stack_size
defc    TAR__crt_stack_size = -1
ENDIF
ENDIF
IFNDEF CLIB_MALLOC_HEAP_SIZE
IFNDEF TAR__clib_malloc_heap_size
defc    TAR__clib_malloc_heap_size = -1
ENDIF
ENDIF

defc    __CPU_CLOCK = 4000000
INCLUDE "crt/classic/crt_rules.inc"

        org    	CRT_ORG_CODE


IF CRT_ORG_CODE = 0x0000
        jp      start
INCLUDE "crt/classic/crt_z80_rsts.inc"
ENDIF

IFDEF CRT_INCLUDE_PREAMBLE
INCLUDE "crt_preamble.asm"
ENDIF

start:
INCLUDE "crt/classic/crt_init_rabbit.inc"
INCLUDE "crt/classic/crt_init_sp.inc"
    ; Setup BSS memory and perform other initialisation
        call    crt0_init
    ; Make room for the atexit() stack
INCLUDE "crt/classic/crt_init_atexit.inc"

    ; Initialize malloc only when enabled by the application CRT settings.
INCLUDE "crt/classic/crt_init_heap.inc"
INCLUDE "crt/classic/crt_init_rabbit_farheap.inc"

    ; Setup the desired interrupt mode
INCLUDE "crt/classic/crt_init_interrupt_mode.inc"
    ; Turn on interrupts if desired
INCLUDE "crt/classic/crt_init_eidi.inc"

    ; Entry to the user code
        call    _main
    ; Exit code is in hl
__Exit:
    ; crt0_exit any resources
        call    crt0_exit

    ; Set the interrupt mode on exit
INCLUDE "crt/classic/crt_exit_eidi.inc"

    ; How does the program end?
INCLUDE "crt/classic/crt_terminate.inc"

l_dcal:
        jp      (hl)

; Call a Spectrum ROM routine whose address follows the call instruction.
call_rom3:
        exx
        ex      (sp), hl
        ld      c, (hl)
        inc     hl
        ld      b, (hl)
        inc     hl
        ex      (sp), hl
        push    bc
        exx
        ret

    ; Select BaseConf's console for stdio builds. Otherwise provide a no-op ABI
    ; symbol so CRT selection cannot pull in ZX's native console.
IF CRT_ENABLE_STDIO = 1
defc TAR__fputc_cons_generic = 1
ELSE
defc TAR__fputc_cons_generic = 0
defc DEFINED_fputc_cons = 1
SECTION code_clib
PUBLIC fputc_cons
fputc_cons:
        ret
ENDIF

INCLUDE "crt/classic/crt_runtime_selection.inc"

    ; If we were given a model then use it
IF DEFINED_CRT_MODEL
defc __crt_model = CRT_MODEL
        ELIF DEFINED_CRT_ORG_BSS
    ;; If BSS is defined, then assume we're ROM model
defc __crt_model = 1
ENDIF
INCLUDE	"crt/classic/crt_section.inc"
INCLUDE "crt/classic/crt_rabbit_banks.inc"
