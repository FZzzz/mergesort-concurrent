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
    the_queue->hold = 0;
    pthread_mutex_init(&(the_queue->mutex), NULL);
    pthread_cond_init(&(the_queue->cond), NULL);
    the_queue->size = 0;
    return 0;
}

task_t *tqueue_pop(tqueue_t *the_queue)
{
    task_t *ret;
    pthread_mutex_lock(&(the_queue->mutex));
    while (the_queue->hold) {
        pthread_cond_wait(&(the_queue->cond), &(the_queue->mutex));
    }
    the_queue->hold = 1;
    pthread_mutex_unlock(&(the_queue->mutex));

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

    pthread_mutex_lock(&(the_queue->mutex));
    the_queue->hold = 0;
    pthread_cond_signal(&(the_queue->cond));
    pthread_mutex_unlock(&(the_queue->mutex));
    return ret;
}

uint32_t tqueue_size(tqueue_t *the_queue)
{
    uint32_t ret;
    pthread_mutex_lock(&(the_queue->mutex));
    while (the_queue->hold) {
        pthread_cond_wait(&(the_queue->cond), &(the_queue->mutex));
    }
    the_queue->hold = 1;
    pthread_mutex_unlock(&(the_queue->mutex));

    ret = the_queue->size;

    pthread_mutex_lock(&(the_queue->mutex));
    the_queue->hold = 0;
    pthread_cond_signal(&(the_queue->cond));
    pthread_mutex_unlock(&(the_queue->mutex));
    return ret;
}

int tqueue_push(tqueue_t *the_queue, task_t *task)
{
    pthread_mutex_lock(&(the_queue->mutex));
    while (the_queue->hold) {
        pthread_cond_wait(&(the_queue->cond), &(the_queue->mutex));
    }
    the_queue->hold = 1;
    pthread_mutex_unlock(&(the_queue->mutex));

    task->last = NULL;
    task->next = the_queue->head;
    if (the_queue->head)
        the_queue->head->last = task;
    the_queue->head = task;
    if (the_queue->size++ == 0)
        the_queue->tail = task;

    pthread_mutex_lock(&(the_queue->mutex));
    the_queue->hold = 0;
    pthread_cond_signal(&(the_queue->cond));
    pthread_mutex_unlock(&(the_queue->mutex));
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
    pthread_mutex_destroy(&(the_queue->mutex));
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
