CC=gcc

CFLAGS=-lpthread -lm

DEPS_PROG=perfect_number.c

program:
	$(CC) $(DEPS_PROG) $(CFLAGS)
	@echo "Usage: ./a.out <value_of_n>"

clean:
	rm -rf *.o *.out