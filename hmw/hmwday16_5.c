#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mtx;
int is_a_done = 0;

void* taskA(void* arg){
    pthread_mutex_lock(&mtx);
    printf("Task A started\n");
    sleep(3);
    printf("Task A completed\n");
    is_a_done = 1;
    pthread_mutex_unlock(&mtx);

    return NULL;
}

void* taskB(void* arg){
    while(1){
        pthread_mutex_lock(&mtx);
        if(is_a_done){
            printf("Task B is now running\n");
            pthread_mutex_unlock(&mtx);
            break;
        }
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main(){
    pthread_t thread1, thread2;
    pthread_mutex_init(&mtx, NULL);
    pthread_create(&thread1, NULL, taskA, NULL);
    pthread_create(&thread2, NULL, taskB, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mtx);
    return 0;
}