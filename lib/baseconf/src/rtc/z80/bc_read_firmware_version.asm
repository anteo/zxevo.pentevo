        MODULE bc_read_firmware_version
SECTION code_clib
PUBLIC _bc_read_firmware_version
EXTERN _bc_read_avr_version

; Both API names return the same AVR metadata.
defc _bc_read_firmware_version = _bc_read_avr_version
