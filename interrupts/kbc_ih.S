/* void update_OBF_status_asm(); */
/* using _C_LABEL() from <machine/asm.h> */
#include  <machine/asm.h>
#include "i8042.h"

.intel_syntax
.global _C_LABEL(update_obf_status_asm)
.text

_C_LABEL(update_obf_status_asm):
	/* Reset obf status */
	mov DWORD PTR [obf_status], 0
	/* Read keyboard status */
    in eax, KBC_CTRL

	/* Handle situations where the output buffer is not full */
	test al, KBC_OBF
	jz update_obf_status_asm_end

	/* Read the keyboard obf */
	mov edx, eax
	in eax, KBC_OUT

	mov [out_buffer], al
	/* Ignore obf value if there is an error */
	test dl, KBC_TIMEOUT_ERR 
	jnz update_obf_status_asm_end

	test dl, KBC_PARITY_ERR
	jnz update_obf_status_asm_end

	/* Set obf status as valid */
	mov DWORD PTR [obf_status], 1
update_obf_status_asm_end:
    ret
