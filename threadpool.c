#include "threadpool.h"
#include <stdio.h>

int task_free(task_t *the_task)
{
    free(the_task->arg);
    free(the_task);
    return 0;
}

int tqueue_init(tqueue_t *the_queue)
{
    the_queue->head = NULL;
    the_queue->tail = NULL;
    //pthread_mutex_init(&(the_queue->mutex), NULL);
    sem_init(&(the_queue->semaphore), 0, 1);
    //pthread_cond_init(&(the_queue->cond), NULL);
    the_queue->size = 0;
    return 0;
}

task_t *tqueue_pop(tqueue_t *the_queue)
{
    task_t *ret;
    sem_wait(&(the_queue->semaphore));
    ret = the_queue->tail;
    if (ret) {
        the_queue->tail = ret->last;
        if (the_queue->tail) {
            the_queue->tail->next = NULL;
        } else {
            the_queue->head = NULL;
        }
        the_queue->size--;
    }
    sem_post(&(the_queue->semaphore));
    return ret;
}

uint32_t tqueue_size(tqueue_t *the_queue)
{
    uint32_t ret;
    sem_wait(&(the_queue->semaphore));
    ret = the_queue->size;
    sem_post(&(the_queue->semaphore));
    return ret;
}

int tqueue_push(tqueue_t *the_queue, task_t *task)
{
    sem_wait(&(the_queue->semaphore));
    task->last = NULL;
    task->next = the_queue->head;
    if (the_queue->head)
        the_queue->head->last = task;
    the_queue->head = task;
    if (the_queue->size++ == 0)
        the_queue->tail = task;
    sem_post(&(the_queue->semaphore));
    return 0;
}

int tqueue_free(tqueue_t *the_queue)
{
    task_t *cur = the_queue->head;
    while (cur) {
        the_queue->head = the_queue->head->next;
        free(cur);
        cur = the_queue->head;
    }
    sem_destroy(&(the_queue->semaphore));
    //pthread_cond_destroy(&(the_queue->cond));
    return 0;
}

int tpool_init(tpool_t *the_pool, uint32_t tcount, void *(*func)(void *))
{
    the_pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * tcount);
    the_pool->count = tcount;
    the_pool->queue = (tqueue_t *) malloc(sizeof(tqueue_t));
    tqueue_init(the_pool->queue);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (uint32_t i = 0; i < tcount; ++i) {
        uint32_t *tmp = malloc(sizeof(uint32_t));
        *tmp = i;
        pthread_create(&(the_pool->threads[i]), &attr, func, (void *) tmp);
    }
    pthread_attr_destroy(&attr);
    return 0;
}

int tpool_free(tpool_t *the_pool)
{
    for (uint32_t i = 0; i < the_pool->count; ++i)
        pthread_join(the_pool->threads[i], NULL);
    free(the_pool->threads);
    tqueue_free(the_pool->queue);
    return 0;
}
