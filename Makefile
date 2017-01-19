# MAKEFILE

CFLAGS += -I/usr/local/include
CFLAGS += -W -Wall -O2
CFLAGS += ${TEST_ITEMS}

LDFLAGS += -L/usr/local/lib
LDFLAGS += -lgmp

OBJECTS += main.o
OBJECTS += shamir.o


all: shamir-split shamir-combine shamir.1 shamir.1.html

shamir-split: ${OBJECTS}
	$(CC) ${LDFLAGS}  -o "$@" ${OBJECTS}
	strip "$@"

shamir-combine: shamir-split
	ln -f shamir-split "$@"

shamir-test: ${OBJECTS}
	$(CC) ${LDFLAGS}  -o "$@" ${OBJECTS}
	strip "$@"

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

test-1:
	${MAKE} clean
	${MAKE} TEST_ITEMS='-DTESTING=1' shamir-test
	./shamir-test
	${MAKE} clean

clean:
	rm -rf shamir-split shamir-combine shamir-test shamir.1 shamir.1.html *.o
