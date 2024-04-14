// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.6 -- Maio de 2023

// estruturas de dados necessárias neste projeto
#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

// funções para corpos de tarefas ==============================================

task_t  *scheduler ()
{
    task_t *taskNext;
    
    // Retorna a primeira tarefa da fila de prontas
    taskNext = readyTasksQueue;
    if(taskNext == NULL)
        return NULL;
    
    #ifdef DEBUG
    printf("scheduler: tarefa id: %d ===> dispatcher\n", taskNext->id);
    #endif
    queue_remove((queue_t**)&readyTasksQueue, (queue_t*)taskNext);
    return taskNext;
}

void dispatcherBody (void* arg) 
{
    // Remove a tarefa dispatcher da fila de prontas para evitar que ative a si próprio
    queue_remove((queue_t**)&readyTasksQueue, (queue_t*)&taskDispatcher);
    userTasks--;
    // Verificar o tamanho da fila de tarefas prontas
    while(userTasks > 0 )
    {
        task_t* taskNext = scheduler(); // próxima tarefa a ser executada
        if (taskNext != NULL) 
        {
            taskNext->status = EXECUTANDO; // seta o status da tarefa como rodando
            task_switch(taskNext); // transfere o contexto para a próxima tarefa selecionada
            switch (taskNext->status)
            {
            case PRONTA:
                
                break;
            case SUSPENSA:
                break;
            case ENCERRADA:
                free(taskNext->context.uc_stack.ss_sp);
                userTasks--;
                break;
            }
        }
    }

    // Encerra a tarefa dispatcher
    task_exit(0);
}

// funções gerais ==============================================================


// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
    setvbuf(stdout, 0, _IONBF, 0);
    
    lastTaskId = 0;
    userTasks = 0;
    taskMain.id = lastTaskId; // Seta o valor da tarefa Main em 0 e depois atualiza   
    taskMain.status = EXECUTANDO; // Seta o status da tarefa Main como rodando
    taskMain.prev = NULL; // Seta o valor do ponteiro anterior da tarefa Main para ela mesma
    taskMain.next = NULL; // Seta o valor do ponteiro próximo da tarefa Main para ela mesma
    taskActual = &taskMain; // tarefa atual aponta para a tarefa Main

    task_init(&taskDispatcher, (void *)dispatcherBody, NULL);
    #ifdef DEBUG
        printf ("ppos_init: PingPongOS inicializado, tarefa Main e tarefa Dispatcher criadas!\n");
    #endif
}

// gerência de tarefas =========================================================

// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
int task_init (task_t *task, void  (*start_func)(void *), void *arg)
{
    // Verificar se a o ponteiro da tarefa é valido
    if (task == NULL) {
        fprintf(stderr, "task_init - ERRO: Tarefa invalida \n");
        return (ERRCREATE);
    }

    // Inicializa o contexto da tarefa
    getcontext(&(task->context));

    // Aloca e inicializa a pilha da tarefa
    char *stack; 
    if((stack = malloc(STACKSIZE*sizeof(char))) == NULL)
    {
        fprintf(stderr,"task_init - ERRO: Não foi possível criar a pilha da tarefa");
        return(ERRMALLOC);
    }

    // Inicializa o contexto da tarefa    
    task->context.uc_stack.ss_sp = stack; // Stack Pointer
    task->context.uc_stack.ss_size = STACKSIZE; // Tamanho da pilha
    task->context.uc_stack.ss_flags = 0; // zero indica que a pilha deve crescer para baixo
    task->context.uc_link = 0; // contexto de retorno
    task->status = PRONTA;
    lastTaskId++;
    task->id = lastTaskId; // Define ID da tarefa
    // task->prev = NULL; // Seta o valor do ponteiro anterior da tarefa para NULL
    // task->next = NULL; // Seta o valor do ponteiro próximo da tarefa para NULL

    // Cria o contexto da tarefa
    makecontext(&(task->context), (void *)start_func, 1, arg); 

    // Adiciona a tarefa na fila de prontas
    queue_append((queue_t**)&readyTasksQueue, (queue_t*)task);

    // Incrementa o número de tarefas do usuário
    userTasks++;

    // DEBUGGER
    #ifdef DEBUG
        printf ("task_init: iniciada tarefa id: %d\n", task->id);
    #endif
    
    // Retorna o ID da tarefa
    return task->id;
}			

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id ()
{
    #ifdef DEBUG
        printf ("task_id: id da tarefa atual: %d\n", taskActual->id);
    #endif
    return taskActual->id;
}

// Termina a tarefa corrente ceom um status de encerramento
void task_exit (int exit_code)
{
    // DEBUGGER, antes devido a troca de contexto
    #ifdef DEBUG
        printf ("task_exit: encerrada tarefa id: %d\n", taskActual->id);
    #endif

    taskActual->status = ENCERRADA;
    // Retorna a execução para o Dispatcher, se o despacher for encerrado, retorna para a tarefa Main
    if(taskActual == &taskDispatcher)
    {
        #ifdef DEBUG
        printf ("task_exit: dispatcher encerrando\n");
        #endif
        exit(0);
    }
    else
    {
        #ifdef DEBUG
        printf ("task_exit: retornando a tarefa dispatcher\n");
        #endif
        task_switch(&taskDispatcher);
    }
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{   
    // Verifica se a tarefa existe
    if(task == NULL)
    {
        fprintf(stderr,"Erro ao alterar a tarefa atual");
        exit(ERRSWITCH);
    }

    // Troca de contexto
    task_t *taskToSwitch = taskActual;
    taskActual = task;
    // DEBUGGER, antes devido a troca de contexto
    #ifdef DEBUG
        printf ("task_switch: trocou a tarefa atual id:%d ===> id: %d\n", taskToSwitch->id, task->id);
    #endif
    swapcontext(&(taskToSwitch->context), &(taskActual->context));

    // Sucesso na troca
    return 0;

}

// suspende a tarefa atual,
// transferindo-a da fila de prontas para a fila "queue"
void task_suspend (task_t **queue);

// acorda a tarefa indicada,
// trasferindo-a da fila "queue" para a fila de prontas
void task_awake (task_t *task, task_t **queue) ;

// operações de escalonamento ==================================================

// a tarefa atual libera o processador para outra tarefa
void task_yield ()
{   
    // Coloca a tarefa atual no final da fila de prontas
    if(taskActual->id != 0)
    {
        queue_append((queue_t**)&readyTasksQueue, (queue_t*)taskActual);
        taskActual->status = PRONTA;

        // DEBUGGER
        #ifdef DEBUG
        printf("task_yield: tarefa %d ===> Fila de tarefas prontas\n", taskActual->id);
        #endif
    }
    
    // DEBUGGER
    #ifdef DEBUG
        printf("task_yield: execução retornou para a tarefa dispatcher\n");
    #endif
    // Retorna a execução para o Dispatcher
    task_switch(&taskDispatcher);

}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) ;

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) ;

// operações de gestão do tempo ================================================

// retorna o relógio atual (em milisegundos)
unsigned int systime () ;

// suspende a tarefa corrente por t milissegundos
void task_sleep (int t) ;

// operações de sincronização ==================================================

// a tarefa corrente aguarda o encerramento de outra task
int task_wait (task_t *task) ;

// inicializa um semáforo com valor inicial "value"
int sem_init (semaphore_t *s, int value) ;

// requisita o semáforo
int sem_down (semaphore_t *s) ;

// libera o semáforo
int sem_up (semaphore_t *s) ;

// "destroi" o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s) ;

// inicializa um mutex (sempre inicialmente livre)
int mutex_init (mutex_t *m) ;

// requisita o mutex
int mutex_lock (mutex_t *m) ;

// libera o mutex
int mutex_unlock (mutex_t *m) ;

// "destroi" o mutex, liberando as tarefas bloqueadas
int mutex_destroy (mutex_t *m) ;

// inicializa uma barreira para N tarefas
int barrier_init (barrier_t *b, int N) ;

// espera na barreira
int barrier_wait (barrier_t *b) ;

// destrói a barreira, liberando as tarefas
int barrier_destroy (barrier_t *b) ;

// operações de comunicação ====================================================

// inicializa uma fila para até max mensagens de size bytes cada
int mqueue_init (mqueue_t *queue, int max, int size) ;

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg) ;

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg) ;

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) ;

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue) ;