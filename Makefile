CC = clang
CFLAGS = -Wall -std=c89 -pedantic
OFLAGS = -Wall -c
SRCS = ioutil.c linkedlist.c tokenizer.c executil.c sh.c
OBJS = ioutil.o linkedlist.o tokenizer.o executil.o sh.o
TARGETS = clean sh
LIBS = 

default: all

all: $(TARGETS)

sh: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(OBJS)

$(SRCS):
	$(CC) $(OFLAGS) $*.c

clean:
	$(RM) *.o sh
