// stage2.h
// Fase 2 — Boa Viagem
//
// Estrutura em três modos sequenciais (transição automática entre eles):
//   1. SAND       — corrida na areia com obstáculos terrestres
//   2. TRANSITION — cutscene curta do mergulho no mar
//   3. SEA        — natação fugindo do tubarão, barra de fôlego ativa
//
// A fase termina quando o jogador supera a distância alvo do modo SEA
// e dispara a cutscene final (Parque das Esculturas).

#ifndef STAGE2_H
#define STAGE2_H

#include <raylib.h>
#include "../structure/Stage2queue.h"
#include "../entities/player.h"

// ===== Constantes da fase =====
#define STAGE2_SAND_DISTANCE     6000.0f   // distância pra terminar a Parte 1
#define STAGE2_SEA_DISTANCE      8000.0f   // distância pra terminar a Parte 2
#define STAGE2_TRANSITION_TIME   2.5f      // duração da cutscene do mergulho

// Limites do mar (modo SEA, sem gravidade)
#define STAGE2_SEA_TOP           60.0f     // superfície (recupera fôlego)
#define STAGE2_SEA_BOTTOM        400.0f    // fundo

// Tubarão
#define STAGE2_SHARK_BASE_X      -100.0f   // posição inicial (atrás do player)
#define STAGE2_SHARK_MAX_X       150.0f    // se chegar aqui, dano constante

// Fôlego
#define STAGE2_BREATH_MAX        100.0f
#define STAGE2_BREATH_DRAIN      8.0f      // por segundo embaixo d'água
#define STAGE2_BREATH_RECOVER    35.0f     // por segundo na superfície

// Power-up coco
#define STAGE2_COCO_DURATION     5.0f

// ===== Modos da fase =====
typedef enum {
    STAGE2_MODE_SAND       = 0,
    STAGE2_MODE_TRANSITION = 1,
    STAGE2_MODE_SEA        = 2,
    STAGE2_MODE_FINISHED   = 3
} Stage2Mode;

// ===== Estrutura principal da fase =====
typedef struct Stage2 {
    // --- Estado geral ---
    Stage2Mode mode;
    float modeTimer;            // tempo dentro do modo atual (pra transição)
    float scrollSpeed;
    float distanceTraveled;     // resetado a cada mudança de modo
    float spawnInterval;
    float obstacleSpawnTimer;
    float difficultyMultiplier;
    int   stage2Complete;

    // --- Cenário ---
    Texture2D bgSand;           // pode falhar load — desenha placeholder
    Texture2D bgSea;
    float backgroundScroll;     // posição do scroll do fundo

    // --- Estrutura central: fila de obstáculos ---
    Stage2Queue obstacleQueue;

    // --- Power-up coco (estado do buff no player) ---
    int   hasCoconutBuff;
    float coconutBuffTimer;

    // --- Parte 2: tubarão (perseguidor único) ---
    Vector2   sharkPosition;
    Rectangle sharkHitbox;
    int       sharkActive;

    // --- Parte 2: barra de fôlego ---
    float breath;

    // --- Parte 2: efeitos atmosféricos ---
    float lightningTimer;       // próximo relâmpago
    float lightningFlash;       // tempo restante do flash atual
    int   stormActive;          // tempestade rolando

    // --- Estado de modo SEA: empurrão de correntes ---
    float currentPushY;         // valor temporário aplicado ao player
} Stage2;

// ===== Interface (mesmo padrão da Stage1) =====
void initStage2(Stage2 *stage);
void updateStage2(Stage2 *stage, Player *player, float deltaTime);
void drawStage2(Stage2 *stage, Player *player);
void unloadStage2(Stage2 *stage);

#endif