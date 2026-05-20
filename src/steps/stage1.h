#ifndef STAGE1_H
#define STAGE1_H

#include <raylib.h>
#include "../entities/player.h"
#include "../entities/bike.h"
#include "../entities/raindrop.h"
#include "../structure/obstacleQueue.h"

// ========== CONSTANTES DA FASE ==========
#define STAGE1_TARGET_DISTANCE 8000.0f
#define STAGE1_BASE_SCROLL_SPEED 200.0f
#define STAGE1_MAX_SCROLL_SPEED 400.0f
#define STAGE1_DIFFICULTY_RAMP 0.001f

// ========== ESTRUTURA PRINCIPAL DA FASE 1 ==========
typedef struct {
    // ===== Estado geral =====
    float scrollSpeed;              // Velocidade de scroll
    float distanceTraveled;         // Distância percorrida nesta fase
    float spawnInterval;            // Intervalo de spawn
    float obstacleSpawnTimer;       // Timer do spawn

    // ===== Entidades =====
    Bike bike;                      // Bicicleta do jogador
    RainSystem rain;                // Sistema de chuva
    ObstacleQueue obstacleQueue;    // Fila de obstáculos

    // ===== Background/Cenário =====
    Texture2D backgroundTexture;    // Textura de fundo
    int bgLoaded;                   // 1 = carregou, 0 = falhou
    Camera2D camera;                // Câmera side-scrolling

    // ===== Dificuldade =====
    float difficultyMultiplier;     // Multiplicador de dificuldade (1.0 -> 2.0)
    float elapsedTime;              // Tempo decorrido

    // ===== Status =====
    int stage1Complete;             // 1 = fase completa, 0 = rodando
    int stage1Failed;               // 1 = jogador morreu, 0 = vivo

} Stage1;

// ========== FUNÇÕES DA FASE ==========
void initStage1(Stage1 *stage);
void updateStage1(Stage1 *stage, Player *player, float deltaTime);
void drawStage1(Stage1 *stage, Player *player);
void unloadStage1(Stage1 *stage);

#endif
