#include "obstacle.h"
#include <raylib.h>

Obstacle createObstacle(Vector2 position, ObstacleType type) {
    Obstacle obstacle;
    obstacle.position = position;
    obstacle.type = type;
    obstacle.active = 1;
    
    switch (type) {
        case OBSTACLE_CRAB:
            obstacle.data.crab = createCrab(position, CRAB_FRAME_SPEED);
            break;
        case OBSTACLE_SHARK:
            obstacle.data.shark = createShark(position, (Vector2){0, 0});
            break;
        case OBSTACLE_PIGEON:
            obstacle.data.pigeon = createPigeon(position);
            break;
        case OBSTACLE_BUS:
            obstacle.data.bus.position = position;
            obstacle.data.bus.hitbox = (Rectangle){
                position.x, 
                position.y, 
                150,
                80
            };
            obstacle.data.bus.speed = 350.0f;
            break;
        default:
            // Inicialize outros tipos de obstáculos conforme necessário
            break;
    }
    
    return obstacle;
}

void updateObstacle(Obstacle* obstacle, Vector2 playerPosition, float deltaTime) {
    switch (obstacle->type) {
        case OBSTACLE_CRAB:
            updateCrab(&obstacle->data.crab);
            break;
        case OBSTACLE_SHARK:
            updateShark(&obstacle->data.shark, playerPosition, deltaTime);
            break;
        case OBSTACLE_BUS:
            obstacle->position.x -= obstacle->data.bus.speed * deltaTime;
            obstacle->data.bus.position = obstacle->position;
            obstacle->data.bus.hitbox.x = obstacle->position.x;
            obstacle->data.bus.hitbox.y = obstacle->position.y;
            break;
        case OBSTACLE_PIGEON: {
            Pigeon *pigeon = &obstacle->data.pigeon;
            int shouldPoop = updatePigeon(pigeon, deltaTime);
            (void)shouldPoop;
            break;
        }
        default:
            break;
    }
}

void drawObstacle(Obstacle obstacle, Texture2D crabTexture, Texture2D sharkTexture) {
    switch (obstacle.type) {
        case OBSTACLE_CRAB:
            drawCrab(obstacle.data.crab, crabTexture);
            break;
        case OBSTACLE_SHARK:
            drawShark(obstacle.data.shark, sharkTexture);
            break;
        case OBSTACLE_BUS: {
            Bus bus = obstacle.data.bus;
            DrawRectangleRec(bus.hitbox, YELLOW);
            DrawRectangleLinesEx(bus.hitbox, 3, BLACK);
            DrawRectangle(bus.hitbox.x + 20, bus.hitbox.y + 10, 25, 20, BLUE);
            DrawRectangle(bus.hitbox.x + 60, bus.hitbox.y + 10, 25, 20, BLUE);
            DrawRectangle(bus.hitbox.x + 100, bus.hitbox.y + 10, 25, 20, BLUE);
            break;
        }
        case OBSTACLE_PIGEON:
            drawPigeon(obstacle.data.pigeon);
            break;
        default:
            break;
    }
}
