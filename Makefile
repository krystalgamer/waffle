PROG=proj

SRCS += proj.c vbe.c util.c 

.PATH: ${.CURDIR}/interrupts
SRCS += mouse.c kbc.c timer.c rtc.c keyboard.c

.PATH: ${.CURDIR}/font
SRCS += letters.c

.PATH: ${.CURDIR}/window
SRCS += window.c wnd_elements.c wnd_state_machine.c wnd_tskbar.c context_menu.c

.PATH: ${.CURDIR}/screensaver
SRCS += screensaver.c

.PATH: ${.CURDIR}/terminus
SRCS += terminus.c

.PATH: ${.CURDIR}/file_browser
SRCS += file_browser.c

CPPFLAGS += -pedantic # -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf 

.include <minix.lcom.mk>
