all:main

main:main.o cat.o dog.o animal_base.o
	$(CC) $^ -o $@

clean:
	rm *.o main -rf
