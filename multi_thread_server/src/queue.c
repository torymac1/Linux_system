
#include <semaphore.h>
#include <errno.h>
#include "queue.h"

// item_destructor_f destroy_function(queue_t *self){
//     queue_node_t *tmp = self->front;
//     self->front = self->front->next;
//     free(self->front);
//     return NULL;
// }




queue_t *create_queue(void) {
    queue_t *sp;
    if ((sp = calloc(1, sizeof(queue_t))) == NULL)
        return NULL;
    sp->front = NULL;
    sp->rear = NULL;
    if (sem_init(&sp->items, 0, 0) < 0)
        return NULL;
    if(pthread_mutex_init(&sp->lock, NULL)!=0)
        return NULL;
    sp->invalid = false;
    return sp;

}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    if(self == NULL || self->invalid){
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->lock);
    while(self->front != NULL){
        destroy_function(self->front->item);
        queue_node_t *tmp = self->front;
        self->front = self->front->next;
        free(tmp);
    }
    self->front = NULL;
    self->rear = NULL;
    self->invalid = true;
    pthread_mutex_unlock(&self->lock);
    return true;
}

bool enqueue(queue_t *self, void *item) {
    if(self == NULL || self->invalid || item == NULL){
        errno = EINVAL;
        return false;
    }
    queue_node_t *tmp;
    if ((tmp = calloc(1, sizeof(queue_node_t))) == NULL)
        return false;
    tmp->item = item;
    tmp->next = NULL;
    //lock and insert to rear
    pthread_mutex_lock(&self->lock);
    if (self->front == NULL){
        self->front = tmp;
        self->rear = tmp;
    }
    else{
        self->rear->next = tmp;
        self->rear = tmp;
    }
    //unlock
    sem_post(&self->items);
    pthread_mutex_unlock(&self->lock);
    return true;
}

void *dequeue(queue_t *self) {
    if(self == NULL || self->invalid == true){
        errno = EINVAL;
        return NULL;
    }
    sem_wait(&self->items);
    pthread_mutex_lock(&self->lock);
    void *rtn = self->front->item;
    queue_node_t *tmp = self->front;
    self->front = self->front->next;
    free(tmp);
    pthread_mutex_unlock(&self->lock);
    return rtn;
}
