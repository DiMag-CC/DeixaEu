#ifndef PIGEON_H
#define PIGEON_H

#include <raylib.h>
#include "../gfx/animation.h"

#define SCREEN_HEIGHT 1080.0f
#define PIGEON_WIDTH 25.0f
#define PIGEON_HEIGHT 20.0f
#define POOP_WIDTH 8.0f
#define POOP_HEIGHT 8.0f
#define MAX_POOPS 50

// ========== ESTRUTURA DE FEZES ==========
typedef struct {
    Vector2 position;
    Rectangle hitbox;
    int active;
    float velocityY;      // Velocidade vertical (gravidade)
    float rotationZ;      // Rotação ao cair
} Poop;

// ========== ESTRUTURA DO POMBO ==========
typedef struct {
    Vector2 position;
    Rectangle hitbox;
    int active;
    float speed;
    float poopTimer;
    float poopInterval;
    float wavePhase;
    float scale;
    Texture2D texture;
    int spriteLoaded;

    // Animação direcional (pigeon1L/R + pigeon2L/R)
    DirectionalAnimationSet animation;
    char direction; // 'L' ou 'R'

    // Fezes que o pombo solta
    Poop poops[MAX_POOPS];
    int poopCount;
} Pigeon;

// ========== FUNÇÕES ==========
Pigeon createPigeon(Vector2 position);
void updatePigeon(Pigeon *pigeon, float scrollSpeed, float deltaTime);
void drawPigeon(Pigeon pigeon);
void unloadPigeonResources(Pigeon *pigeon);

#endif
