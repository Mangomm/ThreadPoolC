Debug = -g -Wall

ALL:test

test:test.o ThreadPool.o
	gcc $^ -o test -lpthread

test.o:test.c
	gcc $(Debug) -c $< -o $@

ThreadPool.o:ThreadPool.c
	g++ $(Debug) -c $< -o $@

clean:
	-rm -rf test.o \
	ThreadPool.o \
	test

.PHONY:ALL