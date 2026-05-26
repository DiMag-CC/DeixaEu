// stage2Queue.h
// Fila dinâmica de obstáculos da Fase 2 — Boa Viagem
//
// Estrutura de dados CENTRAL da Fase 2 (requisito do projeto).
// FIFO (First In First Out): obstáculos entram pela direita da tela
// e saem pela esquerda na mesma ordem em que apareceram.
//
// Usada tanto na Parte 1 (areia) quanto na Parte 2 (mar).

#ifndef STAGE2_QUEUE_H
#define STAGE2_QUEUE_H

#include <raylib.h>

// ===== Tipos de obstáculo da Fase 2 =====
// Mantidos isolados da Fase 1 pra não conflitar com obstacle.h compartilhado.
typedef enum {
    // Parte 1 — Areia
    S2_OBS_CRAB           = 0,  // Caranguejo (pula)
    S2_OBS_UMBRELLA_BEACH = 1,  // Guarda-sol (desvia lateralmente)
    S2_OBS_VENDOR         = 2,  // Ambulante (desvia)
    S2_OBS_SANDCASTLE     = 3,  // Castelo de areia (pula)
    S2_OBS_BEACH_WAVE     = 4,  // Onda lambendo a areia (faixa de cima)
    S2_OBS_KITE           = 5,  // Pipa caindo (agacha)
    S2_OBS_COCONUT        = 6,  // Power-up água de coco

    // Parte 2 — Mar
    S2_OBS_JELLYFISH      = 10, // Água-viva (dano + paralisia)
    S2_OBS_TRASH          = 11, // Lixo boiando (atrasa)
    S2_OBS_FISHNET        = 12, // Rede de pesca (prende)
    S2_OBS_BIG_WAVE       = 13, // Onda gigante (mergulhar pra evitar)
    S2_OBS_CURRENT        = 14  // Corrente marítima (empurra)
} Stage2ObstacleType;

// ===== Estrutura unificada do obstáculo da Fase 2 =====
typedef struct {
    Stage2ObstacleType type;
    Vector2 position;
    Rectangle hitbox;
    int active;             // 0 = ignorar (já colidiu/saiu)
    float animTimer;        // pra animações (onda quebrando, pipa girando)
    int animFrame;
    float verticalPhase;    // pra movimento senoidal (água-viva, lixo)
} Stage2Obstacle;

// ===== Nó e fila =====
typedef struct Stage2Node {
    Stage2Obstacle obstacle;
    struct Stage2Node *next;
} Stage2Node;

typedef struct {
    Stage2Node *front;
    Stage2Node *rear;
    int size;               // contador pra evitar percorrer toda a fila
} Stage2Queue;

// ===== Funções da estrutura de dados (≥6 — requisito do projeto) =====
void initStage2Queue(Stage2Queue *queue);
int  isStage2QueueEmpty(Stage2Queue *queue);
int  stage2QueueSize(Stage2Queue *queue);
void enqueueStage2(Stage2Queue *queue, Stage2Obstacle obstacle);
void dequeueStage2(Stage2Queue *queue);
void freeStage2Queue(Stage2Queue *queue);
int  countStage2ObstaclesByType(Stage2Queue *queue, Stage2ObstacleType type);
void removeOffscreenStage2(Stage2Queue *queue, float minX);

// ===== Helper pra criar obstáculo já configurado =====
Stage2Obstacle createStage2Obstacle(Vector2 position, Stage2ObstacleType type);

#endif