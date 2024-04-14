OBJS = queue.o testafila.o
OUT	= testafila
CC = gcc
FLAGS = -g -c -Wall -std=c99

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

testafila.o: testafila.c queue.h
	$(CC) $(FLAGS) testafila.c

queue.o: queue.c queue.h
	$(CC) $(FLAGS) queue.c 

clean:
	rm -f $(OBJS)
purge: clean
	rm -f *~ $(OUT)