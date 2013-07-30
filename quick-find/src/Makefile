CC=gcc

all:
	$(CC) -c quick-find.c -fPIC `pkg-config --cflags geany`
	$(CC) quick-find.o -o quick-find.so -shared `pkg-config --libs geany`
install:
	cp quick-find.so ~/.config/geany/plugins/
clean:
	rm -f *.o *.so
