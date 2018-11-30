PROG=proj

#DIRS += interrupts

.PATH: ${.CURDIR}/interrupts

SRCS += proj.c vbe.c util.c window.c
SRCS += mouse.c kbc.c timer.c 

CPPFLAGS += -pedantic #-D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>
