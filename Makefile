PROG=lab5
SRCS = lab5.c vbe.c util.c timer.c mouse.c kbc.c

CPPFLAGS += -pedantic

DPADD += ${LIBLCF} ${LIBLM}
LDADD += -llcf -llm

.include <minix.lcom.mk>
