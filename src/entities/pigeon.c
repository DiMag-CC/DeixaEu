#include "pigeon.h"
#include <math.h>
#include <stdlib.h>

#define PIGEON_SPEED 150.0f
#define POOP_SPAWN_INTERVAL 0.8f
#define POOP_SPEED 200.0f

// ========== CRIAR POMBO ==========
Pigeon createPigeon(Vector2 position) {
    Pigeon pigeon;
    pigeon.position = position;
    pigeon.active = 1;
    pigeon.speed = PIGEON_SPEED;
    pigeon.poopTimer = 0.0f;
    pigeon.poopInterval = POOP_SPAWN_INTERVAL;
    pigeon.wavePhase = 0.0f;
    pigeon.poopCount = 0;

    pigeon.spriteLoaded = 0;
    pigeon.texture = LoadTexture("assets/img/pigeon.png");
    if (pigeon.texture.id != 0) {
        pigeon.spriteLoaded = 1;
    }

    // Hitbox
    pigeon.hitbox = (Rectangle){
        pigeon.position.x - PIGEON_WIDTH / 2,
        pigeon.position.y - PIGEON_HEIGHT / 2,
        PIGEON_WIDTH,
        PIGEON_HEIGHT
    };

    // Inicializar fezes
    for (int i = 0; i < MAX_POOPS; i++) {
        pigeon.poops[i].active = 0;
        pigeon.poops[i].position = (Vector2){ 0, 0 };
        pigeon.poops[i].speed = POOP_SPEED;
    }

    return pigeon;
}

// ========== ATUALIZAR POMBO ==========
void updatePigeon(Pigeon *pigeon, float scrollSpeed, float deltaTime) {
    if (!pigeon->active) return;

    // Movimento horizontal com scroll
    pigeon->position.x -= scrollSpeed * deltaTime;

    // Movimento vertical senoidal (voar)
    pigeon->wavePhase += deltaTime * 3.0f;
    pigeon->position.y += sinf(pigeon->wavePhase) * 20.0f * deltaTime;

    // Atualizar hitbox
    pigeon->hitbox.x = pigeon->position.x - PIGEON_WIDTH / 2;
    pigeon->hitbox.y = pigeon->position.y - PIGEON_HEIGHT / 2;

    // Spawnar fezes
    pigeon->poopTimer += deltaTime;
    if (pigeon->poopTimer >= pigeon->poopInterval) {
        pigeon->poopTimer = 0.0f;

        // Procurar um espaço vazio
        for (int i = 0; i < MAX_POOPS; i++) {
            if (!pigeon->poops[i].active) {
                pigeon->poops[i].position = (Vector2){
                    pigeon->position.x,
                    pigeon->position.y + PIGEON_HEIGHT / 2
                };
                pigeon->poops[i].active = 1;
                pigeon->poopCount++;
                if (pigeon->poopCount > MAX_POOPS) pigeon->poopCount = MAX_POOPS;
                break;
            }
        }
    }

    // Atualizar fezes
    for (int i = 0; i < MAX_POOPS; i++) {
        if (pigeon->poops[i].active) {
            pigeon->poops[i].position.y += pigeon->poops[i].speed * deltaTime;
            pigeon->poops[i].hitbox = (Rectangle){
                pigeon->poops[i].position.x - POOP_WIDTH / 2,
                pigeon->poops[i].position.y - POOP_HEIGHT / 2,
                POOP_WIDTH,
                POOP_HEIGHT
            };

            // Remover fora da tela
            if (pigeon->poops[i].position.y > SCREEN_HEIGHT + 20) {
                pigeon->poops[i].active = 0;
                pigeon->poopCount--;
            }
        }
    }

    // Desativar se sair da tela
    if (pigeon->position.x < -PIGEON_WIDTH) {
        pigeon->active = 0;
    }
}

// ========== DESENHAR POMBO ==========
void drawPigeon(Pigeon pigeon) {
    if (!pigeon.active) return;

    if (pigeon.spriteLoaded) {
        DrawTextureEx(pigeon.texture,
                     (Vector2){ pigeon.position.x - PIGEON_WIDTH / 2, pigeon.position.y - PIGEON_HEIGHT / 2 },
                     0, 1.0f, WHITE);
    } else {
        // Placeholder: triângulo cinzento (pombo)
        DrawTriangle(
            (Vector2){ pigeon.position.x, pigeon.position.y - PIGEON_HEIGHT / 2 },
            (Vector2){ pigeon.position.x - PIGEON_WIDTH / 2, pigeon.position.y + PIGEON_HEIGHT / 2 },
            (Vector2){ pigeon.position.x + PIGEON_WIDTH / 2, pigeon.position.y + PIGEON_HEIGHT / 2 },
            GRAY
        );
        DrawTriangleLines(
            (Vector2){ pigeon.position.x, pigeon.position.y - PIGEON_HEIGHT / 2 },
            (Vector2){ pigeon.position.x - PIGEON_WIDTH / 2, pigeon.position.y + PIGEON_HEIGHT / 2 },
            (Vector2){ pigeon.position.x + PIGEON_WIDTH / 2, pigeon.position.y + PIGEON_HEIGHT / 2 },
            BLACK
        );
    }

    // Desenhar fezes
    for (int i = 0; i < MAX_POOPS; i++) {
        if (pigeon.poops[i].active) {
            DrawRectangle(pigeon.poops[i].hitbox.x, pigeon.poops[i].hitbox.y,
                         POOP_WIDTH, POOP_HEIGHT, BROWN);
            DrawRectangleLinesEx(pigeon.poops[i].hitbox, 1, DARKBROWN);
        }
    }

    // DrawRectangleLinesEx(pigeon.hitbox, 1, RED);
}

// ========== DESCARREGAR RECURSOS ==========
void unloadPigeonResources(Pigeon *pigeon) {
    if (pigeon->spriteLoaded) {
        UnloadTexture(pigeon->texture);
        pigeon->spriteLoaded = 0;
    }
}
