CFLAGS=-std=c2x -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

chibicc: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): chibicc.h

test: chibicc
	./test.sh

clean:
	rm -f chibicc *.o *~ tmp*

.PHONY: test clean
