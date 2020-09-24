CC = clang
CFLAGS = -Wall 
OFLAGS = -Wall -c
SRCS = ioutil.c linkedlist.c tokenizer.c executil.c sh.c
OBJS = ioutil.o linkedlist.o tokenizer.o executil.o sh.o
TARGETS = clean sh
LIBS = 

.PHONY: penn-sh


default: all

all: $(TARGETS)

penn-sh: $(OBJS) clean
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(OBJS)

$(SRCS):
	$(CC) $(OFLAGS) $*.c

clean:
	$(RM) *.o sh
