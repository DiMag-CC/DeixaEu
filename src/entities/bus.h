#ifndef BUS_H
#define BUS_H

#include <raylib.h>

#define BUS_WIDTH 150.0f
#define BUS_HEIGHT 80.0f

typedef struct {
    Vector2 position;
    Rectangle hitbox;
    int active;
    float speed;
    float scale;
    Texture2D texture;
    int spriteLoaded;

    Rectangle topHitbox;   
    int playerOnTop;       
    float playerStandingTime;
} Bus;

Bus createBus(Vector2 position);
void updateBus(Bus *bus, float scrollSpeed, float deltaTime);
void drawBus(Bus bus);
void unloadBusResources(Bus *bus);

#endif
