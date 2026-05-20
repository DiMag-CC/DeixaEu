#ifndef STAGE1_H
#define STAGE1_H

#include <raylib.h>
#include "../structure/obstacleQueue.h"
#include "../entities/player.h"
#include "../entities/pigeon.h"
#include "../entities/raindrop.h"
#include "../entities/umbrella.h"

// ========== CONSTANTES ==========
#define MAX_OBSTACLES 30
#define MAX_PIGEONS 5
#define MAX_RAINDROPS 50

// ========== ESTRUTURA STAGE1 ==========
typedef struct Stage1 {
    // Background e cenário
    float scrollSpeed;
    Texture2D background;
    Texture2D buildings[5];
    float roadPosition;
    
    // Obstáculos
    ObstacleQueue obstacleQueue;
    float obstacleSpawnTimer;
    float spawnInterval;
    
    // Pombos (array separado)
    Pigeon pigeons[MAX_PIGEONS];
    int pigeonCount;
    
    // Chuva
    Raindrop raindrops[MAX_RAINDROPS];
    int raindropCount;
    float rainSpawnTimer;
    float rainSpawnInterval;
    
    // Guarda-Chuva
    Umbrella umbrellas[2];
    int umbrellasSpawned;
    
    // Progressão
    float distanceTraveled;
    int stage1Complete;
    float difficultyMultiplier;
} Stage1;

// ========== FUNÇÕES ==========

void initStage1(Stage1 *stage);
void updateStage1(Stage1 *stage, Player *player, float deltaTime);
void drawStage1(Stage1 *stage, Player *player);
void unloadStage1(Stage1 *stage);

#endif