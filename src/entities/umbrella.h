#ifndef UMBRELLA_H
#define UMBRELLA_H

#include <raylib.h>

#define UMBRELLA_WIDTH 60.0f
#define UMBRELLA_HEIGHT 70.0f
#define UMBRELLA_DURATION 5.0f

// ========== ESTRUTURA DO GUARDA-CHUVA ==========
typedef struct {
    Vector2 position;
    Rectangle hitbox;
    int active;
    float speed;
    float rotationAngle;
    Texture2D texture;
    int spriteLoaded;
} Umbrella;

// ========== FUNÇÕES ==========
Umbrella createUmbrella(Vector2 position);
void updateUmbrella(Umbrella *umbrella, float scrollSpeed, float deltaTime);
void drawUmbrella(Umbrella umbrella);
void unloadUmbrellaResources(Umbrella *umbrella);

#endif
