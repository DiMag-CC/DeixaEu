#ifndef SHARK_H
#define SHARK_H

#include <raylib.h>

#define SHARK_WIDTH 120
#define SHARK_HEIGHT 60
#define SHARK_SPEED 180.0f

typedef struct {
    Vector2 position;
    Rectangle hitbox;
    Vector2 velocity;
} Shark;

Shark createShark(Vector2 position, Vector2 velocity);
void updateShark(Shark *shark, Vector2 playerPosition, float deltaTime);
void drawShark(Shark shark, Texture2D sharkTexture);

#endif
