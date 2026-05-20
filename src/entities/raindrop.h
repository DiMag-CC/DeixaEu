#ifndef RAINDROP_H
#define RAINDROP_H

#include <raylib.h>

#define MAX_RAINDROPS 200

// ========== ESTRUTURA DA GOTA DE CHUVA ==========
typedef struct {
    Vector2 position;
    float speed;
    int active;
} Raindrop;

// ========== SISTEMA DE CHUVA ==========
typedef struct {
    Raindrop drops[MAX_RAINDROPS];
    int count;
    float spawnTimer;
    float spawnInterval;
} RainSystem;

// ========== FUNÇÕES ==========
RainSystem createRainSystem(void);
void updateRainSystem(RainSystem *rain, float deltaTime);
void drawRainSystem(RainSystem rain);
void resetRainSystem(RainSystem *rain);

#endif
