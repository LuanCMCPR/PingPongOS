// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

// tamanho da pilha das tarefas
#define STACKSIZE 64*1024

// codigos de erro
#define ERRSWITCH -1
#define ERRMALLOC -2

// status de Tarefas
// #define NOVATAREFA 0 // Nova Tarefa   
#define PRONTA 1 // Tarefa Pronta
#define SUSPENSA 2 // Tarefa Suspensa
#define RODANDO 3 // Tarefa Rodando
#define ENCERRADA 4 // Tarefa Terminada

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
// preencher quando necessário
} mqueue_t ;

// variáveis globais
int lastTaskId;
task_t taskMain, *taskActual;

#endif
