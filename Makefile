# MAKEFILE

CFLAGS += -I/usr/local/include
CFLAGS += -W -Wall -O2

LDFLAGS += -L/usr/local/lib
LDFLAGS += -lgmp

OBJECTS += main.o
OBJECTS += shamir.o


all: shamir-split shamir-combine shamir.1 shamir.1.html

shamir-split: ${OBJECTS}
	$(CC) ${LDFLAGS}  -o shamir-split ${OBJECTS}
	strip shamir-split

shamir-combine: shamir-split
	ln -f shamir-split shamir-combine

shamir.1: shamir.manpage.xml
	xmltoman shamir.manpage.xml > shamir.1

shamir.1.html: shamir.manpage.xml
	xmlmantohtml shamir.manpage.xml > shamir.1.html


${OBJECTSS}: shamir.h

clean:
	rm -rf shamir-split shamir-combine shamir.1 shamir.1.html
