CC = clang

CFLAGS = -Wall

OFLAGS = -Wall -c

TARGETS = clobber penn-sh

.PHONY: penn-sh




default: all

all: $(TARGETS) penn-sh

penn-sh: strutil.o ioutil.o penn-sh.c
	$(CC) $(CFLAGS) $^ -o $@

strutil.o:
	$(CC) $(OFLAGS) strutil.c

ioutil.o:
	$(CC) $(OFLAGS) -c ioutil.c

clean:
	$(RM) *.o

clobber: clean
	$(RM) penn-sh
