#ifndef BIKE_H
#define BIKE_H

#include <raylib.h>
#include "player.h"

#define BIKE_WIDTH 50.0f
#define BIKE_HEIGHT 35.0f
#define WHEEL_RADIUS 8.0f

// ========== ESTRUTURA DA BICICLETA ==========
typedef struct {
    Vector2 position;           // Posição da bike (relativa ao player)
    float wheelAngle;           // Ângulo de rotação das rodas (em graus)
    float wheelSpeed;           // Velocidade de rotação das rodas
    Texture2D bikeTexture;      // Sprite da bike
    int spriteLoaded;           // 1 = carregou, 0 = falhou
} Bike;

// ========== FUNÇÕES DA BICICLETA ==========
Bike createBike(void);
void updateBike(Bike *bike, Player *player, float deltaTime);
void drawBike(Bike bike, Player player);
void unloadBikeResources(Bike *bike);

#endif
