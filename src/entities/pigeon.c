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
    pigeon.scale = 0.8f;
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
            int screenHeight = GetScreenHeight();
            if (pigeon->poops[i].position.y > screenHeight + 20) {
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

    float scaledWidth = PIGEON_WIDTH * pigeon.scale;
    float scaledHeight = PIGEON_HEIGHT * pigeon.scale;

    if (pigeon.spriteLoaded) {
        DrawTextureEx(pigeon.texture,
                     (Vector2){ pigeon.position.x - scaledWidth / 2,
                               pigeon.position.y - scaledHeight / 2 },
                     0, pigeon.scale, WHITE);
    } else {
        // Placeholder visual melhorado: pombo reconhecível
        float centerX = pigeon.position.x;
        float centerY = pigeon.position.y;

        // Corpo (círculo)
        DrawCircle(centerX, centerY, 6.0f * pigeon.scale, GRAY);
        DrawCircleLines(centerX, centerY, 6.0f * pigeon.scale, BLACK);

        // Cabeça
        DrawCircle(centerX + 5.0f * pigeon.scale, centerY - 3.0f * pigeon.scale,
                  3.0f * pigeon.scale, DARKGRAY);

        // Olho
        DrawCircle(centerX + 6.5f * pigeon.scale, centerY - 4.0f * pigeon.scale,
                  1.0f * pigeon.scale, BLACK);

        // Asas (triângulos)
        DrawTriangle(
            (Vector2){ centerX - 4.0f * pigeon.scale, centerY },
            (Vector2){ centerX - 10.0f * pigeon.scale, centerY - 2.0f * pigeon.scale },
            (Vector2){ centerX - 10.0f * pigeon.scale, centerY + 2.0f * pigeon.scale },
            LIGHTGRAY
        );
        DrawTriangle(
            (Vector2){ centerX + 4.0f * pigeon.scale, centerY },
            (Vector2){ centerX + 10.0f * pigeon.scale, centerY - 2.0f * pigeon.scale },
            (Vector2){ centerX + 10.0f * pigeon.scale, centerY + 2.0f * pigeon.scale },
            LIGHTGRAY
        );
    }

    // Desenhar fezes com círculos
    for (int i = 0; i < MAX_POOPS; i++) {
        if (pigeon.poops[i].active) {
            DrawCircle(pigeon.poops[i].position.x, pigeon.poops[i].position.y,
                      4.0f, BROWN);
            DrawCircleLines(pigeon.poops[i].position.x, pigeon.poops[i].position.y,
                           4.0f, DARKBROWN);
        }
    }
}

// ========== DESCARREGAR RECURSOS ==========
void unloadPigeonResources(Pigeon *pigeon) {
    if (pigeon->spriteLoaded) {
        UnloadTexture(pigeon->texture);
        pigeon->spriteLoaded = 0;
    }
}
