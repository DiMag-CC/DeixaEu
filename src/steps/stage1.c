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

        if (!qobs->active) {
            cur = cur->next;
            continue;
        }

        // Atualizar espec??fico por tipo
        switch (qobs->type) {
            case QUEUE_OBS_HOLE:
                updateObstacle(&qobs->data.hole, stage->scrollSpeed, deltaTime);
                qobs->position = qobs->data.hole.position;
                qobs->active = qobs->data.hole.active;
                break;

            case QUEUE_OBS_BUS:
                updateBus(&qobs->data.bus, stage->scrollSpeed, deltaTime);
                qobs->position = qobs->data.bus.position;
                qobs->active = qobs->data.bus.active;
                break;

            case QUEUE_OBS_PIGEON:
                updatePigeon(&qobs->data.pigeon, stage->scrollSpeed, deltaTime);
                qobs->position = qobs->data.pigeon.position;
                qobs->active = qobs->data.pigeon.active;
                break;

            case QUEUE_OBS_UMBRELLA:
                // Umbrella ?? colet??vel, atualizar como item flutuante
                updateUmbrella(&qobs->data.umbrella, stage->scrollSpeed, deltaTime);
                qobs->position = qobs->data.umbrella.position;
                qobs->active = qobs->data.umbrella.active;
                break;
        }

        cur = cur->next;
    }

    // Remover obst??culos fora da tela
    removeOffscreenObstacles(&stage->obstacleQueue, -100.0f);
}

static void handleCollisions(Stage1 *stage, Player *player) {
    QueueNode *cur = stage->obstacleQueue.front;
    int standingOnBus = 0;

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
                if (qobs->data.hole.active) {
                    Rectangle holeDamageHitbox = obstacleHitbox;
                    holeDamageHitbox.x -= 8.0f;
                    holeDamageHitbox.width += 16.0f;
                    holeDamageHitbox.y -= 6.0f;
                    holeDamageHitbox.height += 14.0f;

                    Rectangle playerFootProbe = {
                        player->hitbox.x + 8.0f,
                        player->hitbox.y + player->hitbox.height - 6.0f,
                        player->hitbox.width - 16.0f,
                        12.0f
                    };

                    float playerGroundY = GLOBAL_GROUND_LEVEL - player->height + 112.0f;
                    int nearGround = player->position.y >= playerGroundY - 25.0f;
                    int touchingHole =
                        CheckCollisionRecs(player->hitbox, holeDamageHitbox) ||
                        CheckCollisionRecs(playerFootProbe, holeDamageHitbox);

                    if (!nearGround || !touchingHole) {
                        break;
                    }

                    // ===== APLICAR DANO =====
                    damagePlayer(player, 200.0f);
                    
                    // ===== RESETAR POSI????O (n??o deixar descer) =====
                    float groundY = GLOBAL_GROUND_LEVEL - player->height + 112.0f;
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
                    
                    // Desativar obst??culo
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

                Rectangle busStandHitbox = bus->standPlatformHitbox;
                Rectangle busFullHitbox = bus->hitbox;
                Rectangle busDamageHitbox = busFullHitbox;
                float busDeltaX = bus->position.x - bus->prevX;

                // Hitbox de dano reduzida: metade do ônibus e sem a faixa superior.
                // A plataforma de cima continua inteira no busStandHitbox.
                busDamageHitbox.x += busDamageHitbox.width * 0.25f;
                busDamageHitbox.width *= 0.5f;
                busDamageHitbox.y += busDamageHitbox.height * 0.28f;
                busDamageHitbox.height *= 0.72f;

                // Evita dano indevido na borda esquerda da tela:
                // quando o ônibus está saindo do mapa, removemos sua colisão.
                if (busFullHitbox.x + busFullHitbox.width <= 8.0f) {
                    bus->playerOnTop = 0;
                    bus->active = 0;
                    qobs->active = 0;
                    break;
                }

                float playerBottom = player->hitbox.y + player->hitbox.height;
                float playerTop = player->hitbox.y;
                float playerLeft = player->hitbox.x;
                float playerRight = player->hitbox.x + player->hitbox.width;

                float busTop = busDamageHitbox.y;
                float busLeft = busDamageHitbox.x;
                float busRight = busDamageHitbox.x + busDamageHitbox.width;
                float busBottom = busDamageHitbox.y + busDamageHitbox.height;

                Rectangle playerFootProbe = {
                    player->hitbox.x + 6.0f,
                    player->hitbox.y + player->hitbox.height - 5.0f,
                    player->hitbox.width - 12.0f,
                    10.0f
                };

                int horizontalStandOverlap =
                    playerFootProbe.x + playerFootProbe.width > busStandHitbox.x + 4.0f &&
                    playerFootProbe.x < busStandHitbox.x + busStandHitbox.width - 4.0f;

                int nearStandHeight =
                    playerBottom >= busStandHitbox.y - 12.0f &&
                    playerBottom <= busStandHitbox.y + 18.0f;

                int descendingOrStable = (player->velocity.y >= -30.0f);

                int canSnapOnStand = horizontalStandOverlap &&
                                     nearStandHeight &&
                                     descendingOrStable &&
                                     playerTop < busStandHitbox.y + busStandHitbox.height;

                int tryingToJump = IsKeyDown(KEY_SPACE) || IsKeyPressed(KEY_SPACE);
                int keepOnStand = bus->playerOnTop &&
                                  horizontalStandOverlap &&
                                  playerBottom <= busStandHitbox.y + 28.0f &&
                                  playerTop < busStandHitbox.y + busStandHitbox.height &&
                                  !tryingToJump &&
                                  player->velocity.y >= -40.0f;

                if (canSnapOnStand || keepOnStand) {
                    // Plataforma fina invis??vel: estabiliza o player sem flicker.
                    player->position.x += busDeltaX;
                    player->position.y = busStandHitbox.y + 4.0f;
                    player->velocity.y = 0.0f;
                    player->isGrounded = 1;
                    player->grounded = 1;
                    player->isJumping = 0;

                    bus->playerOnTop = 1;
                    standingOnBus = 1;

                    player->hitbox.x =
                        player->position.x - player->width * 0.35f;

                    player->hitbox.y =
                        player->position.y - player->height + 20.0f;

                    player->hitbox.width =
                        player->width * 0.7f;

                    player->hitbox.height =
                        player->height - 20.0f;
                } else if (bus->playerOnTop) {
                    bus->playerOnTop = 0;
                }

                // =====================================
                // BATIDA LATERAL OU POR BAIXO
                // =====================================
                if (!standingOnBus && CheckCollisionRecs(player->hitbox, busDamageHitbox)) {
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
                // Colis??o com pombo
                obstacleHitbox = qobs->data.pigeon.hitbox;
                if (qobs->data.pigeon.active && CheckCollisionRecs(player->hitbox, obstacleHitbox)) {
                    damagePlayer(player, 100.0f);
                    qobs->data.pigeon.active = 0;
                    qobs->active = 0;
                }

                // Colis??o com fezes do pombo
                for (int i = 0; i < MAX_POOPS; i++) {
                    Poop *poop = &qobs->data.pigeon.poops[i];
                    if (poop->active && CheckCollisionRecs(player->hitbox, poop->hitbox)) {
                        // Sem umbrella, o coc?? causa dano e lentid??o.
                        if (player->hasUmbrella <= 0) {
                            damagePlayer(player, 110.0f);
                            applySlowDown(player, 50.0f, 2.0f);  // 50% slowdown por 2 segundos
                        }
                        poop->active = 0;
                        break;
                    }
                }
                break;

            case QUEUE_OBS_UMBRELLA:
                // Colis??o com guarda-chuva (colet??vel)
                obstacleHitbox = qobs->data.umbrella.hitbox;
                if (qobs->data.umbrella.active && CheckCollisionRecs(player->hitbox, obstacleHitbox)) {
                    // Coletar umbrella
                    addUmbrellaShield(player, 8.0f);  // 8 segundos de prote????o
                    qobs->data.umbrella.active = 0;
                    qobs->active = 0;
                }
                break;
        }

        cur = cur->next;
    }

    player->movementControlledExternally = standingOnBus;
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

    // Inicializar c??mera side-scrolling
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
    stage->cameraDamping = 0.15f;

    // Inicializar bike
    stage->bike = createBike();

    // Inicializar chuva
    stage->rain = createRainSystem();

    // Inicializar sistema de nuvens
    stage->cloudSystem = createCloudSystem();

    // Inicializar fila de obst??culos
    initObstacleQueue(&stage->obstacleQueue);

    // Carregar background com parallax
    stage->bgLoaded = 0;
    stage->backgroundTexture = LoadTexture("assets/img/landscapeFase1New2.png");
    if (stage->backgroundTexture.id != 0) {
        stage->bgLoaded = 1;
    }

    stage->groundLevel =
        GetScreenHeight() * GROUND_Y_RATIO;

    // Carregar plataforma (ch??o)
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

    // ===== ATUALIZAR OBST??CULOS PRIMEIRO =====
    updateBike(&stage->bike, player, deltaTime);
    updateRainSystem(&stage->rain, deltaTime);
    updateCloudSystem(&stage->cloudSystem, stage->scrollSpeed, deltaTime);
    updateObstacles(stage, deltaTime);

    stage->parallaxOffset += stage->scrollSpeed * deltaTime;

    // ===== COLIS??ES PRIMEIRO (ANTES de gravidade) =====
    handleCollisions(stage, player);

    // ===== VERIFICAR GAME OVER =====
    if (player->lives <= 0) {
        stage->stage1Failed = 1;
    }

    // ===== VERIFICAR VIT??RIA =====
    if (stage->distanceTraveled >= STAGE1_TARGET_DISTANCE) {
        stage->stage1Complete = 1;
    }

    // ===== DIST??NCIA PERCORRIDA =====
    stage->distanceTraveled += stage->scrollSpeed * deltaTime;
}

void drawStage1(Stage1 *stage, Player *player) {

    // BeginMode2D(stage->camera);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float groundY = stage->groundLevel;

    float bgScroll = fmod(stage->parallaxOffset * 0.2f,
             screenWidth);

    // =========================================
    // CAMADA 2 ??? BACKGROUND
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

        Rectangle source = {
            0,
            stage->platformTexture.height * 0.29f,
            (float)stage->platformTexture.width,
            stage->platformTexture.height * 0.36f
        };

        float platformHeight = screenHeight * 0.34f;
        float platformWidth = source.width * (platformHeight / source.height);
        float roadY = groundY - screenHeight * 0.03f;

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
