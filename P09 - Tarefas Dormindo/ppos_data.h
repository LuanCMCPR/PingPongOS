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
#define ERRCREATE -3
#define ERRPRIO -4
#define ERRHANDLER -5
#define ERRTIMER -6
#define ERRSIGNAL -7
#define ERRQUEUE -8
#define ERRTASK -9


// status de Tarefas   
#define PRONTA 1 // Tarefa Pronta
#define SUSPENSA 2 // Tarefa Suspensa
#define EXECUTANDO 3 // Tarefa Rodando
#define ENCERRADA 4 // Tarefa Terminada
#define DORMINDO 5 // Tarefa Dormindo

// Preempção
#define QTDTICKS 20 // Quantum de Ticks


// Bibliotecas
#include <ucontext.h>	
#include <signal.h>
#include <sys/time.h>
#include "queue.h"


// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next, *waitQueue;		// Ponteiros para usar em filas
  ucontext_t context;			      // Contexto armazenado da tarefa
  short status;			            // Pronta, Rodando, Suspensa, ...
  short static_prio, dynamic_prio;// Prioridade
  short isSystemTask;             // Tarefa do Sistema
  int id;				                // Identificador da tarefa
  int quantum;                    // Quantum de Ticks, para preempção
  int procTime, execTime, actvs, sleepTime; // Tempo de Processador, Tempo de Execução e Ativações
  int exitCode;                   // Código de Saída
} task_t;

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
int lastTaskId, userTasks; // Controle dos IDs das Tarefas
int systemTime; // Tempo de Execução do Sistema
task_t taskMain, taskDispatcher, *taskActual; // Tarefas Essenciais ao sistema
task_t *readyTasksQueue, *sleepingTasksQueue; // Filas de Gerenciamento das Tarefas
struct sigaction action; // Trata o sinal de timer
struct itimerval timer; // Timer do sistema para preempção

#endif
