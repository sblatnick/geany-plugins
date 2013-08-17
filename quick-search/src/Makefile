CC=gcc

all:
	$(CC) -c quick-search.c -fPIC `pkg-config --cflags geany`
	$(CC) quick-search.o -o quick-search.so -shared `pkg-config --libs geany`
install:
	cp quick-search.so ~/.config/geany/plugins/
clean:
	rm -f *.o *.so
