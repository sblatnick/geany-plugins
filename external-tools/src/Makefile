CC=gcc

all:
	$(CC) -c external-tools.c -fPIC `pkg-config --cflags geany`
	$(CC) external-tools.o -o external-tools.so -shared `pkg-config --libs geany`
install:
	cp external-tools.so ~/.config/geany/plugins/
clean:
	rm -f *.o *.so
