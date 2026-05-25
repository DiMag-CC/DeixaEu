#include "stage1.h"
#include "../utils/gameConstants.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static float worldScale = 1.0f;

static void spawnRandomObstacle(Stage1 *stage) {

    int roll = rand() % 100;

    int screenWidth = GetScreenWidth();

    float groundY = stage->groundLevel;

    float spawnX =
        stage->camera.target.x +
        screenWidth;

    Vector2 spawnPos = {
        spawnX,
        groundY
    };

    QueueObstacle qobs;

    qobs.position = spawnPos;

    qobs.active = 1;
    int canSpawnBus =
        stage->distanceTraveled - stage->lastBusSpawnDistance >=
        STAGE1_BUS_MIN_DISTANCE;

    if (roll >= 40 && roll < 70 && !canSpawnBus) {
        roll = (rand() % 2 == 0) ? (rand() % 40) : (70 + rand() % 25);
    }

    if (roll < 40) {

        qobs.type = QUEUE_OBS_HOLE;

        qobs.data.hole =
            createObstacle(
                (Vector2){
                    spawnX,
                    groundY + 120.0f
                },
                OBS_HOLE
            );

        enqueueObstacle(
            &stage->obstacleQueue,
            qobs
        );
    }
    else if (roll < 70) {

        qobs.type = QUEUE_OBS_BUS;
        stage->lastBusSpawnDistance = stage->distanceTraveled;

        qobs.data.bus =
            createBus(
                (Vector2){
                    spawnX,
                    groundY + 120.0f
                }
            );

        enqueueObstacle(
            &stage->obstacleQueue,
            qobs
        );
    }
    else if (roll < 95) {

        qobs.type = QUEUE_OBS_PIGEON;

        float minPigeonY =
            groundY - (GetScreenHeight() * 0.48f);

        float maxPigeonY =
            groundY - (GetScreenHeight() * 0.28f);

        float pigeonY =
            minPigeonY +
            ((float)rand() / RAND_MAX) *
            (maxPigeonY - minPigeonY);

        qobs.data.pigeon =
            createPigeon(
                (Vector2){
                    spawnPos.x,
                    pigeonY
                }
            );

        enqueueObstacle(
            &stage->obstacleQueue,
            qobs
        );
    }
    else {

        qobs.type = QUEUE_OBS_UMBRELLA;

        qobs.data.umbrella =
            createUmbrella(
                (Vector2){
                    spawnX,
                    groundY - 120.0f
                }
            );

        enqueueObstacle(
            &stage->obstacleQueue,
            qobs
        );
    }
}

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
                // Umbrella é coletável, atualizar como item flutuante
                updateUmbrella(&qobs->data.umbrella, stage->scrollSpeed, deltaTime);
                break;
        }

        cur = cur->next;
    }

    // Remover obstáculos fora da tela
    removeOffscreenObstacles(&stage->obstacleQueue, -100.0f);
}

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
                        // Se tem umbrella, não aplica debuff
                        if (player->hasUmbrella <= 0) {
                            applySlowDown(player, 50.0f, 2.0f);  // 50% slowdown por 2 segundos
                        }
                        poop->active = 0;
                    }
                }
                break;

            case QUEUE_OBS_UMBRELLA:
                // Colisão com guarda-chuva (coletável)
                obstacleHitbox = qobs->data.umbrella.hitbox;
                if (qobs->data.umbrella.active && CheckCollisionRecs(player->hitbox, obstacleHitbox)) {
                    // Coletar umbrella
                    addUmbrellaShield(player, 8.0f);  // 8 segundos de proteção
                    qobs->data.umbrella.active = 0;
                    qobs->active = 0;
                }
                break;
        }

        cur = cur->next;
    }
}

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
                if (qobs->data.umbrella.active) {
                    drawUmbrella(qobs->data.umbrella);
                }
                break;
        }

        cur = cur->next;
    }
}

void initStage1(Stage1 *stage) {

    GLOBAL_WORLD_SCALE =
        (float)GetScreenHeight() / BASE_SCREEN_HEIGHT;

    GLOBAL_GROUND_LEVEL =
        GetScreenHeight() * 0.82f;

    stage->scrollSpeed = STAGE1_BASE_SCROLL_SPEED;
    stage->distanceTraveled = 0.0f;
    stage->lastBusSpawnDistance = -STAGE1_BUS_MIN_DISTANCE;
    stage->spawnInterval = 1.5f;
    stage->obstacleSpawnTimer = 0.0f;
    stage->difficultyMultiplier = 1.0f;
    stage->elapsedTime = 0.0f;
    stage->stage1Complete = 0;
    stage->stage1Failed = 0;

    // Inicializar câmera side-scrolling
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    worldScale = (float)screenHeight / 720.0f;

    stage->camera.target =
    (Vector2){ screenWidth * 0.5f,
            screenHeight * 0.5f };

    stage->camera.offset =
    (Vector2){
        screenWidth * 0.35f,
        screenHeight * 0.50f
    };

    stage->camera.rotation = 0.0f;
    stage->camera.zoom = 1.0f;
    stage->cameraDamping = 0.15f;  // Damping suave para câmera responsiva

    // Inicializar bike
    stage->bike = createBike();

    // Inicializar chuva
    stage->rain = createRainSystem();

    // Inicializar sistema de nuvens
    stage->cloudSystem = createCloudSystem();

    // Inicializar fila de obstáculos
    initObstacleQueue(&stage->obstacleQueue);

    // Carregar background com parallax
    stage->bgLoaded = 0;
    stage->backgroundTexture = LoadTexture("assets/img/landscapeLevel1.png");
    if (stage->backgroundTexture.id != 0) {
        stage->bgLoaded = 1;
    }

    stage->groundLevel =
        GetScreenHeight() * GROUND_Y_RATIO;

    // Carregar plataforma (chão)
    stage->platformLoaded = 0;
    stage->platformTexture = LoadTexture("assets/img/plataformLevel1.png");
    if (stage->platformTexture.id != 0) {
        stage->platformLoaded = 1;
    }

    // Inicializar parallax
    stage->parallaxOffset = 0.0f;
}

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

    float playerGroundY =
        stage->groundLevel - player->height + 112.0f;

    // Se o player está muito acima do chão, trazer com gravidade
    if (player->position.y < playerGroundY - 5.0f) {
        player->velocity.y += 600.0f * deltaTime;
    } else if (player->position.y > playerGroundY) {
        // Player no chão
        player->position.y = playerGroundY;
        player->velocity.y = 0;
        player->isGrounded = 1;
    } else {
        player->isGrounded = 1;
    }


    updateBike(&stage->bike, player, deltaTime);
    updateRainSystem(&stage->rain, deltaTime);
    updateCloudSystem(&stage->cloudSystem, stage->scrollSpeed, deltaTime);
    updateObstacles(stage, deltaTime);

    stage->parallaxOffset += stage->scrollSpeed * deltaTime;

    // ===== COLISÕES =====
    handleCollisions(stage, player);

    // ===== DISTÂNCIA PERCORRIDA =====
    stage->distanceTraveled += stage->scrollSpeed * deltaTime;

    // ===== VERIFICAR GAME OVER =====
    if (player->lives <= 0) {
        stage->stage1Failed = 1;
    }

    // ===== VERIFICAR VITÓRIA =====
    if (stage->distanceTraveled >= STAGE1_TARGET_DISTANCE) {
        stage->stage1Complete = 1;
    }
}

void drawStage1(Stage1 *stage, Player *player) {

    // BeginMode2D(stage->camera);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float groundY = stage->groundLevel;

    float bgScroll = fmod(stage->parallaxOffset * 0.2f,
             screenWidth);

    // =========================================
    // CAMADA 1 — CÉU
    // =========================================
    DrawRectangle(
        -10000,
        -10000,
        20000,
        20000,
        (Color){135, 206, 235, 255}
    );

    // =========================================
    // NUVENS
    // =========================================
    for (int i = 0; i < 8; i++) {

        float cloudX =
            i * 400 - bgScroll;

        float cloudY =
            screenHeight * 0.12f +
            (i % 3) * 35;

        DrawCircle(cloudX - 30, cloudY, 20, WHITE);
        DrawCircle(cloudX,      cloudY, 28, WHITE);
        DrawCircle(cloudX + 30, cloudY, 20, WHITE);
    }

    // =========================================
    // CAMADA 2 — BACKGROUND
    // =========================================
    if (stage->bgLoaded &&
        stage->backgroundTexture.id != 0) {

        float bgScale =
            ((float)screenHeight * 0.72f) /
            stage->backgroundTexture.height;

        float bgWidth =
            stage->backgroundTexture.width *
            bgScale + 360.0f;

        Rectangle source = {
            0,
            0,
            (float)stage->backgroundTexture.width,
            (float)stage->backgroundTexture.height
        };

        for (int i = -1; i < 3; i++) {

            float bgHeight =
                stage->backgroundTexture.height *
                bgScale + 360.0f;

            Rectangle dest = {
                i * bgWidth - bgScroll,
                0,
                bgWidth,
                bgHeight
            };

            dest.y = -24.0f;

            DrawTexturePro(
                stage->backgroundTexture,
                source,
                dest,
                (Vector2){0,0},
                0,
                WHITE
            );
        }
    }

    // =========================================
    // PLATAFORMA / ESTRADA
    // =========================================
    if (stage->platformLoaded &&
        stage->platformTexture.id != 0) {

        float platformWidth =
            stage->platformTexture.width + 50.0f;

        float platformHeight =
            stage->platformTexture.height + 12.0f;

        float roadY =
            screenHeight -
            platformHeight + 520.0f;

        Rectangle source = {
            0,
            0,
            (float)stage->platformTexture.width,
            (float)stage->platformTexture.height
        };

        for (int i = -1; i < 4; i++) {

            float perspectiveOffset = 80.0f;

                Rectangle dest = {
                    i * platformWidth -
                    fmod(stage->parallaxOffset,
                        platformWidth),

                    roadY + 2.0f,

                    platformWidth + perspectiveOffset,
                    platformHeight
                };

            DrawTexturePro(
                stage->platformTexture,
                source,
                dest,
                (Vector2){0,0},
                0.0f,
                WHITE
            );

        }

    } else {

        DrawRectangle(
            -5000,
            groundY - 40,
            10000,
            80,
            DARKGRAY
        );
    }

    drawCloudSystem(stage->cloudSystem);
    drawRainSystem(stage->rain);

    drawObstacles(stage);

    drawPlayer(*player);

    // EndMode2D();
}

void unloadStage1(Stage1 *stage) {
    unloadBikeResources(&stage->bike);
    freeObstacleQueue(&stage->obstacleQueue);
    resetCloudSystem(&stage->cloudSystem);

    if (stage->bgLoaded) {
        UnloadTexture(stage->backgroundTexture);
        stage->bgLoaded = 0;
    }

    if (stage->platformLoaded) {
        UnloadTexture(stage->platformTexture);
        stage->platformLoaded = 0;
    }
}
