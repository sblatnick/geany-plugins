CC=gcc

all:
	$(CC) -c tab-utils.c -fPIC -Wwrite-strings `pkg-config --cflags geany`
	$(CC) tab-utils.o -o tab-utils.so -shared `pkg-config --libs geany`
install:
	cp tab-utils.so ~/.config/geany/plugins/
clean:
	rm -f *.o *.so
