all:main

main:main.o sqstack.o queue.o
	$(CC) $^ -o $@

clean:
	rm *.o main -rf