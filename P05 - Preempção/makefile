OBJS	= ppos_core.o queue.o preempcao.o
SOURCE	= ppos_core.c queue.c preempcao.c
HEADER	= ppos_data.h ppos.h queue.h
OUT	= preempcao
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

preempcao.o: preempcao.c
	$(CC) $(FLAGS) preempcao.c

# Ativa flag de Debug
debug: all
debug: DEBUG += -DDEBUG
debug: purge
	@echo "Executavel com simbolos de Debug..."

#compactar arquivos
compact:
	@echo "Compactando arquivos..."
	tar -cvzf preempcao_lcmc20.tar.gz $(SOURCE) $(HEADER) makefile README.md

# Remove arquivos temporarios
clean:
	@echo "Removendo arquivos temporarios..."
	rm -f $(OBJS)

# Remove arquivos temporarios e o executavel
purge: clean
	@echo "Removendo executavel..."
	rm -f *~ $(OUT)
