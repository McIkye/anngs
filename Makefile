
MCU=atmega1280
FREQ=16000000

PROG=	anngs
SRCS=	anngs.c gui.c navi.c pacman.c
#	drivers
SRCS+=	eeprom.c ads7846.c hx8347.c nmea_uart.c	# zigbee vs1053 sdcard
#
SRCS+=	delay.S

CPPFLAGS+= -I${.CURDIR} -I. -DDEBUG
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
LDFLAGS	= -Wl,-Map=$(PROG).map,--cref
CLEANFILES+=${PROG}.map

.include "../bsd.avr.mk"
