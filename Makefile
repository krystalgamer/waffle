PROG=proj

SRCS = proj.c vbe.c mouse.c kbc.c util.c timer.c

CPPFLAGS += -pedantic #-D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>
