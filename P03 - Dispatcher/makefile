OBJS	= ppos_core.o queue.o dispatcher.o
SOURCE	= ppos_core.c queue.c dispatcher.c
HEADER	= ppos_data.h ppos.h queue.h
OUT	= dispatcher
CC	 = gcc
FLAGS	 = -g -c -Wall

all: $(OBJS)
	@echo "Gerando executavel..."
	$(CC) -g $(OBJS) -o $(OUT) 

ppos_core.o: ppos_core.c
	$(CC) $(FLAGS) ppos_core.c

queue.o: queue.c
	$(CC) $(FLAGS) queue.c 

dispatcher.o: dispatcher.c
	$(CC) $(FLAGS) dispatcher.c 

# Ativa flag de Debug
debug: FLAGS += -DDEBUG
debug: all
	@echo "Executavel com simbolos de Debug..."

# Remove arquivos temporarios
clean:
	@echo "Removendo arquivos temporarios..."
	rm -f $(OBJS)

# Remove arquivos temporarios e o executavel
purge: clean
	@echo "Removendo executavel..."
	rm -f *~ $(OUT)