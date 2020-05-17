CC=gcc

CFLAGS=-lpthread -lm

DEPS_PROG=perfect_number.c

program:
	$(CC) $(DEPS_PROG) $(CFLAGS)

clean:
	rm -rf *.o *.out