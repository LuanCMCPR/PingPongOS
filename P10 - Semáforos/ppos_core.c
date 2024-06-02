// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.6 -- Maio de 2023

// estruturas de dados necessárias neste projeto
#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

// funções para corpos de tarefas =============s=================================

void setUpTimer()
{
    // Ajusta valores do temporizador
    timer.it_value.tv_sec = 0;      // Primeiro disparo, em segundos
    timer.it_value.tv_usec = 1000;      // Primeiro disparo, em micro-segundos
    timer.it_interval.tv_sec = 0;   // Disparos subsequentes, em segundos
    timer.it_interval.tv_usec = 1000;   // Disparos subsequentes, em micro-segundos

    // Arma o temporizador ITIMER_REAL
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        fprintf(stderr, "setUpTimer - ERRO em iniciar o temporizador\n");
        exit(ERRTIMER);
    }
    //DEBUGGER
    #ifdef DEBUG
        printf("setUpHandler: Tratador de sinal iniciado\n");
    #endif
}

void timerHandler()
{
    // DEBUGGER
    // #ifdef DEBUG
    //     printf("timerHandler: Entrando no Timer Handler\n");
    // #endif

    // Incrementa o tempo do sistema
    systemTime++;

    // Se a tarefa atual for uma tarefa do sistema, não decrementa o quantum
    if(taskActual->isSystemTask == 1)
    {
        #ifdef DEBUG
            printf("timerHandler: tarefa id: %d ===> Tarefa do Sistema\n", taskActual->id);
        #endif
        return; 
    }

    // Decrementa o quantum da tarefa atual
    taskActual->quantum--;
    // DEBBUGER
    // #ifdef DEBUG
    //     printf("timerHandler: tarefa id: %d ===> Quantum: %d\n", taskActual->id, taskActual->quantum);
    // #endif

    // Se o quantum da tarefa atual for 0, a tarefa é colocada no final da fila de prontas
    if(taskActual->quantum == 0)
    {
        // #ifdef DEBUG
        //     printf("timerHandler: tarefa id: %d ===> Fim do Quantum\n", taskActual->id);
        //     printf("timerHandler: tarefa id: %d ===> Fila de tarefas prontas\n", taskActual->id);
        // #endif
        task_yield(); // Retorna a execução para o Dispatcher
    }
    
}

void setUpHandler()
{
    // Registra a ação para o sinal de timer SIGALRM (sinal do timer)
    action.sa_handler = timerHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        fprintf(stderr, "setUpHandler - Erro em iniciar o tratador de sinal\n");
        exit(ERRHANDLER);
    }
    
    //DEBUGGER
    // #ifdef DEBUG
    //     printf("setUpHandler: Tratador de sinal iniciado\n");
    // #endif
}

task_t* scheduler()
{
    #ifdef DEBUG
        printf("scheduler: Entrando no Scheduler\n");
    #endif

    task_t *taskNext = readyTasksQueue;

    // Se a fila de prontas estiver vazia, retorna NULL
    if(taskNext == NULL)
        return NULL;

    task_t *taskPriority = taskNext;

    do
    {
        // Se a tarefa atual tiver prioridade maior que a tarefa com maior prioridade (-20 sendo a maior), a tarefa atual passa a ser a tarefa com maior prioridade
        if(taskPriority->dynamic_prio > taskNext->dynamic_prio)
            taskPriority = taskNext;
                    
        // Vai para a próxima tarefa
        taskNext = taskNext->next;
    }
    while(taskNext != readyTasksQueue);


    do
    {
    	// Se a tarefa tiver prioridade maior que -20, a prioridade é decrementada em 1, fazendo o aging e evitando Starvation
        if(taskNext->dynamic_prio > -20)
        {
            taskNext->dynamic_prio--;
            #ifdef DEBUG
                printf("scheduler: tarefa id: %d, prioridade: %d ===> aging\n", taskNext->id, taskNext->dynamic_prio);
            #endif
        }

        // Vai para a próxima tarefa
        taskNext = taskNext->next;
    }
    while(taskNext != readyTasksQueue); 


    #ifdef DEBUG
    printf("scheduler: tarefa id: %d escolhida, prioridade: %d ===> dispatcher\n", taskPriority->id, taskPriority->dynamic_prio);
    #endif


    // Como a tarefa foi escolhida, a prioridade dinâmica é reseta para a prioridade estática. Evitando Starvation
    taskPriority->dynamic_prio = taskPriority->static_prio;

    return taskPriority;
}

void task_print(void *task)
{
    task_t *taskPrint = (task_t*)task;
    printf("%d,", taskPrint->id);
}

void dispatcherBody (void* arg) 
{
    // Remove a tarefa dispatcher da fila de prontas para evitar que ative a si próprio
    queue_remove((queue_t**)&readyTasksQueue, (queue_t*)&taskDispatcher);
    // userTasks--; 
    // Verificar o tamanho da fila de tarefas prontas
    while(userTasks > 0 )
    {
        #ifdef DEBUG
            printf("dispatcherBody: Dispatcher em execução\n");
            printf("dispatcherBody: %d tarefas prontas\n", userTasks);
        #endif

        /* Verificar fila */
        if(sleepingTasksQueue != NULL)
        {
            task_t * sleepAux = sleepingTasksQueue;
            do
            {
                if(sleepAux->sleepTime <= systime())
                {
                    task_t *next = sleepAux->next;
                    task_awake(sleepAux, &sleepingTasksQueue);
                    // DEBBUG
                    #ifdef DEBUG
                        printf("dispatcherBody: tarefa id: %d adormecida ===> Fila de tarefas prontas\n", taskAux->id);
                    #endif
                    sleepAux = next;
                }
                else
                    sleepAux = sleepAux->next;

                if(sleepingTasksQueue == NULL)
                    break;

            } while (sleepAux != sleepingTasksQueue);
        }
        
        unsigned int timeBefore, timeAfter;
        task_t* taskNext = scheduler(); // próxima tarefa a ser executada

        if (taskNext != NULL) 
        {
            queue_remove((queue_t**)&readyTasksQueue, (queue_t*)taskNext); // remove tarefa da fila de prontas
            taskNext->status = EXECUTANDO; // seta o status da tarefa como rodando
            taskNext->quantum = QTDTICKS; // seta o quantum da tarefa
            timeBefore = systime();
            task_switch(taskNext); // transfere o contexto para a próxima tarefa selecionada
            timeAfter = systime();
            taskNext->procTime += timeAfter - timeBefore;

            switch (taskNext->status)
            {
            case PRONTA:
                queue_append((queue_t**)&readyTasksQueue, (queue_t*)taskNext);
                #ifdef DEBUG
                    printf("dispatcherBody: tarefa id: %d ===> Fila de tarefas prontas\n", taskNext->id);
                #endif
                break;
            case SUSPENSA:

                break;
            case ENCERRADA:
                if(queue_size((queue_t*)taskNext->waitQueue) > 0)
                {
                    task_t *taskAux1 = taskNext->waitQueue;
                    task_t *taskAux2;
                    while(taskNext->waitQueue != NULL)
                    {
                        taskAux2 = taskAux1->next;
                        task_awake(taskAux1, &taskNext->waitQueue);
                        taskAux1 = taskAux2;

                    }
                }
                free(taskNext->context.uc_stack.ss_sp);
                #ifdef DEBUG
                    printf("dispatcherBody: tarefa id: %d encerrada\n", taskNext->id);
                #endif
                userTasks--;
                break;
            }
        }
    }

    // Encerra a tarefa dispatcher quando não houver mais tarefas
    task_exit(0);
}

// funções gerais ==============================================================

// Operações atômicas ==========================================================

// Entra na seção critica
void enter_cs (int *lock)
{
  // atomic OR (Intel macro for GCC)
  while (__sync_fetch_and_or (lock, 1)) ;   // busy waiting
}
 
// Sai da seção critica
void leave_cs (int *lock)
{
  (*lock) = 0 ;
}


// gerência de tarefas =========================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
    setvbuf(stdout, 0, _IONBF, 0);
    
    lastTaskId = 0;
    userTasks = 0;
    taskMain.id = lastTaskId; // Seta o valor da tarefa Main em 0 e depois atualiza   
    taskMain.status = PRONTA; // Seta o status da tarefa Main como rodando
    taskMain.prev = NULL; // Seta o valor do ponteiro anterior da tarefa Main para ela mesma
    taskMain.next = NULL; // Seta o valor do ponteiro próximo da tarefa Main para ela mesma
    taskMain.execTime = systime(); // Tempo de execução
    taskMain.procTime = 0; // Tempo de processador
    taskMain.actvs = 1; // Número de ativações
    taskMain.dynamic_prio = 0; // Prioridade dinâmica
    taskMain.static_prio = 0; // Prioridade estática
    taskMain.quantum = QTDTICKS; // Quantum de Ticks
    taskMain.isSystemTask = 0; // Tarefa do usuário
    taskActual = &taskMain; // tarefa atual aponta para a tarefa Main
    
    queue_append((queue_t**)&readyTasksQueue, (queue_t*)&taskMain); // Adiciona a tarefa Main na fila de prontas
    #ifdef DEBUG
        printf ("tarefa Main id => 0\n");
    #endif

    // Inicializa o tempo do sistema e o temporizador
    systemTime = 0;
    setUpTimer();
    setUpHandler();

    // Cria a tarefa Dispatcher
    task_init(&taskDispatcher, (void *)dispatcherBody, NULL);
    taskDispatcher.isSystemTask = 1; // Seta a tarefa Dispatcher como tarefa do sistema
    #ifdef DEBUG
        printf ("tarefa Dispatcher id => 1\n");
        printf ("ppos_init: PingPongOS inicializado, tarefa Main e tarefa Dispatcher criadas!\n");
    #endif
}

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
    task->context.uc_stack.ss_flags = 0; // Zero indica que a pilha deve crescer para baixo
    task->context.uc_link = 0; // Contexto de retorno
    task->status = PRONTA; // Tarefa pronta para execução
    lastTaskId++;
    task->id = lastTaskId; // Define ID da tarefa
    task->static_prio = 0; // Prioridade estática
    task->dynamic_prio = 0; // Prioridade dinâmica
    task->isSystemTask = 0; // Tarefa do usuário
    task->actvs = 0; // Número de ativações
    task->procTime = 0; // Tempo de processador
    task->execTime = systime(); // Tempo de execução

    // Cria o contexto da tarefa
    makecontext(&(task->context), (void *)start_func, 1, arg); 

    // Adiciona a tarefa na fila de prontas
    queue_append((queue_t**)&readyTasksQueue, (queue_t*)task);

    // Incrementa o número de tarefas do usuário
    userTasks++;

    // DEBUGGER
    #ifdef DEBUG
        printf ("task_init: %d tarefas prontas\n", task->id);
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

    // Atualiza o tempo de execução da tarefa
    taskActual->execTime = systime() - taskActual->execTime;
    taskActual->status = ENCERRADA;
    taskActual->exitCode = exit_code;
    // Retorna a execução para o Dispatcher, se o despacher for encerrado, retorna para a tarefa Main
    if(taskActual == &taskDispatcher)
    {
        #ifdef DEBUG
        printf ("task_exit: dispatcher encerrando\n");
        #endif
        free(taskActual->context.uc_stack.ss_sp); // Libera a pilha do dispatcher
        printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", taskActual->id, taskActual->execTime, taskActual->procTime, taskActual->actvs);
        exit(0);
    }
    else
    {
        #ifdef DEBUG
        printf ("task_exit: encerrada tarefa id: %d\n", taskActual->id);
        printf ("task_exit: retornando a tarefa dispatcher\n");
        #endif
        printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", taskActual->id, taskActual->execTime, taskActual->procTime, taskActual->actvs);
        task_switch(&taskDispatcher); // Retorna a execução para o Dispatcher
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
    // Incrementa o número de ativações da tarefa
    taskActual->actvs++;
    swapcontext(&(taskToSwitch->context), &(taskActual->context));

    // Sucesso na troca
    return 0;

}

// suspende a tarefa atual,
// transferindo-a da fila de prontas para a fila "queue"
void task_suspend (task_t **queue)
{   
    
    queue_append((queue_t**)&readyTasksQueue, (queue_t*)taskActual);
    
    queue_remove((queue_t**)&readyTasksQueue, (queue_t*)taskActual);
    
    taskActual->status = SUSPENSA;
    
    if(queue != NULL)
    {   
        queue_append((queue_t**)queue, (queue_t*)taskActual);
    #ifdef DEBUG
        printf("task_suspend: tarefa %d ===> Fila de tarefas suspensas\n", taskActual->id);
    #endif
    }

    task_switch(&taskDispatcher);

}

// acorda a tarefa indicada,
// trasferindo-a da fila "queue" para a fila de prontas
void task_awake (task_t *task, task_t **queue)
{

    if(queue != NULL)
    {
        if(queue_remove((queue_t**)queue, (queue_t*)task) == -4)
        task->status = PRONTA;
        queue_append((queue_t**)&readyTasksQueue, (queue_t*)task);
    }   

    return;

    #ifdef DEBUG
        printf("task_awake: tarefa %d ===> Fila de tarefas prontas\n", task->id);
    #endif
    
}

// operações de escalonamento ==================================================

// a tarefa atual libera o processador para outra tarefa
void task_yield ()
{   
    // Coloca a tarefa atual no final da fila de prontas
    // if(taskActual->id != 0)
    // {
    //     // queue_append((queue_t**)&readyTasksQueue, (queue_t*)taskActual);
    //     taskActual->status = PRONTA;
    //     // DEBUGGER
    //     #ifdef DEBUG
    //     printf("task_yield: tarefa %d ===> Fila de tarefas prontas\n", taskActual->id);
    //     #endif
    // }

    taskActual->status = PRONTA;
    // DEBUGGER
    #ifdef DEBUG
        printf("task_yield: tarefa %d ===> Fila de tarefas prontas\n", taskActual->id);
    #endif
    
    // DEBUGGER
    #ifdef DEBUG
        printf("task_yield: execução retornou para a tarefa dispatcher\n");
    #endif
    // Retorna a execução para o Dispatcher
    task_switch(&taskDispatcher);

}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio)
{
    if(prio < -20 || prio > 20)
    {
        fprintf(stderr,"task_setprio: Prioridade inválida\n");
        exit(ERRPRIO);
    }
    // Se a tarefa é NULA, ajusta a prioridade da tarefa atual
    if(task == NULL)
    {
        taskActual->static_prio = prio;
        taskActual->dynamic_prio = prio;
        #ifdef DEBUG
            printf("task_setprio: tarefa atual id:%d ===> recebeu prioridade %d\n",taskActual->id, prio);
        #endif
        return;
    }

    // Ajusta a prioridade da tarefa
    task->static_prio = prio;
    task->dynamic_prio = prio;
    #ifdef DEBUG
        printf("task_setprio: tarefa id:%d ===> recebeu prioridade %d\n",task->id, prio);
    #endif
    return;
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task)
{
    // Se a tarefa pe NULA, retorna a prioridade da tarefa atual
    if(task == NULL)
    {
        return taskActual->static_prio;
        #ifdef DEBUG
            printf("task_getprio: tarefa atual id:%d ===> possui prioridade %d\n",taskActual->id, task->static_prio);
        #endif
    }

    // Retorna a prioridade da tarefa
    #ifdef DEBUG
            printf("task_getprio: tarefa id:%d ===> possui prioridade %d\n",task->id, task->static_prio);
    #endif
    return task->static_prio;
}
// operações de gestão do tempo ================================================

// retorna o relógio atual (em milisegundos)
unsigned int systime ()
{
    return systemTime;
}

// suspende a tarefa corrente por t milissegundos
void task_sleep (int t)
{
    taskActual->sleepTime = systime() + t;
    task_suspend(&sleepingTasksQueue);
}

// operações de sincronização ==================================================

// a tarefa corrente aguarda o encerramento de outra task
int task_wait (task_t *task)
{
    if(task == NULL)
        return -1;
    
    if(task->status == ENCERRADA)
        return task->exitCode;

    task_suspend(&(task->waitQueue));

    return task->exitCode;
}

// inicializa um semáforo com valor inicial "value"
int sem_init (semaphore_t *s, int value)
{
    // Verificar se o semáforo existe
    if(s == NULL)
        return -1;

    // Seta o valor do semáforo
    s->counter = value;
    s->queue = NULL;
    s->lock = 0;
    return 0;

}

// requisita o semáforo
int sem_down (semaphore_t *s)
{
    // Verificar se o semáforo existe
    if(s == NULL)
        return -1;
    
    // while (test_and_set(&(s->lock)));
    // Entra na seção crítica
    enter_cs(&(s->lock));

    // Decrementa o contador do semáforo
    s->counter--;

    // Se o contador for menor que 0, a tarefa é bloqueada
    if(s->counter < 0)
    {
        leave_cs(&(s->lock));
        task_suspend(&(s->queue));
    }
    else
        leave_cs(&(s->lock));

    return 0;

}

// libera o semáforo
int sem_up (semaphore_t *s)
{
    // Verificar se o semáforo existe
    if(s == NULL)
        return -1;

    // while (test_and_set(&(s->lock)));
    // Entra na seção crítica
    enter_cs(&(s->lock));

    // Incrementa o contador do semáforo
    s->counter++;

    // Se o contador for maior que 0, acorda a tarefa
    if(s->counter <= 0)
    {
        task_awake(s->queue, &s->queue);
    }

    leave_cs(&(s->lock));

    return 0;

}

// "destroi" o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s)
{
    // Verifica se o semáforo existe
    if(s == NULL)
        return -1;

    // Entra na seção crítica
    enter_cs(&(s->lock));

    // Acorda todas as tarefas bloqueadas
    while(s->queue != NULL)
    {
        task_t *taskAux = s->queue;
        task_awake(taskAux, &s->queue);
    }

    leave_cs(&(s->lock));

    return 0;
}

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
