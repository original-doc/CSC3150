
#include <stdlib.h>
#include <pthread.h>
#include "async.h"
#include "utlist.h"


my_queue_t *jobthread_queue;
pthread_mutex_t mutex_queue;
pthread_cond_t cond_queue;

int count_thread = 0;


void* thread_func(void* args);
void async_init(int num_threads);
void async_run(void (*hanlder)(int), int args);



void async_init(int num_threads) {
    pthread_t threads[num_threads];

    my_queue_t template_queue = {
        .head = NULL,
        .size = 0
    };
    jobthread_queue = (my_queue_t*)malloc(sizeof(template_queue));
    //jobthread_queue = (my_queue_t*)malloc(sizeof(my_queue_t));
    jobthread_queue->head = NULL;
    jobthread_queue->size = 0;

    pthread_mutex_init(&mutex_queue, NULL);
    pthread_cond_init(&cond_queue, NULL);


    for(int i = 0; i < num_threads ; i++){
        int rc = pthread_create(&threads[i], NULL, &thread_func, NULL);
        printf("thread %d is created.", i);
        if(rc){
            printf("ERROR: Failed to create pthread %d\n", i);
        }
    }
    return;
    // TODO: create num_threads threads and initialize the thread pool 
}


void async_run(void (*hanlder)(int), int args) {
    //hanlder(args);
    my_item_t template_node_thread = {
        .args = args,
        .next = NULL,
        .prev = NULL,
        .handler_func = hanlder
    };
    my_item_t* thread_node;
    thread_node = (my_item_t*)malloc(sizeof(template_node_thread));
    //thread_node = (my_item_t*)malloc(sizeof(my_item_t));
    thread_node->args = args;
    thread_node->next = NULL;
    thread_node->prev = NULL;
    thread_node->handler_func = hanlder;
    printf("item args = %d\n", thread_node->args);
    printf("item handler_func = %d\n", (thread_node->handler_func));
    pthread_mutex_lock(&mutex_queue);
    printf("enqueue    queue.size = %d \n", jobthread_queue->size);
    DL_APPEND(jobthread_queue->head, thread_node);
    jobthread_queue->size ++;
    pthread_mutex_unlock(&mutex_queue);

    pthread_cond_signal(&cond_queue);
     // TODO: rewrite it to support thread pool 
} 


void* thread_func(void* args){
    count_thread ++;
    int local_num = count_thread;

    printf("thread %d starts", count_thread);
    while(1){
        my_item_t* task;

        pthread_mutex_lock(&mutex_queue);
        while(jobthread_queue->size == 0){
            pthread_cond_wait(&cond_queue, &mutex_queue);
            printf("thread %d condition wait\n", local_num);
        }
        
        task = jobthread_queue->head;
        DL_DELETE(jobthread_queue->head, jobthread_queue->head);
        jobthread_queue->size --;
        pthread_mutex_unlock(&mutex_queue);

        printf("task.args = %d\n", task->args);
        printf("task.handler_func = %d\n", (task->handler_func));
        task->handler_func(task->args);
        printf("done...\n");
    }

}



