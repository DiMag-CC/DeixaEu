#ifndef OBSTACLE_QUEUE_H
#define OBSTACLE_QUEUE_H

#include <raylib.h>
#include "../entities/obstacle.h"
#include "../entities/bus.h"
#include "../entities/pigeon.h"

// ========== TIPOS DE OBSTÁCULO STAGE1 ==========
typedef enum {
    QUEUE_OBS_HOLE    = 0,
    QUEUE_OBS_BUS     = 1,
    QUEUE_OBS_PIGEON  = 2,
    QUEUE_OBS_UMBRELLA = 3
} QueueObstacleType;

// ========== ESTRUTURA GENÉRICA DO OBSTÁCULO ==========
typedef struct {
    QueueObstacleType type;
    Vector2 position;
    int active;

    // União de dados (um só será usado dependendo do tipo)
    union {
        Obstacle hole;
        Bus bus;
        Pigeon pigeon;
    } data;
} QueueObstacle;

// ========== NÓ E FILA ==========
typedef struct QueueNode {
    QueueObstacle obstacle;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} ObstacleQueue;

// ========== FUNÇÕES DA FILA ==========
void initObstacleQueue(ObstacleQueue *queue);
int isObstacleQueueEmpty(ObstacleQueue *queue);
int obstacleQueueSize(ObstacleQueue *queue);
void enqueueObstacle(ObstacleQueue *queue, QueueObstacle obstacle);
void dequeueObstacle(ObstacleQueue *queue);
void freeObstacleQueue(ObstacleQueue *queue);
void removeOffscreenObstacles(ObstacleQueue *queue, float minX);

#endif
