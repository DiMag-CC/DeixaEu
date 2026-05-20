#ifndef BUS_H
#define BUS_H

#include <raylib.h>

#define BUS_WIDTH 80.0f
#define BUS_HEIGHT 40.0f
#define GROUND_LEVEL 350.0f

// ========== ESTRUTURA DO ÔNIBUS ==========
typedef struct {
    Vector2 position;
    Rectangle hitbox;
    int active;
    float speed;
    float scale;
    Texture2D texture;
    int spriteLoaded;
} Bus;

// ========== FUNÇÕES ==========
Bus createBus(Vector2 position);
void updateBus(Bus *bus, float scrollSpeed, float deltaTime);
void drawBus(Bus bus);
void unloadBusResources(Bus *bus);

#endif
