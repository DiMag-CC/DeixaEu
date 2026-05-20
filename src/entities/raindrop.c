#include "raindrop.h"
#include <stdlib.h>

#define RAIN_SPEED 300.0f
#define RAIN_SPAWN_INTERVAL 0.05f

// ========== CRIAR SISTEMA DE CHUVA ==========
RainSystem createRainSystem(void) {
    RainSystem rain;
    rain.count = 0;
    rain.spawnTimer = 0.0f;
    rain.spawnInterval = RAIN_SPAWN_INTERVAL;

    for (int i = 0; i < MAX_RAINDROPS; i++) {
        rain.drops[i].position = (Vector2){ 0, 0 };
        rain.drops[i].speed = RAIN_SPEED;
        rain.drops[i].active = 0;
    }

    return rain;
}

// ========== ATUALIZAR CHUVA ==========
void updateRainSystem(RainSystem *rain, float deltaTime) {
    // Spawnar novas gotas
    rain->spawnTimer += deltaTime;
    if (rain->spawnTimer >= rain->spawnInterval && rain->count < MAX_RAINDROPS) {
        rain->spawnTimer = 0.0f;

        for (int i = 0; i < MAX_RAINDROPS; i++) {
            if (!rain->drops[i].active) {
                rain->drops[i].position.x = rand() % SCREEN_WIDTH;
                rain->drops[i].position.y = -10.0f;
                rain->drops[i].speed = RAIN_SPEED + (float)(rand() % 100);
                rain->drops[i].active = 1;
                rain->count++;
                break;
            }
        }
    }

    // Atualizar gotas
    for (int i = 0; i < MAX_RAINDROPS; i++) {
        if (rain->drops[i].active) {
            rain->drops[i].position.y += rain->drops[i].speed * deltaTime;

            // Remover gota fora da tela
            if (rain->drops[i].position.y > SCREEN_HEIGHT + 20) {
                rain->drops[i].active = 0;
                rain->count--;
            }
        }
    }
}

// ========== DESENHAR CHUVA ==========
void drawRainSystem(RainSystem rain) {
    for (int i = 0; i < MAX_RAINDROPS; i++) {
        if (rain.drops[i].active) {
            DrawLine(rain.drops[i].position.x, rain.drops[i].position.y,
                    rain.drops[i].position.x + 2, rain.drops[i].position.y + 10,
                    SKYBLUE);
        }
    }
}

// ========== RESETAR CHUVA ==========
void resetRainSystem(RainSystem *rain) {
    rain->count = 0;
    rain->spawnTimer = 0.0f;

    for (int i = 0; i < MAX_RAINDROPS; i++) {
        rain->drops[i].active = 0;
    }
}
