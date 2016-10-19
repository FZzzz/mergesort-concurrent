#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "threadpool.h"
#include "list.h"

#define USAGE "usage: ./sort [thread_count] [input_file]\n"
#define CLOCK_ID CLOCK_MONOTONIC_RAW
#define ONE_SEC 1000000000.0

struct {
    sem_t semaphore;
    int cut_thread_count;
} data_context;

static llist_t *tmp_list;
static llist_t *the_list = NULL;

static int thread_count = 0, max_cut = 0 , data_count = 0;
static const char* input_file;
static tpool_t *pool = NULL;

static int task_count[1024] = {0};

llist_t *merge_list(llist_t *a, llist_t *b)
{
    llist_t *_list = list_new();
    node_t *current = NULL;
    while (a->size && b->size) {
        llist_t* small = (strcmp(a->head->data , b->head->data) <= 0) ? a:b;

        if (current) {
            current->next = small->head;
            current = current->next;
        } else {
            _list->head = small->head;
            current = _list->head;
        }
        small->head = small->head->next;
        --small->size;
        ++_list->size;
        current->next = NULL;
    }

    llist_t *remaining = (llist_t *) ((intptr_t) a * (a->size > 0) +
                                      (intptr_t) b * (b->size > 0));
    if (current) current->next = remaining->head;
    _list->size += remaining->size;
    free(a);
    free(b);
    return _list;
}

llist_t *merge_sort(llist_t *list)
{
    if (list->size < 2)
        return list;
    int mid = list->size / 2;
    llist_t *left = list;
    llist_t *right = list_new();
    right->head = list_nth(list, mid);
    right->size = list->size - mid;
    list_nth(list, mid - 1)->next = NULL;
    left->size = mid;
    return merge_list(merge_sort(left), merge_sort(right));
}

void merge(void *data)
{
    llist_t *_list = (llist_t *) data;
    if (_list->size < (uint32_t) data_count) {
        sem_wait(&(data_context.semaphore));
        llist_t *_t = tmp_list;
        if (!_t) {
            tmp_list = _list;
            sem_post(&(data_context.semaphore));
        } else {
            tmp_list = NULL;
            sem_post(&(data_context.semaphore));
            task_t *_task = (task_t *) malloc(sizeof(task_t));
            _task->func = merge;
            _task->arg = merge_list(_list, _t);
            tqueue_push(pool->queue, _task);
        }
    } else {
        the_list = _list;
        task_t *_task = (task_t *) malloc(sizeof(task_t));
        _task->func = NULL;
        tqueue_push(pool->queue, _task);
        //list_print(_list);
    }
}

void cut_func(void *data)
{
    llist_t *list = (llist_t *) data;
    sem_wait(&(data_context.semaphore));
    int cut_count = data_context.cut_thread_count;

    if (list->size > 1 && cut_count < max_cut) {
        ++data_context.cut_thread_count;
        sem_post(&(data_context.semaphore));

        /* cut list */
        int mid = list->size / 2;
        llist_t *_list = list_new();
        _list->head = list_nth(list, mid);
        _list->size = list->size - mid;
        list_nth(list, mid - 1)->next = NULL;//cut
        list->size = mid;

        /* create new task: left */
        task_t *_task = (task_t *) malloc(sizeof(task_t));
        _task->func = cut_func;
        _task->arg = _list;
        tqueue_push(pool->queue, _task);

        /* create new task: right */
        _task = (task_t *) malloc(sizeof(task_t));
        _task->func = cut_func;
        _task->arg = list;
        tqueue_push(pool->queue, _task);
    } else {
        sem_post(&(data_context.semaphore));
        merge(merge_sort(list));
    }
}

static void *task_run(void *data)
{
    uint32_t *tid = (uint32_t *)data;
    //printf("%lu \n" , pthread_self());
    while (1) {
        task_t *_task = tqueue_pop(pool->queue);
        if (_task) {
            if (!_task->func) {
                tqueue_push(pool->queue, _task);//set queue empty and leave execution
                break;
            } else {
                _task->func(_task->arg);//call function and run
                task_count[ (*tid) ]++;
                free(_task);
            }
        }
    }
    free(data);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    if (argc < 3) {
        printf(USAGE);
        return -1;
    }

    thread_count = atoi(argv[1]);
    //data_count = argv[2];  --> fix this
    input_file = argv[2];
    /*
        max_cut = thread_count * (thread_count <= data_count) +
                  data_count * (thread_count > data_count) - 1;
    */
    /* Read data */
    the_list = list_new();

    char line[16];
    FILE* input_fptr = fopen(input_file , "r");

    assert(input_fptr && "NULL FILE PTR\n");
#if defined(__GNUC__)
    __builtin___clear_cache((void *) the_list , (void *) the_list + sizeof(llist_t));
#endif

    while (fgets(line, sizeof(line), input_fptr)) {
        int i=0;
        while (line[i] != '\0' && i < 16)
            i++;
        line[i - 1] = '\0';
        list_add(the_list , line);
        data_count++;
    }

    max_cut = thread_count * (thread_count <= data_count) +
              data_count * (thread_count > data_count) - 1;

#if defined(__GNUC__)
    __builtin___clear_cache((void *) the_list , (void *) the_list + sizeof(llist_t));
#endif
    fclose(input_fptr);
    printf("start merge sort\n");

    FILE* exec_fptr = fopen("exec_time.csv" , "a+");
    struct timespec start = {0, 0};
    struct timespec end = {0, 0};

    clock_gettime(CLOCK_ID , &start);

    /* initialize tasks inside thread pool */
    sem_init(&(data_context.semaphore),0,1);
    data_context.cut_thread_count = 0;
    tmp_list = NULL;
    pool = (tpool_t *) malloc(sizeof(tpool_t));
    tpool_init(pool, thread_count, task_run);

    /* launch the first task */
    task_t *_task = (task_t *) malloc(sizeof(task_t));
    _task->func = cut_func;
    _task->arg = the_list;
    tqueue_push(pool->queue, _task);

    /* release thread pool and join threads*/
    tpool_free(pool);

    clock_gettime(CLOCK_ID , &end);

    const double exec_time = (double) (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/ONE_SEC;
    fprintf(exec_fptr , "%d %.10lf\n" , thread_count , exec_time);
    fprintf(stdout , "%.10lf ms\n" , exec_time);

    fclose(exec_fptr);


    FILE* result_fptr = fopen("result.txt" , "w+");
    assert(result_fptr && "null result file pointer");
    list_print(the_list , result_fptr);
    fclose(result_fptr);

    FILE* scalability_fptr = fopen("scalability.txt" , "w+");
    assert(scalability_fptr && "null scala txt");
    for(int i=0; i<thread_count; i++) {
        fprintf(scalability_fptr , "%d %d\n",i , task_count[i]);
    }
    fclose(scalability_fptr);


    return 0;
}
