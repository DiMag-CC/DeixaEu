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

        float holeYOffset =
            (float)((rand() % 16) - 8);

        qobs.data.hole =
            createObstacle(
                (Vector2){
                    spawnX,
                    groundY + 20.0f + holeYOffset
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

        float busYOffset =
            (float)((rand() % 10) - 4);

        qobs.data.bus =
            createBus(
                (Vector2){
                    spawnX,
                    groundY + 120.0f + busYOffset
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
                    // ===== APLICAR DANO =====
                    damagePlayer(player, 200.0f);
                    
                    // ===== RESETAR POSIÇÃO (não deixar descer) =====
                    float groundY = GLOBAL_GROUND_LEVEL - player->height + 300.0f;
                    player->position.y = groundY;
                    player->velocity.y = 0.0f;
                    player->isGrounded = 1;
                    player->grounded = 1;
                    
                    // ===== ATUALIZAR HITBOX =====
                    player->hitbox.x =
                        player->position.x - player->width * 0.35f;

                    player->hitbox.y =
                        player->position.y - player->height + 20.0f;

                    player->hitbox.width =
                        player->width * 0.7f;

                    player->hitbox.height =
                        player->height - 20.0f;
                    
                    // Desativar obstáculo
                    qobs->data.hole.active = 0;
                    qobs->active = 0;
                }
                break;

            case QUEUE_OBS_BUS: {
                Bus *bus = &qobs->data.bus;
                
                if (!bus->active) {
                    cur = cur->next;
                    continue;
                }

                Rectangle busTopHitbox = bus->topHitbox;
                Rectangle busFullHitbox = bus->hitbox;

                float playerBottom = player->hitbox.y + player->hitbox.height;
                float playerTop = player->hitbox.y;
                float playerLeft = player->hitbox.x;
                float playerRight = player->hitbox.x + player->hitbox.width;

                float busTop = busFullHitbox.y;
                float busLeft = busFullHitbox.x;
                float busRight = busFullHitbox.x + busFullHitbox.width;
                float busBottom = busFullHitbox.y + busFullHitbox.height;

                if (
                    player->velocity.y > 0 &&
                    playerBottom >= busTop &&
                    playerBottom <= busTop + 40.0f &&
                    playerRight > busLeft + 20.0f &&
                    playerLeft < busRight - 20.0f
                ) {
                    // ===== PLAYER EM PÉ SOBRE O ÔNIBUS =====
                    
                    // Calcular posição Y correta (em cima do ônibus)
                    float newPlayerY = busTop - player->height + 20.0f;
                    
                    player->position.y = newPlayerY;

                    // Resetar velocidade vertical
                    player->velocity.y = 0.0f;

                    // Player está "groundado" (em pé)
                    player->isGrounded = 1;
                    player->grounded = 1;
                    player->isJumping = 0;

                    // Flag do ônibus: player está em cima
                    bus->playerOnTop = 1;

                    player->hitbox.x =
                        player->position.x - player->width * 0.35f;

                    player->hitbox.y =
                        player->position.y - player->height + 20.0f;

                    player->hitbox.width =
                        player->width * 0.7f;

                    player->hitbox.height =
                        player->height - 20.0f;
                }
                // =====================================
                // PLAYER SAIU DE CIMA DO ÔNIBUS
                // =====================================
                else if (bus->playerOnTop) {
                    if (!CheckCollisionRecs(player->hitbox, busTopHitbox)) {
                        bus->playerOnTop = 0;
                    }
                }
                // =====================================
                // BATIDA LATERAL OU POR BAIXO
                // =====================================
                else if (CheckCollisionRecs(player->hitbox, busFullHitbox)) {
                    float overlapLeft = 
                        (playerRight) - (busLeft);
                    
                    float overlapRight = 
                        (busRight) - (playerLeft);
                    
                    float overlapTop = 
                        (playerBottom) - (busTop);
                    
                    float overlapBottom = 
                        (busBottom) - (playerTop);

                    float minOverlap = overlapLeft;
                    int side = 0;

                    if (overlapRight < minOverlap) {
                        minOverlap = overlapRight;
                        side = 1;
                    }
                    if (overlapTop < minOverlap) {
                        minOverlap = overlapTop;
                        side = 2;
                    }
                    if (overlapBottom < minOverlap) {
                        minOverlap = overlapBottom;
                        side = 3;
                    }

                    switch (side) {
                        case 0:
                            damagePlayer(player, 300.0f);
                            player->position.x -= 50.0f;
                            player->velocity.x = -400.0f;
                            break;
                        case 1:
                            damagePlayer(player, 300.0f);
                            player->position.x += 50.0f;
                            player->velocity.x = 400.0f;
                            break;
                        case 2:
                            damagePlayer(player, 300.0f);
                            player->velocity.y = -300.0f;
                            break;
                        case 3:
                            damagePlayer(player, 300.0f);
                            player->velocity.y = -300.0f;
                            break;
                    }

                    bus->active = 0;
                    qobs->active = 0;
                }

                break;
            }

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
        screenWidth * 0.5f,
        screenHeight * 0.5f
    };

    stage->camera.rotation = 0.0f;
    stage->camera.zoom = STAGE1_CAMERA_ZOOM;
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
    stage->backgroundTexture = LoadTexture("assets/img/landscapeFase1New2.png");
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

    float progress = stage->distanceTraveled / STAGE1_TARGET_DISTANCE;
    if (progress > 1.0f) progress = 1.0f;
    stage->difficultyMultiplier = 1.0f + (progress * 1.0f);

    stage->scrollSpeed = STAGE1_BASE_SCROLL_SPEED +
                         (STAGE1_MAX_SCROLL_SPEED - STAGE1_BASE_SCROLL_SPEED) * progress;

    stage->spawnInterval = 1.5f - (progress * 0.8f);

    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        stage->obstacleSpawnTimer = 0.0f;
        spawnRandomObstacle(stage);
    }

    float playerGroundY =
        stage->groundLevel -
        player->height +
        112.0f;

    // ===== ATUALIZAR OBSTÁCULOS PRIMEIRO =====
    updateBike(&stage->bike, player, deltaTime);
    updateRainSystem(&stage->rain, deltaTime);
    updateCloudSystem(&stage->cloudSystem, stage->scrollSpeed, deltaTime);
    updateObstacles(stage, deltaTime);

    stage->parallaxOffset += stage->scrollSpeed * deltaTime;

    // ===== COLISÕES PRIMEIRO (ANTES de gravidade) =====
    handleCollisions(stage, player);

    // ===== GRAVIDADE (AGORA, DEPOIS DE COLISÕES) =====
    // Só aplica gravidade se NÃO está groundado!
    
    if (!player->isGrounded) {
        player->velocity.y +=
            900.0f * deltaTime;
    }

    // ===== COLISÃO COM O CHÃO =====
    
    if (
        !player->isGrounded &&
        player->position.y >= playerGroundY &&
        player->velocity.y >= 0
    ) {
        player->position.y =
            playerGroundY;

        player->velocity.y = 0.0f;

        player->isGrounded = 1;

        player->isJumping = 0;
    }

    // ===== SEGURANÇA: Nunca deixar player descer muito =====
    // Se por algum motivo ele cair abaixo do chão, resetar
    float maxGroundY = GLOBAL_GROUND_LEVEL + 500.0f;  // Limite máximo
    
    if (player->position.y > maxGroundY) {
        player->position.y = playerGroundY;
        player->velocity.y = 0.0f;
        player->isGrounded = 1;
    }
    // ====================================================

    // ===== VERIFICAR GAME OVER =====
    if (player->lives <= 0) {
        stage->stage1Failed = 1;
    }

    // ===== VERIFICAR VITÓRIA =====
    if (stage->distanceTraveled >= STAGE1_TARGET_DISTANCE) {
        stage->stage1Complete = 1;
    }

    // ===== DISTÂNCIA PERCORRIDA =====
    stage->distanceTraveled += stage->scrollSpeed * deltaTime;
}

void drawStage1(Stage1 *stage, Player *player) {

    BeginMode2D(stage->camera);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float groundY = stage->groundLevel;

    float bgScroll = fmod(stage->parallaxOffset * 0.2f,
             screenWidth);

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
                bgScale + 180.0f;

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

    EndMode2D();
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
