CC=gcc

all:
	$(CC) -c hide-menu.c -fPIC `pkg-config --cflags geany`
	$(CC) hide-menu.o -o hide-menu.so -shared `pkg-config --libs geany`
install:
	cp hide-menu.so ~/.config/geany/plugins/
clean:
	rm -f *.o *.so
