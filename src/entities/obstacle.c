#include "obstacle.h"

// ========== CRIAR OBSTÁCULO ==========
Obstacle createObstacle(Vector2 position, ObstacleType type) {
    Obstacle obs;
    obs.position = position;
    obs.type = type;
    obs.active = 1;

    obs.spriteLoaded = 0;
    obs.texture = LoadTexture("assets/img/hole.png");
    if (obs.texture.id != 0) {
        obs.spriteLoaded = 1;
    }

    // Hitbox
    obs.hitbox = (Rectangle){
        obs.position.x - HOLE_WIDTH / 2,
        obs.position.y - HOLE_HEIGHT / 2,
        HOLE_WIDTH,
        HOLE_HEIGHT
    };

    return obs;
}

// ========== ATUALIZAR OBSTÁCULO ==========
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

// ========== DESENHAR OBSTÁCULO ==========
void drawObstacle(Obstacle obs) {
    if (!obs.active) return;

    if (obs.spriteLoaded) {
        DrawTextureEx(obs.texture,
                     (Vector2){ obs.position.x - HOLE_WIDTH / 2, obs.position.y - HOLE_HEIGHT / 2 },
                     0, 1.0f, WHITE);
    } else {
        // Placeholder: círculo preto (buraco)
        DrawCircle(obs.position.x, obs.position.y, HOLE_WIDTH / 2, BLACK);
        DrawCircleLines(obs.position.x, obs.position.y, HOLE_WIDTH / 2, RED);
    }

    // DrawRectangleLinesEx(obs.hitbox, 1, RED);
}

// ========== DESCARREGAR RECURSOS ==========
void unloadObstacleResources(Obstacle *obs) {
    if (obs->spriteLoaded) {
        UnloadTexture(obs->texture);
        obs->spriteLoaded = 0;
    }
}
