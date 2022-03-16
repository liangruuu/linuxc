CFLAGS+=-Wall

all:tetris
tetris:tetris.o drawlib.o
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm *.o tetris -rf
