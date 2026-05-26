// stage2Queue.c
// Implementação da fila dinâmica de obstáculos da Fase 2.

#include <stdlib.h>
#include "Stage2queue.h"

// ===== Operações básicas da fila =====

void initStage2Queue(Stage2Queue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

int isStage2QueueEmpty(Stage2Queue *queue) {
    return (queue->front == NULL);
}

int stage2QueueSize(Stage2Queue *queue) {
    return queue->size;
}

void enqueueStage2(Stage2Queue *queue, Stage2Obstacle obstacle) {
    Stage2Node *newNode = (Stage2Node*)malloc(sizeof(Stage2Node));
    if (newNode == NULL) return;

    newNode->obstacle = obstacle;
    newNode->next = NULL;

    if (isStage2QueueEmpty(queue)) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

void dequeueStage2(Stage2Queue *queue) {
    if (isStage2QueueEmpty(queue)) return;

    Stage2Node *tmp = queue->front;
    queue->front = queue->front->next;
    free(tmp);
    queue->size--;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }
}

void freeStage2Queue(Stage2Queue *queue) {
    while (!isStage2QueueEmpty(queue)) {
        dequeueStage2(queue);
    }
}

// ===== Operações específicas que fazem a fila ser CENTRAL da gameplay =====

// Conta quantos obstáculos de um tipo específico estão na fila.
// Usado pra regras de spawn (ex: "não spawnar onda gigante se já tem uma").
int countStage2ObstaclesByType(Stage2Queue *queue, Stage2ObstacleType type) {
    int count = 0;
    Stage2Node *cur = queue->front;
    while (cur != NULL) {
        if (cur->obstacle.type == type) count++;
        cur = cur->next;
    }
    return count;
}

// Remove em lote todos os obstáculos que saíram da tela.
// Substitui o "while (!empty && front.x < -150) dequeue" inline do stage1.
void removeOffscreenStage2(Stage2Queue *queue, float minX) {
    while (!isStage2QueueEmpty(queue) &&
           queue->front->obstacle.position.x < minX) {
        dequeueStage2(queue);
    }
}

// ===== Helper de criação =====
// Tamanhos e posições calibrados pra placeholders.
// Quando arte chegar, só ajustar dimensões aqui.
Stage2Obstacle createStage2Obstacle(Vector2 position, Stage2ObstacleType type) {
    Stage2Obstacle obs;
    obs.type = type;
    obs.position = position;
    obs.active = 1;
    obs.animTimer = 0.0f;
    obs.animFrame = 0;
    obs.verticalPhase = 0.0f;

    // Dimensões da hitbox por tipo
    float w = 40, h = 40;
    switch (type) {
        case S2_OBS_CRAB:           w = 40;  h = 40;  break;
        case S2_OBS_UMBRELLA_BEACH: w = 50;  h = 80;  break;
        case S2_OBS_VENDOR:         w = 45;  h = 70;  break;
        case S2_OBS_SANDCASTLE:     w = 50;  h = 40;  break;
        case S2_OBS_BEACH_WAVE:     w = 80;  h = 30;  break;
        case S2_OBS_KITE:           w = 35;  h = 35;  break;
        case S2_OBS_COCONUT:        w = 30;  h = 30;  break;
        case S2_OBS_JELLYFISH:      w = 35;  h = 45;  break;
        case S2_OBS_TRASH:          w = 30;  h = 25;  break;
        case S2_OBS_FISHNET:        w = 60;  h = 200; break;
        case S2_OBS_BIG_WAVE:       w = 200; h = 250; break;
        case S2_OBS_CURRENT:        w = 100; h = 80;  break;
    }
    obs.hitbox = (Rectangle){ position.x, position.y, w, h };
    return obs;
}