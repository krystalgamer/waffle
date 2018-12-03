PROG=proj

SRCS += proj.c vbe.c util.c 

.PATH: ${.CURDIR}/interrupts
SRCS += mouse.c kbc.c timer.c rtc.c

.PATH: ${.CURDIR}/font
SRCS += letters.c

.PATH: ${.CURDIR}/window
SRCS += window.c wnd_elements.c wnd_state_machine.c wnd_tskbar.c

CPPFLAGS += -pedantic #-D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf 

.include <minix.lcom.mk>
