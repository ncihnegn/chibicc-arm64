CFLAGS=-std=c11 -g -static -Wall

chibicc: main.c
	$(CC) -o $@ $? $(LDFLAGS)

test: chibicc
	./test.sh

clean:
	rm -f chibicc *.o *~ tmp*

.PHONY: test clean
