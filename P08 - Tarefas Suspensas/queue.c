/*
    Luan Carlos Maia Cruz - UFPR
    Biblioteca de Filas para PingPongOS
*/

#include<stdio.h>
#include"queue.h"

#define ERRFILA -1 // Fila nao exister 
#define ERRVAZIA -2 // Fila esta vazia
#define ERRELEM -3 // Elemento nao existe
#define ERRPTC -4 // Elemento nao pertence a fila
#define ERRREP -5 // Elemento existe em outra fila

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue)
{
    // Fila vazia, não possui nenhum elemento
    if(queue == NULL)
        return 0;

    // A partir de 1 elemento
    int count = 1;
    queue_t *aux = queue;

    while(aux->next != queue)
    {
        count++;
        aux = aux->next;
    }

    return count;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*))
{
    queue_t *aux = queue;

    // Se fila é vazia não imprime nenhum elemento
    if(queue == NULL)
        return;

    // Imprime enquanto o próximo elemento enquanto o próximo é diferente da cabeça 
    while(aux->next != queue)
    {
        print_elem(aux);
        aux = aux->next;
    }
}

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_append (queue_t **queue, queue_t *elem)
{
    // Verifica se a fila existe
    if(queue == NULL)
    {
        fprintf(stderr,"ERRO: Fila nao existe\n");
        return ERRFILA;
    }

    // Verifica se o elemento existe
    if(elem == NULL)
    {
        fprintf(stderr,"ERRO: Elemento nao existe\n");
        return ERRELEM;
    }

    // Verifica se o elemento existe em outra fila,
    // testa se os ponteiros são nulos
    if(elem->next != NULL || elem->prev != NULL)
    {
        fprintf(stderr,"ERRO: Elemento existe em outra fila\n");
        return ERRREP;
    }

    // Se a fila estiver vazia, o prórprio elemento é o primeiro e o último
    if(*queue == NULL)
    {
        *queue = elem;
        elem->next = elem->prev = elem;
    }
    // Se a fila nao estiver vazia, o elemento é inserido no final
    else
    {
        // O ponteiro prev do elemento recebe o auxiliar, pois o elemento será o último
        elem->prev = (*queue)->prev;
        // O ponteiro next do elemento recebe o início da fila, pois o elemento será o último
        elem->next = *queue;
        // O ponteiro next do auxiliar recebe o elemento, pois o elemento será o penultimo
        (*queue)->prev->next = elem;
        // O ponteiro prev do início da fila recebe o elemento, pois o elemento será o último
        (*queue)->prev = elem;
    }
    
    return 0;


}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_remove (queue_t **queue, queue_t *elem)
{
    // Verifica se a fila existe
    if(queue == NULL)
    {
        fprintf(stderr,"ERRO: Fila nao existe\n");
        return ERRFILA;
    }

    // Verifica se a fila está vazia
    if(*queue == NULL)
    {
        fprintf(stderr,"ERRO: Fila esta vazia\n");
        return ERRVAZIA;
    }

    // Verifica se o elemento existe
    if(elem == NULL)
    {
        fprintf(stderr,"ERRO: Elemento nao existe\n");
        return ERRELEM;
    }

    // Se a fila possui somente 1 elemento
    if (((*queue)->next == *queue) && ((*queue)->prev == *queue)) {
        if (*queue == elem) // Verifica se o elemento a ser removido pertence a fila
        {
            (*queue)->prev = NULL;
            (*queue)->next = NULL;
            *queue = NULL;
            return 0;
        } 
        else 
        {
            // O elemento não pertence à fila
            fprintf(stderr, "ERRO: Elemento nao pertence a fila\n");
            return ERRPTC;
        }
    }

    // Fila possui mais de 1 elemento
    queue_t *aux = *queue;
    while(aux != elem)
    {
        aux = aux->next;
        if(aux == *queue)
        {
            fprintf(stderr,"ERRO: Elemento nao pertence a fila\n");
            return ERRPTC;
        }
    }

    // Ajusta os ponteiros do elemento anterior e do próximo
    aux->prev->next = aux->next;
    aux->next->prev = aux->prev;
    
    // Se o elemento a ser removido é a cabeça da fila
    if(aux == *queue)
        *queue = aux->next;

    // Se os ponteiros do elemento removido para NULL
    aux->next = NULL;
    aux->prev = NULL;

    return 0;
}
