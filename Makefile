CC=gcc
CFLAGS=-O3 -DFRAME_DELAY=30000

LIBS=-lX11

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

xchomp: contact.o drivers.o main.o props.o resources.o demo.o maze.o status.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o xchomp
