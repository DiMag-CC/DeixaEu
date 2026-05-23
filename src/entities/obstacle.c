#include "obstacle.h"
#include "../utils/gameConstants.h"

Obstacle createObstacle(Vector2 position, ObstacleType type) {
    Obstacle obs;
    obs.position = position;
    obs.type = type;
    obs.active = 1;
    obs.scale = 0.18f;

    obs.spriteLoaded = 0;
    obs.texture = LoadTexture("assets/img/hole.png");
    if (obs.texture.id != 0) {
        obs.spriteLoaded = 1;
    }

    obs.hitbox = (Rectangle){
        obs.position.x - HOLE_WIDTH / 2,
        obs.position.y - HOLE_HEIGHT / 2,
        HOLE_WIDTH,
        HOLE_HEIGHT
    };

    return obs;
}

void updateObstacle(Obstacle *obs, float scrollSpeed, float deltaTime) {
    if (!obs->active) return;

    // Mover com scroll
    obs->position.x -= scrollSpeed * deltaTime;

    // Atualizar hitbox
    obs->hitbox.x = obs->position.x - HOLE_WIDTH / 2;
    obs->hitbox.y = obs->position.y - HOLE_HEIGHT / 2;

    // Deativar se sair da tela
    if (obs->position.x < -HOLE_WIDTH) {
        obs->active = 0;
    }
}

void drawObstacle(Obstacle obs) {
    if (!obs.active) return;

    const float HOLE_TARGET_WIDTH = 70.0f;
    float scaleX = HOLE_TARGET_WIDTH / obs.texture.width;
    float scaleY = scaleX;  // Manter aspect ratio

    float scaledWidth = HOLE_TARGET_WIDTH;
    float scaledHeight = obs.texture.height * scaleY;

    if (obs.spriteLoaded && obs.texture.id != 0) {
        float holeGroundY = GLOBAL_GROUND_LEVEL - scaledHeight / 2 + 32.0f; 

        Rectangle source = { 0, 0, (float)obs.texture.width, (float)obs.texture.height };
        Rectangle dest = {
            obs.position.x - scaledWidth / 2,
            holeGroundY,
            scaledWidth,
            scaledHeight
        };
        DrawTexturePro(obs.texture, source, dest, (Vector2){0, 0}, 0, WHITE);
    } else {
        float holeGroundY = 380.0f - 15.0f;  // Altura aprox
        float radius = 30.0f;
        DrawCircle((int)obs.position.x, (int)holeGroundY, radius, BLACK);
        DrawCircleLines((int)obs.position.x, (int)holeGroundY, radius - 2.0f, RED);
    }
}

void unloadObstacleResources(Obstacle *obs) {
    if (obs->spriteLoaded) {
        UnloadTexture(obs->texture);
        obs->spriteLoaded = 0;
    }
}
