PROG=proj

SRCS += proj.c vbe.c util.c 

.PATH: ${.CURDIR}/interrupts
SRCS += mouse.c kbc.c timer.c rtc.c keyboard.c serial_port.c queue.c kbc_ih.S rtc_ih.S

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

.PATH: ${.CURDIR}/system_info
SRCS += system_info.c

.PATH: ${.CURDIR}/background_chooser
SRCS += background_chooser.c

.PATH: ${.CURDIR}/painter
SRCS += painter.c

.PATH: ${.CURDIR}/calculator
SRCS += calculator.c

.PATH: ${.CURDIR}/login
SRCS += login.c

.PATH: ${.CURDIR}/multi_painter
SRCS += multi_painter.c

.PATH: ${.CURDIR}/guess_painter
SRCS += guess_painter.c

.PATH: ${.CURDIR}/example
SRCS += example.c

CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf 

.include <minix.lcom.mk>
