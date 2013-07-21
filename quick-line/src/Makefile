CC=gcc

all:
	$(CC) -c quick-line.c -fPIC `pkg-config --cflags geany`
	$(CC) quick-line.o -o quick-line.so -shared `pkg-config --libs geany`
install:
	cp quick-line.so ~/.config/geany/plugins/
clean:
	rm -f *.o *.so
