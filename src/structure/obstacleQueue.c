#include "obstacleQueue.h"
#include <stdlib.h>

static void releaseQueueObstacleResources(QueueObstacle *obstacle) {
    if (obstacle == NULL) return;

    switch (obstacle->type) {
        case QUEUE_OBS_HOLE:
            unloadObstacleResources(&obstacle->data.hole);
            break;
        case QUEUE_OBS_BUS:
            unloadBusResources(&obstacle->data.bus);
            break;
        case QUEUE_OBS_PIGEON:
            unloadPigeonResources(&obstacle->data.pigeon);
            break;
        case QUEUE_OBS_UMBRELLA:
            unloadUmbrellaResources(&obstacle->data.umbrella);
            break;
    }
}

// ========== INICIALIZAR FILA ==========
void initObstacleQueue(ObstacleQueue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

// ========== VERIFICAR SE FILA VAZIA ==========
int isObstacleQueueEmpty(ObstacleQueue *queue) {
    return (queue->front == NULL);
}

// ========== TAMANHO DA FILA ==========
int obstacleQueueSize(ObstacleQueue *queue) {
    return queue->size;
}

// ========== ENFILEIRAR (ADICIONAR) ==========
void enqueueObstacle(ObstacleQueue *queue, QueueObstacle obstacle) {
    QueueNode *newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (newNode == NULL) return;

    newNode->obstacle = obstacle;
    newNode->next = NULL;

    if (isObstacleQueueEmpty(queue)) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

// ========== DESENFILEIRAR (REMOVER DO INÍCIO) ==========
void dequeueObstacle(ObstacleQueue *queue) {
    if (isObstacleQueueEmpty(queue)) return;

    QueueNode *tmp = queue->front;
    queue->front = queue->front->next;
    releaseQueueObstacleResources(&tmp->obstacle);
    free(tmp);
    queue->size--;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }
}

// ========== LIBERAR TODA A FILA ==========
void freeObstacleQueue(ObstacleQueue *queue) {
    while (!isObstacleQueueEmpty(queue)) {
        dequeueObstacle(queue);
    }
}

// ========== REMOVER OBSTÁCULOS FORA DA TELA ==========
void removeOffscreenObstacles(ObstacleQueue *queue, float minX) {
    while (!isObstacleQueueEmpty(queue)) {
        QueueNode *front = queue->front;

        if (!front->obstacle.active || front->obstacle.position.x < minX) {
            dequeueObstacle(queue);
        } else {
            break;
        }
    }
}
