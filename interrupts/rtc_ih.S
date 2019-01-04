/* void rtc_int_handle_asm(); */
/* using _C_LABEL() from <machine/asm.h> */
#include  <machine/asm.h>
#define BIT(x) (1<<x)

/* Registers ports */
#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71

/* RTC Internal Address Space */
#define SECOND 0x0
#define MINUTE 0x02
#define HOUR 0x04
#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B
#define RTC_REG_C 0x0C
#define RTC_REG_D 0x0D

/* Register B */
#define REG_B_UPDATE_INT BIT(4)

.intel_syntax
.global _C_LABEL(rtc_int_handle_asm)
.text


/* Read register, consiste no write_address + read data */
/* Write Address, outb para RTC_ADDR_REG */ 
/* Read data, inb para RTC_DATA_REG */
_C_LABEL(rtc_int_handle_asm):

    /* Reads C register */
    mov al, RTC_REG_C
    out RTC_ADDR_REG, al 

    in al, RTC_DATA_REG

    /* Not a update interrupt */
    test al, REG_B_UPDATE_INT 
    jz rtc_int_handle_asm_end

    /* Reads seconds */
    mov al, SECOND
    out RTC_ADDR_REG, al 
    in al, RTC_DATA_REG
    mov [s], al

    /* Reads minutes */
    mov al, MINUTE
    out RTC_ADDR_REG, al 
    in al, RTC_DATA_REG
    mov [m], al

    /* Reads hour */
    mov al, HOUR
    out RTC_ADDR_REG, al 
    in al, RTC_DATA_REG
    mov [h], al

rtc_int_handle_asm_end:
    ret
