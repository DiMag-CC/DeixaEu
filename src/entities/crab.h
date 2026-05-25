#ifndef CRAB_H
#define CRAB_H

#include "raylib.h"

#define CRAB_WIDTH  140.0f
#define CRAB_HEIGHT 85.0f
#define CRAB_NUM_FRAMES 2

typedef struct {
    Vector2 position;
    Rectangle hitbox;
    int currentFrame;
    int frameCounter;
    int frameSpeed;
} Crab;

Crab createCrab(Vector2 position, int frameSpeed);
void updateCrab(Crab* crab);

void drawCrab(Crab crab, Texture2D crabTextures[]);

#endif // CRAB_H