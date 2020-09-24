CC = clang
CFLAGS = -Wall 
OFLAGS = -Wall -c
SRCS = ioutil.c linkedlist.c tokenizer.c executil.c penn-sh.c
OBJS = ioutil.o linkedlist.o tokenizer.o executil.o penn-sh.o
TARGETS = clean penn-sh
LIBS = 

.PHONY: penn-sh


default: all

all: $(TARGETS)

penn-sh: $(OBJS) clean
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(OBJS)

$(SRCS):
	$(CC) $(OFLAGS) $*.c

clean:
	$(RM) *.o penn-sh
