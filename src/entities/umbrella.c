#include "umbrella.h"
#include <math.h>

// ========== CRIAR GUARDA-CHUVA ==========
Umbrella createUmbrella(Vector2 position) {
    Umbrella umbrella;
    umbrella.position = position;
    umbrella.active = 1;
    umbrella.speed = 0.0f;
    umbrella.rotationAngle = 0.0f;

    umbrella.spriteLoaded = 0;
    umbrella.texture = LoadTexture("assets/img/umbrella.png");
    if (umbrella.texture.id != 0) {
        umbrella.spriteLoaded = 1;
    }

    // Hitbox
    umbrella.hitbox = (Rectangle){
        umbrella.position.x - UMBRELLA_WIDTH / 2,
        umbrella.position.y - UMBRELLA_HEIGHT / 2,
        UMBRELLA_WIDTH,
        UMBRELLA_HEIGHT
    };

    return umbrella;
}

// ========== ATUALIZAR GUARDA-CHUVA ==========
void updateUmbrella(Umbrella *umbrella, float scrollSpeed, float deltaTime) {
    if (!umbrella->active) return;

    // Mover com scroll
    umbrella->position.x -= scrollSpeed * deltaTime;

    // Rotação contínua (animação)
    umbrella->rotationAngle += 360.0f * deltaTime;
    if (umbrella->rotationAngle >= 360.0f) {
        umbrella->rotationAngle = 0.0f;
    }

    // Movimento vertical (flutuar)
    umbrella->position.y += sinf(umbrella->rotationAngle * PI / 180.0f) * 30.0f * deltaTime;

    // Atualizar hitbox
    umbrella->hitbox.x = umbrella->position.x - UMBRELLA_WIDTH / 2;
    umbrella->hitbox.y = umbrella->position.y - UMBRELLA_HEIGHT / 2;

    // Deativar se sair da tela
    if (umbrella->position.x < -UMBRELLA_WIDTH) {
        umbrella->active = 0;
    }
}

// ========== DESENHAR GUARDA-CHUVA ==========
void drawUmbrella(Umbrella umbrella) {
    if (!umbrella.active) return;

    if (umbrella.spriteLoaded) {
        DrawTextureEx(umbrella.texture,
                     (Vector2){ umbrella.position.x - UMBRELLA_WIDTH / 2, umbrella.position.y - UMBRELLA_HEIGHT / 2 },
                     umbrella.rotationAngle, 1.0f, WHITE);
    } else {
        // Placeholder: semicírculo verde (guarda-chuva)
        DrawCircleSector(umbrella.position, UMBRELLA_WIDTH / 2, 0, 180, 10, GREEN);
        DrawCircleSectorLines(umbrella.position, UMBRELLA_WIDTH / 2, 0, 180, 10, DARKGREEN);

        // Cabo
        DrawLine(umbrella.position.x, umbrella.position.y,
                umbrella.position.x, umbrella.position.y + UMBRELLA_HEIGHT / 2, DARKGREEN);
    }

    // DrawRectangleLinesEx(umbrella.hitbox, 1, RED);
}

// ========== DESCARREGAR RECURSOS ==========
void unloadUmbrellaResources(Umbrella *umbrella) {
    if (umbrella->spriteLoaded) {
        UnloadTexture(umbrella->texture);
        umbrella->spriteLoaded = 0;
    }
}
