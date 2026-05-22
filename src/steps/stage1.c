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
    if (roll < 40) {

        qobs.type = QUEUE_OBS_HOLE;

        qobs.data.hole =
            createObstacle(
                (Vector2){
                    spawnX,
                    groundY
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

        qobs.data.bus =
            createBus(
                (Vector2){
                    spawnX,
                    groundY
                }
            );

        enqueueObstacle(
            &stage->obstacleQueue,
            qobs
        );
    }
    else if (roll < 95) {

        qobs.type = QUEUE_OBS_PIGEON;

        float pigeonY =
            stage->groundLevel -
            (GetScreenHeight() * 0.28f);

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

    // ===== OBTER DIMENSÕES DA TELA =====
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // ===== SINCRONIZAR PLAYER COM GROUND_Y FIXO =====
    float playerGroundY =
        stage->groundLevel - player->height;

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

    float targetX = player->position.x + 100.0f;  // Lookahead
    float targetY = player->position.y - 80.0f;

    // Aplicar damping suave (interpolação)
    stage->camera.target.x += (targetX - stage->camera.target.x) * stage->cameraDamping;
    stage->camera.target.y += (targetY - stage->camera.target.y) * stage->cameraDamping;

    // Manter bounds horizontais
    if (stage->camera.target.x < screenWidth * 0.5f) {
        stage->camera.target.x = screenWidth * 0.5f;
    }

    stage->camera.offset = (Vector2){ screenWidth * 0.25f, screenHeight * 0.6f };

    // ===== ATUALIZAR PARALLAX (Background move 0.3x speed) =====
    stage->parallaxOffset = stage->camera.target.x * 0.3f;

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

static void drawStage1HUD(Stage1 *stage, Player *player) {
    int fontSize = 16;

    // ===== CANTO SUPERIOR ESQUERDO: VIDAS, PONTOS, TEMPO =====
    char livesText[64];
    sprintf(livesText, "LIVES: %d / 3", player->lives);
    DrawText(livesText, 10, 10, fontSize, WHITE);

    char scoreText[64];
    sprintf(scoreText, "PONTOS: %.0f", player->score);
    DrawText(scoreText, 10, 30, fontSize, WHITE);

    char timeText[64];
    sprintf(timeText, "TEMPO: %ds", (int)stage->elapsedTime);
    DrawText(timeText, 10, 50, fontSize, WHITE);

    // ===== CANTO SUPERIOR DIREITO: DIFICULDADE =====
    char diffText[64];
    sprintf(diffText, "DIFICULDADE: x%.1f", stage->difficultyMultiplier);
    DrawText(diffText, 650, 10, fontSize, YELLOW);

    // ===== PROTEÇÃO COM GUARDA-CHUVA (inferior esquerdo se ativo) =====
    if (player->hasUmbrella > 0) {
        char umbrellaText[64];
        sprintf(umbrellaText, "UMBRELLA: %.1fs", player->umbrellaTimer);
        DrawText(umbrellaText, 10, 400, fontSize, SKYBLUE);
    }
}

void drawStage1(Stage1 *stage, Player *player) {

    BeginMode2D(stage->camera);

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
            bgScale;

        Rectangle source = {
            0,
            0,
            (float)stage->backgroundTexture.width,
            (float)stage->backgroundTexture.height
        };

        for (int i = -1; i < 3; i++) {

            float bgHeight =
                stage->backgroundTexture.height *
                bgScale;

            Rectangle dest = {
                i * bgWidth - bgScroll,
                0,
                bgWidth,
                bgHeight
            };

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

        float roadHeight =
            screenHeight * 0.18f;

        float platformScale =
            roadHeight /
            stage->platformTexture.height;

        float platformWidth =
            stage->platformTexture.width *
            platformScale;

        float roadY =
            groundY - roadHeight * 0.35f;

        Rectangle source = {
            0,
            0,
            (float)stage->platformTexture.width,
            (float)stage->platformTexture.height
        };

        for (int i = -1; i < 4; i++) {

            Rectangle dest = {
                i * platformWidth -
                fmod(stage->parallaxOffset,
                     platformWidth),

                roadY,

                platformWidth,
                roadHeight
            };

            DrawTexturePro(
                stage->platformTexture,
                source,
                dest,
                (Vector2){0,0},
                0,
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

    EndMode2D();

    drawStage1HUD(stage, player);
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
