#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct {
    Node *front, *rear;
    int size;
} Queue;

Queue* queue;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
const int MAX_SIZE = 10;
const int INIT_SIZE = 8;

Queue* initQueue(){
    Queue* q = (Queue*) malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue* q, int data){
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;

    if(q->rear == NULL){
        q->front = q->rear = newNode;
    } else{
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

int dequeue(Queue* q){
    if(q->front == NULL) return -1;

    Node* temp = q->front;
    int data = temp->data;
    q->front = q->front->next;

    if(q->front == NULL){
        q->rear = NULL;
    }

    free(temp);
    q->size--;
    return data;
}


void* producer(void* arg){
    int id = *(int*) arg;

    while(1){
        sleep(3);
        pthread_mutex_lock(&mutex);
        while(queue->size >= MAX_SIZE){
            printf("Producer %d is waiting (queue is full)...\n", id);
            pthread_cond_wait(&not_full, &mutex);
        }

        int product = rand() % 900 + 100;
        enqueue(queue, product);
        printf("Producer %d produced: %d (queue sized: %d)\n:", id, product, queue->size);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* consumer(void* arg){
    int id = *(int*) arg;
    sleep(5);

    while(1){
        sleep(1);
        pthread_mutex_lock(&mutex);

        while(queue->size <= 0){
            printf("Consumer %d is waiting (queue is empty)...\n", id);
            pthread_cond_wait(&not_empty, &mutex);
        }

        int product = dequeue(queue);
        printf("Consumer %d consumed: %d (queue size: %d)\n", id, product, queue->size);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(){
    srand(time(NULL));

    queue = initQueue();
    // 初试生成8个商品
    for(int i = 0; i < INIT_SIZE; ++i){
        int product = rand() % 900 + 100;
        enqueue(queue, product);
        printf("Initial product added: %d\n", product);
    }

    // 创建3个生产者线程
    pthread_t producers[3];
    int producer_ids[3] = {1, 2, 3};
    for(int i = 0; i < 3; ++i){
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }

    pthread_t consumers[2];
    int consumer_ids[2] = {1, 2};
    for(int i = 0; i < 2; ++i){
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }

    for(int i = 0; i < 3; ++i){
        pthread_join(producers[i], NULL);
    }

    for(int i = 0; i < 2; ++i){
        pthread_join(consumers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    free(queue);
    
    return 0;
}