# MAKEFILE

CFLAGS += -I/usr/local/include
CFLAGS += -W -Wall -O2
//CFLAGS += -DTESTING=1

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


${OBJECTS}: shamir.h


test:
	./shamir-split -t 2 -n 3 -w pr23 -s 256
	echo 'copy/paste 2 of above'
	./shamir-combine -t 2
	echo
	./shamir-split -t 3 -n 5 -w pr35 -s 128
	echo 'copy/paste 3 of above'
	./shamir-combine -t 3

clean:
	rm -rf shamir-split shamir-combine shamir.1 shamir.1.html *.o
