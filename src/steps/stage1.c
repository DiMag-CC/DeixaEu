#include "stage1.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ========== HELPER: SPAWNAR OBSTÁCULOS ==========
static void spawnRandomObstacle(Stage1 *stage) {
    int roll = rand() % 100;
    int screenWidth = GetScreenWidth();
    Vector2 spawnPos = { screenWidth + 50, GROUND_LEVEL };

    QueueObstacle qobs;
    qobs.position = spawnPos;
    qobs.active = 1;

    if (roll < 40) {
        // 40% Buraco
        qobs.type = QUEUE_OBS_HOLE;
        qobs.data.hole = createObstacle(spawnPos, OBS_HOLE);
        enqueueObstacle(&stage->obstacleQueue, qobs);
    } else if (roll < 70) {
        // 30% Ônibus
        qobs.type = QUEUE_OBS_BUS;
        qobs.data.bus = createBus(spawnPos);
        enqueueObstacle(&stage->obstacleQueue, qobs);
    } else if (roll < 95) {
        // 25% Pombo
        qobs.type = QUEUE_OBS_PIGEON;
        qobs.data.pigeon = createPigeon((Vector2){ spawnPos.x, GROUND_LEVEL - 80 });
        enqueueObstacle(&stage->obstacleQueue, qobs);
    } else {
        // 5% Guarda-chuva (power-up)
        qobs.type = QUEUE_OBS_UMBRELLA;
        qobs.data.pigeon.position = (Vector2){ spawnPos.x, GROUND_LEVEL - 50 };
    }
}

// ========== HELPER: ATUALIZAR OBSTÁCULOS ==========
static void updateObstacles(Stage1 *stage, float deltaTime) {
    QueueNode *cur = stage->obstacleQueue.front;

    while (cur != NULL) {
        QueueObstacle *qobs = &cur->obstacle;

        // Atualizar específico por tipo
        switch (qobs->type) {
            case QUEUE_OBS_HOLE:
                updateObstacle(&qobs->data.hole, stage->scrollSpeed, deltaTime);
                break;

            case QUEUE_OBS_BUS:
                updateBus(&qobs->data.bus, stage->scrollSpeed, deltaTime);
                break;

            case QUEUE_OBS_PIGEON:
                updatePigeon(&qobs->data.pigeon, stage->scrollSpeed, deltaTime);
                break;

            case QUEUE_OBS_UMBRELLA:
                // Umbrella é coletável, não obstáculo
                break;
        }

        cur = cur->next;
    }

    // Remover obstáculos fora da tela
    removeOffscreenObstacles(&stage->obstacleQueue, -100.0f);
}

// ========== HELPER: COLISÕES COM OBSTÁCULOS ==========
static void handleCollisions(Stage1 *stage, Player *player) {
    QueueNode *cur = stage->obstacleQueue.front;

    while (cur != NULL) {
        QueueObstacle *qobs = &cur->obstacle;
        if (!qobs->active) {
            cur = cur->next;
            continue;
        }

        Rectangle obstacleHitbox;

        switch (qobs->type) {
            case QUEUE_OBS_HOLE:
                obstacleHitbox = qobs->data.hole.hitbox;
                if (qobs->data.hole.active && CheckCollisionRecs(player->hitbox, obstacleHitbox)) {
                    damagePlayer(player, 200.0f);
                    qobs->data.hole.active = 0;
                    qobs->active = 0;
                }
                break;

            case QUEUE_OBS_BUS:
                obstacleHitbox = qobs->data.bus.hitbox;
                if (qobs->data.bus.active && CheckCollisionRecs(player->hitbox, obstacleHitbox)) {
                    damagePlayer(player, 300.0f);
                    qobs->data.bus.active = 0;
                    qobs->active = 0;
                }
                break;

            case QUEUE_OBS_PIGEON:
                // Colisão com pombo
                obstacleHitbox = qobs->data.pigeon.hitbox;
                if (qobs->data.pigeon.active && CheckCollisionRecs(player->hitbox, obstacleHitbox)) {
                    damagePlayer(player, 100.0f);
                    qobs->data.pigeon.active = 0;
                    qobs->active = 0;
                }

                // Colisão com fezes do pombo
                for (int i = 0; i < MAX_POOPS; i++) {
                    Poop *poop = &qobs->data.pigeon.poops[i];
                    if (poop->active && CheckCollisionRecs(player->hitbox, poop->hitbox)) {
                        applySlowDown(player, 50.0f, 2.0f);
                        poop->active = 0;
                    }
                }
                break;

            case QUEUE_OBS_UMBRELLA:
                // TODO: Implementar coletável de guarda-chuva
                break;
        }

        cur = cur->next;
    }
}

// ========== HELPER: DESENHAR OBSTÁCULOS ==========
static void drawObstacles(Stage1 *stage) {
    QueueNode *cur = stage->obstacleQueue.front;

    while (cur != NULL) {
        QueueObstacle *qobs = &cur->obstacle;
        if (!qobs->active) {
            cur = cur->next;
            continue;
        }

        switch (qobs->type) {
            case QUEUE_OBS_HOLE:
                if (qobs->data.hole.active) {
                    drawObstacle(qobs->data.hole);
                }
                break;

            case QUEUE_OBS_BUS:
                if (qobs->data.bus.active) {
                    drawBus(qobs->data.bus);
                }
                break;

            case QUEUE_OBS_PIGEON:
                if (qobs->data.pigeon.active) {
                    drawPigeon(qobs->data.pigeon);
                }
                break;

            case QUEUE_OBS_UMBRELLA:
                break;
        }

        cur = cur->next;
    }
}

// ========== INICIALIZAR STAGE 1 ==========
void initStage1(Stage1 *stage) {
    stage->scrollSpeed = STAGE1_BASE_SCROLL_SPEED;
    stage->distanceTraveled = 0.0f;
    stage->spawnInterval = 1.5f;
    stage->obstacleSpawnTimer = 0.0f;
    stage->difficultyMultiplier = 1.0f;
    stage->elapsedTime = 0.0f;
    stage->stage1Complete = 0;
    stage->stage1Failed = 0;

    // Inicializar câmera side-scrolling
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    stage->camera.target = (Vector2){ screenWidth * 0.25f, screenHeight * 0.6f };
    stage->camera.offset = (Vector2){ screenWidth * 0.25f, screenHeight * 0.6f };
    stage->camera.rotation = 0.0f;
    stage->camera.zoom = 1.0f;

    // Inicializar bike
    stage->bike = createBike();

    // Inicializar chuva
    stage->rain = createRainSystem();

    // Inicializar fila de obstáculos
    initObstacleQueue(&stage->obstacleQueue);

    // Carregar background (placeholder se falhar)
    stage->bgLoaded = 0;
    stage->backgroundTexture = LoadTexture("assets/img/stage1_bg.png");
    if (stage->backgroundTexture.id != 0) {
        stage->bgLoaded = 1;
    }
}

// ========== ATUALIZAR STAGE 1 ==========
void updateStage1(Stage1 *stage, Player *player, float deltaTime) {
    if (stage->stage1Complete || stage->stage1Failed) {
        return;
    }

    stage->elapsedTime += deltaTime;

    // ===== AUMENTAR DIFICULDADE =====
    float progress = stage->distanceTraveled / STAGE1_TARGET_DISTANCE;
    if (progress > 1.0f) progress = 1.0f;
    stage->difficultyMultiplier = 1.0f + (progress * 1.0f);

    // ===== SCROLL SPEED DINÂMICO =====
    stage->scrollSpeed = STAGE1_BASE_SCROLL_SPEED +
                         (STAGE1_MAX_SCROLL_SPEED - STAGE1_BASE_SCROLL_SPEED) * progress;

    // ===== INTERVALO DE SPAWN DINÂMICO =====
    stage->spawnInterval = 1.5f - (progress * 0.8f);  // De 1.5s para 0.7s

    // ===== SPAWN DE OBSTÁCULOS =====
    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        stage->obstacleSpawnTimer = 0.0f;
        spawnRandomObstacle(stage);
    }

    // ===== ATUALIZAR ENTIDADES =====
    updateBike(&stage->bike, player, deltaTime);
    updateRainSystem(&stage->rain, deltaTime);
    updateObstacles(stage, deltaTime);

    // ===== COLISÕES =====
    handleCollisions(stage, player);

    // ===== DISTÂNCIA PERCORRIDA =====
    stage->distanceTraveled += stage->scrollSpeed * deltaTime;

    // ===== ATUALIZAR CÂMERA SIDE-SCROLLING =====
    stage->camera.target.x = player->position.x + 100.0f;
    stage->camera.target.y = player->position.y - 80.0f;

    // ===== VERIFICAR GAME OVER =====
    if (player->lives <= 0) {
        stage->stage1Failed = 1;
    }

    // ===== VERIFICAR VITÓRIA =====
    if (stage->distanceTraveled >= STAGE1_TARGET_DISTANCE) {
        stage->stage1Complete = 1;
    }
}

// ========== DESENHAR STAGE 1 ==========
void drawStage1(Stage1 *stage, Player *player) {
    // ===== INICIAR MODO 2D COM CÂMERA =====
    BeginMode2D(stage->camera);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // ===== DESENHAR BACKGROUND =====
    if (stage->bgLoaded) {
        float bgTileWidth = stage->backgroundTexture.width;
        float worldX = stage->camera.target.x - screenWidth;
        int firstTile = (int)(worldX / bgTileWidth);

        for (int i = -1; i < 3; i++) {
            float tileX = (firstTile + i) * bgTileWidth;
            DrawTextureEx(stage->backgroundTexture, (Vector2){ tileX, 0 }, 0, 1.0f, WHITE);
        }
    } else {
        // Placeholder: céu azul
        float worldWidth = stage->camera.target.x * 2 + screenWidth;
        DrawRectangle(-5000, 0, 10000, GROUND_LEVEL - 50, SKYBLUE);
    }

    // ===== DESENHAR ESTRADA =====
    float roadY = GROUND_LEVEL;
    float roadHeight = 60.0f;
    float roadX = (int)(stage->camera.target.x / 100) * 100;
    for (int i = -3; i < 5; i++) {
        float segmentX = roadX + (i * 100);
        DrawRectangle(segmentX, roadY, 100, roadHeight, (Color){100, 100, 100, 255});
        DrawLine(segmentX + 50, roadY, segmentX + 50, roadY + roadHeight, WHITE);
    }

    // ===== DESENHAR CHUVA (ATRÁS DE TUDO) =====
    drawRainSystem(stage->rain);

    // ===== DESENHAR OBSTÁCULOS =====
    drawObstacles(stage);

    // ===== DESENHAR PLAYER =====
    drawPlayer(*player);

    // ===== DESENHAR BIKE =====
    drawBike(stage->bike, *player);

    // ===== FINALIZAR MODO 2D =====
    EndMode2D();
}

// ========== DESCARREGAR RESOURCES STAGE 1 ==========
void unloadStage1(Stage1 *stage) {
    unloadBikeResources(&stage->bike);
    freeObstacleQueue(&stage->obstacleQueue);

    if (stage->bgLoaded) {
        UnloadTexture(stage->backgroundTexture);
        stage->bgLoaded = 0;
    }
}
