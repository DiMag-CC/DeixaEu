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
        qobs.data.umbrella = createUmbrella((Vector2){ spawnPos.x, GROUND_LEVEL - 50 });
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
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int fontSize = (screenWidth < 1024) ? 12 : 14;

    // ===== VIDAS E SCORE (canto superior esquerdo) =====
    char livesText[64];
    sprintf(livesText, "VIDAS: %d", player->lives);
    DrawText(livesText, 20, 20, fontSize, RED);

    char scoreText[64];
    sprintf(scoreText, "PONTOS: %.0f", player->score);
    DrawText(scoreText, 20, 45, fontSize, WHITE);

    // ===== PROGRESSO (centro superior) =====
    float progress = stage->distanceTraveled / STAGE1_TARGET_DISTANCE;
    if (progress > 1.0f) progress = 1.0f;

    char progressText[64];
    sprintf(progressText, "DISTANCIA: %.0fm / %.0fm", stage->distanceTraveled, STAGE1_TARGET_DISTANCE);
    int centerX = screenWidth / 2 - MeasureText(progressText, fontSize) / 2;
    DrawText(progressText, centerX, 20, fontSize, WHITE);

    // Barra de progresso
    int barY = 50;
    int barWidth = screenWidth - 40;
    DrawRectangle(20, barY, barWidth, 15, (Color){50, 50, 50, 200});
    DrawRectangle(20, barY, (int)(barWidth * progress), 15, (Color){0, 200, 100, 200});
    DrawRectangleLinesEx((Rectangle){20, barY, barWidth, 15}, 1, WHITE);

    // ===== DIFICULDADE (canto superior direito) =====
    char diffText[64];
    sprintf(diffText, "DIFICULDADE: x%.2f", stage->difficultyMultiplier);
    int rightX = screenWidth - MeasureText(diffText, fontSize) - 20;
    DrawText(diffText, rightX, 20, fontSize, YELLOW);

    // ===== PROTEÇÃO COM GUARDA-CHUVA =====
    if (player->hasUmbrella > 0) {
        char umbrellaText[64];
        sprintf(umbrellaText, "PROTEGIDO: %.1fs", player->umbrellaTimer);
        DrawText("☔", 20, screenHeight - 40, 20, LIGHTBLUE);
        DrawText(umbrellaText, 50, screenHeight - 35, fontSize, LIGHTBLUE);

        // Barra de proteção
        int protBarWidth = 150;
        float protProgress = player->umbrellaTimer / 8.0f;
        DrawRectangle(50, screenHeight - 15, protBarWidth, 10, (Color){50, 50, 100, 200});
        DrawRectangle(50, screenHeight - 15, (int)(protBarWidth * protProgress), 10, (Color){100, 200, 255, 200});
        DrawRectangleLinesEx((Rectangle){50, screenHeight - 15, protBarWidth, 10}, 1, LIGHTBLUE);
    }

    // ===== FPS (canto inferior direito) =====
    char fpsText[32];
    sprintf(fpsText, "FPS: %d", GetFPS());
    DrawText(fpsText, screenWidth - 100, screenHeight - 30, 12, LIME);
}

// ========== DESENHAR DEBUG OVERLAY ==========
static void drawStage1Debug(Stage1 *stage, Player *player) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // ===== GRID =====
    for (int x = 0; x < screenWidth; x += 50) {
        DrawLine(x, 0, x, screenHeight, (Color){50, 50, 50, 100});
    }
    for (int y = 0; y < screenHeight; y += 50) {
        DrawLine(0, y, screenWidth, y, (Color){50, 50, 50, 100});
    }

    // ===== PLAYER HITBOX =====
    DrawRectangleLinesEx(player->hitbox, 2, RED);
    char playerText[128];
    sprintf(playerText, "PLAYER: (%.0f, %.0f) | Vel: (%.0f, %.0f) | State: %d",
            player->position.x, player->position.y,
            player->velocity.x, player->velocity.y,
            player->state);
    DrawText(playerText, 20, 100, 11, RED);

    // ===== ENTIDADES ATIVAS =====
    int entityCount = 0;
    QueueNode *cur = stage->obstacleQueue.front;
    while (cur) {
        entityCount++;
        cur = cur->next;
    }
    char entityText[64];
    sprintf(entityText, "ENTIDADES: %d", entityCount);
    DrawText(entityText, 20, 125, 11, YELLOW);

    // ===== CAMERA =====
    char cameraText[128];
    sprintf(cameraText, "CAMERA: (%.0f, %.0f) | Scroll Speed: %.0f",
            stage->camera.target.x, stage->camera.target.y,
            stage->scrollSpeed);
    DrawText(cameraText, 20, 150, 11, LIGHTBLUE);

    // ===== INSTRUÇÃO PARA DESATIVAR =====
    DrawText("DEBUG MODE - Pressione D para desativar", 20, screenHeight - 50, 12, RED);
}

// ========== DESENHAR STAGE 1 ==========
void drawStage1(Stage1 *stage, Player *player) {
    // ===== INICIAR MODO 2D COM CÂMERA =====
    BeginMode2D(stage->camera);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // ===== CAMADA 1: CÉU (parallax 0.1x - muito distante) =====
    float parallax_sky = stage->camera.target.x * 0.1f;
    DrawRectangle(-10000, 0, 20000, GROUND_LEVEL - 100, (Color){135, 206, 235, 255});  // Céu azul

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
        DrawRectangle(-5000, 80, 10000, GROUND_LEVEL - 130, (Color){100, 100, 120, 100});
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
        DrawRectangle((int)poleX - 5, GROUND_LEVEL - 150, 10, 150, (Color){80, 80, 80, 200});
    }

    // ===== DESENHAR PLATAFORMA (CHÃO) COM SCROLL INFINITO =====
    float platformY = GROUND_LEVEL;
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

    // ===== DESENHAR BIKE =====
    drawBike(stage->bike, *player);

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
