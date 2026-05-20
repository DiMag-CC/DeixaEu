#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <raylib.h>

#define HOLE_WIDTH 40.0f
#define HOLE_HEIGHT 30.0f
#define GROUND_LEVEL 350.0f

// ========== TIPOS DE OBSTÁCULO ==========
typedef enum {
    OBS_HOLE = 0
} ObstacleType;

// ========== ESTRUTURA DO OBSTÁCULO ==========
typedef struct {
    Vector2 position;
    Rectangle hitbox;
    ObstacleType type;
    int active;
    Texture2D texture;
    int spriteLoaded;
} Obstacle;

// ========== FUNÇÕES ==========
Obstacle createObstacle(Vector2 position, ObstacleType type);
void updateObstacle(Obstacle *obs, float scrollSpeed, float deltaTime);
void drawObstacle(Obstacle obs);
void unloadObstacleResources(Obstacle *obs);

#endif
