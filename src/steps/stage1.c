#include "stage1.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ========== HELPER: SPAWNAR OBSTÁCULOS ==========
static void spawnRandomObstacle(Stage1 *stage) {
    int roll = rand() % 100;
    int screenWidth = GetScreenWidth();
    Vector2 spawnPos = { screenWidth + 50, GROUND_Y };

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
        qobs.data.pigeon = createPigeon((Vector2){ spawnPos.x, GROUND_Y - 80 });
        enqueueObstacle(&stage->obstacleQueue, qobs);
    } else {
        // 5% Guarda-chuva (power-up)
        qobs.type = QUEUE_OBS_UMBRELLA;
        qobs.data.umbrella = createUmbrella((Vector2){ spawnPos.x, GROUND_Y - 50 });
        enqueueObstacle(&stage->obstacleQueue, qobs);
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
                // Umbrella é coletável, atualizar como item flutuante
                updateUmbrella(&qobs->data.umbrella, stage->scrollSpeed, deltaTime);
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
                if (qobs->data.umbrella.active) {
                    drawUmbrella(qobs->data.umbrella);
                }
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

    // Usar GROUND_Y constante para sincronização
    stage->groundLevel = GROUND_Y;

    // Carregar plataforma (chão)
    stage->platformLoaded = 0;
    stage->platformTexture = LoadTexture("assets/img/plataformLevel1.png");
    if (stage->platformTexture.id != 0) {
        stage->platformLoaded = 1;
    }

    // Inicializar parallax
    stage->parallaxOffset = 0.0f;
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

    // ===== OBTER DIMENSÕES DA TELA =====
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // ===== SINCRONIZAR PLAYER COM GROUND_Y FIXO =====
    float playerGroundY = GROUND_Y - player->height;

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

    // ===== ATUALIZAR ENTIDADES =====
    updateBike(&stage->bike, player, deltaTime);
    updateRainSystem(&stage->rain, deltaTime);
    updateCloudSystem(&stage->cloudSystem, stage->scrollSpeed, deltaTime);
    updateObstacles(stage, deltaTime);

    // ===== ATUALIZAR CÂMERA SIDE-SCROLLING COM DAMPING =====
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

// ========== DESENHAR HUD STAGE 1 ==========
static void drawStage1HUD(Stage1 *stage, Player *player) {
    int fontSize = 16;

    // ===== CANTO SUPERIOR ESQUERDO: VIDAS, PONTOS, TEMPO =====
    char livesText[64];
    sprintf(livesText, "LIVES: %d / 3", player->lives);
    DrawText(livesText, 10, 10, fontSize, WHITE);

    char scoreText[64];
    sprintf(scoreText, "PUNTOS: %.0f", player->score);
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

// ========== DESENHAR STAGE 1 ==========
void drawStage1(Stage1 *stage, Player *player) {
    // ===== INICIAR MODO 2D COM CÂMERA =====
    BeginMode2D(stage->camera);

    int screenWidth = GetScreenWidth();

    // ===== CAMADA 1: CÉU (parallax 0.1x - muito distante) =====
    float parallax_sky = stage->camera.target.x * 0.1f;
    DrawRectangle(-10000, 0, 20000, GROUND_Y - 100, (Color){135, 206, 235, 255});  // Céu azul

    // Nuvens procedurais no céu
    int cloudCount = 8;
    for (int i = 0; i < cloudCount; i++) {
        float cloudX = ((int)(parallax_sky / 300) % cloudCount + i) * 300 - parallax_sky;
        float cloudY = 50.0f + (i % 3) * 30;
        DrawCircle((int)cloudX - 30, (int)cloudY, 20, WHITE);
        DrawCircle((int)cloudX, (int)cloudY, 25, WHITE);
        DrawCircle((int)cloudX + 30, (int)cloudY, 20, WHITE);
    }

    // ===== CAMADA 2: BACKGROUND COM PARALLAX (0.3x) =====
    if (stage->bgLoaded) {
        float bgTileWidth = stage->backgroundTexture.width;
        float parallaxX = stage->parallaxOffset;
        int firstTile = (int)(parallaxX / bgTileWidth);

        for (int i = -1; i < 3; i++) {
            float tileX = (firstTile + i) * bgTileWidth + (stage->parallaxOffset - (int)stage->parallaxOffset / bgTileWidth * bgTileWidth);
            DrawTextureEx(stage->backgroundTexture, (Vector2){ tileX, 0 }, 0, 1.0f, WHITE);
        }
    } else {
        // Placeholder: cinza para prédios/paisagem
        DrawRectangle(-5000, 80, 10000, GROUND_Y - 130, (Color){100, 100, 120, 100});
    }

    // ===== DESENHAR NUVENS PROCEDURAIS E CHUVA =====
    drawCloudSystem(stage->cloudSystem);

    // ===== DESENHAR CHUVA LEGACY (MANTIDA PARA COMPATIBILIDADE) =====
    drawRainSystem(stage->rain);

    // ===== CAMADA 3: ELEMENTOS PRÓXIMOS (parallax 0.6x) =====
    float parallax_foreground = stage->camera.target.x * 0.6f;

    // Postes de estrada
    int poleCount = 5;
    for (int i = 0; i < poleCount; i++) {
        float poleX = ((int)(parallax_foreground / 400) % poleCount + i) * 400 - parallax_foreground;
        DrawRectangle((int)poleX - 5, GROUND_Y - 150, 10, 150, (Color){80, 80, 80, 200});
    }

    // ===== DESENHAR PLATAFORMA (CHÃO) COM SCROLL INFINITO =====
    float platformY = GROUND_Y;
    if (stage->platformLoaded) {
        float platformTileWidth = stage->platformTexture.width;
        float worldX = stage->camera.target.x - screenWidth;
        int firstTile = (int)(worldX / platformTileWidth);

        for (int i = -1; i < 4; i++) {
            float tileX = (firstTile + i) * platformTileWidth;
            DrawTextureEx(stage->platformTexture, (Vector2){ tileX, platformY }, 0, 1.0f, WHITE);
        }
    } else {
        // Placeholder: estrada cinza com linhas
        float roadHeight = 60.0f;
        float roadX = (int)(stage->camera.target.x / 100) * 100;
        for (int i = -3; i < 5; i++) {
            float segmentX = roadX + (i * 100);
            DrawRectangle(segmentX, platformY, 100, roadHeight, (Color){100, 100, 100, 255});
            DrawLine(segmentX + 50, platformY, segmentX + 50, platformY + roadHeight, YELLOW);
        }
    }

    // ===== DESENHAR OBSTÁCULOS =====
    drawObstacles(stage);

    // ===== DESENHAR PLAYER =====
    drawPlayer(*player);

    // ===== FINALIZAR MODO 2D =====
    EndMode2D();

    // ===== DESENHAR HUD (fora do modo câmera, no espaço de tela) =====
    // Nota: drawGameHUD em main.c é redundante, aqui temos versão local em stage1
    drawStage1HUD(stage, player);

    // ===== DEBUG MODE (se ativado externamente via main.c) =====
    // Debug é controlado por flag em main.c para evitar dependência circular
}

// ========== DESCARREGAR RESOURCES STAGE 1 ==========
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
