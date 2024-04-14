OBJS	= ppos_core.o queue.o pingpong-scheduler.o
SOURCE	= ppos_core.c queue.c pingpong-scheduler.c
HEADER	= ppos_data.h ppos.h queue.h
OUT	= scheduler
CC	 = gcc
FLAGS	 = -g -c -Wall
DEBUG =

all: $(OBJS)
all: purge
	$(CC) -g $(OBJS) -o $(OUT) 
	@echo "Gerando executavel..."

ppos_core.o: ppos_core.c
	$(CC) $(FLAGS) $(DEBUG) ppos_core.c

queue.o: queue.c
	$(CC) $(FLAGS) $(DEBUG) queue.c 

pingpong-scheduler.o: pingpong-scheduler.c
	$(CC) $(FLAGS) pingpong-scheduler.c

# Ativa flag de Debug
debug: all
debug: DEBUG += -DDEBUG
debug: purge
	@echo "Executavel com simbolos de Debug..."

#compactar arquivos
compact:
	@echo "Compactando arquivos..."
	tar -cvzf scheduler_lcmc20.tar.gz $(SOURCE) $(HEADER) makefile README.md

# Remove arquivos temporarios
clean:
	@echo "Removendo arquivos temporarios..."
	rm -f $(OBJS)

# Remove arquivos temporarios e o executavel
purge: clean
	@echo "Removendo executavel..."
	rm -f *~ $(OUT)
