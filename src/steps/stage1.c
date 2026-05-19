#include "stage1.h"
#include "../entities/pigeon.h"
#include "../entities/raindrop.h"
#include "../entities/umbrella.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void initStage1(Stage1 *stage) {
    initObstacleQueue(&stage->obstacleQueue);
    stage->obstacleSpawnTimer = 0.0f;
    stage->spawnInterval = 1.5f;
    
    stage->scrollSpeed = 100.0f;
    stage->roadPosition = 0.0f;
    stage->distanceTraveled = 0.0f;
    stage->stage1Complete = 0;
    stage->difficultyMultiplier = 1.0f;
    
    // ========== CARREGAR TEXTURAS COM VALIDAÇÃO ==========
    stage->background = LoadTexture("assets/img/background1.png");
    if (stage->background.id == 0) {
        fprintf(stderr, "ERRO: Não foi possível carregar background1.png\n");
        fflush(stderr);
    }
    
    for (int i = 0; i < 5; i++) {
        char fileName[128];
        sprintf(fileName, "assets/img/building%d.png", i + 1);
        stage->buildings[i] = LoadTexture(fileName);
        
        if (stage->buildings[i].id == 0) {
            fprintf(stderr, "AVISO: building%d.png não encontrado\n", i + 1);
            fflush(stderr);
        }
    }
    
    printf("Stage1 inicializado com sucesso. Assets carregados.\n");
    fflush(stdout);
    
    stage->pigeonCount = 0;
    for (int i = 0; i < MAX_PIGEONS; i++) {
        stage->pigeons[i].isActive = 0;
    }
    
    stage->raindropCount = 0;
    stage->rainSpawnTimer = 0.0f;
    stage->rainSpawnInterval = 0.2f;  // 5 gotas por segundo
    for (int i = 0; i < MAX_RAINDROPS; i++) {
        stage->raindrops[i].isActive = 0;
    }
    

    stage->umbrellasSpawned = 0;
    stage->umbrellas[0] = createUmbrella((Vector2){150, GROUND_LEVEL - 20});
    stage->umbrellas[1] = createUmbrella((Vector2){650, GROUND_LEVEL - 20});
}


void updateStage1(Stage1 *stage, Player *player, float deltaTime) {

    stage->distanceTraveled += stage->scrollSpeed * deltaTime;
    stage->roadPosition += stage->scrollSpeed * deltaTime;
    
    if (stage->roadPosition >= GetScreenWidth()) {
        stage->roadPosition = 0.0f;
    }
    
    // Aumentar dificuldade ao longo do tempo
    stage->difficultyMultiplier = 1.0f + (stage->distanceTraveled / 10000.0f) * 0.5f;
    stage->spawnInterval = 1.5f / stage->difficultyMultiplier;
    stage->scrollSpeed = 100.0f + (stage->difficultyMultiplier - 1.0f) * 50.0f;

    stage->obstacleSpawnTimer += deltaTime;
    
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        
        int randomType = rand() % 100;
        int obstacleType = -1;
        
        if (randomType < 35) {
            obstacleType = OBSTACLE_POTHOLE;
        } else if (randomType < 65) {
            obstacleType = OBSTACLE_PIGEON_POOP;
        } else if (randomType < 85) {
            obstacleType = OBSTACLE_BUS;
        } else {
            // 15% para pombo - vai para array separado!
            obstacleType = -1;  // Flag especial
        }
        if (obstacleType == -1) {
            // Adicionar pombo ao array separado
            for (int i = 0; i < MAX_PIGEONS; i++) {
                if (!stage->pigeons[i].isActive) {
                    Vector2 spawnPos = {GetScreenWidth(), 50 + (rand() % 100)};
                    stage->pigeons[i] = createPigeon(spawnPos);
                    break;
                }
            }
        } else {
            Vector2 spawnPos = {GetScreenWidth(), GROUND_LEVEL + 20};
            
            // Posição especial para fezes
            if (obstacleType == OBSTACLE_PIGEON_POOP) {
                spawnPos.y = 100 + (rand() % 150);
            }
            
            Obstacle newObstacle = createObstacle(spawnPos, obstacleType);
            enqueueObstacle(&stage->obstacleQueue, newObstacle);
        }
        
        stage->obstacleSpawnTimer = 0.0f;
    }

    for (int i = 0; i < MAX_PIGEONS; i++) {
        if (stage->pigeons[i].isActive) {
            // Pombo se move para esquerda
            stage->pigeons[i].position.x -= stage->scrollSpeed * deltaTime;
            stage->pigeons[i].hitbox.x = stage->pigeons[i].position.x;
            
            // updatePigeon retorna 1 quando deve soltar fezes
            int shouldPoop = updatePigeon(&stage->pigeons[i], deltaTime);
            
            if (shouldPoop) {
                // Criar fezes abaixo do pombo
                Vector2 poopPos = {
                    stage->pigeons[i].position.x,
                    stage->pigeons[i].position.y + PIGEON_HEIGHT
                };
                
                Obstacle poop = createObstacle(poopPos, OBSTACLE_PIGEON_POOP);
                enqueueObstacle(&stage->obstacleQueue, poop);
            }
            
            // Desativar se saiu da tela
            if (stage->pigeons[i].position.x < -PIGEON_WIDTH) {
                stage->pigeons[i].isActive = 0;
            }
        }
    }

    stage->rainSpawnTimer += deltaTime;
    
    if (stage->rainSpawnTimer >= stage->rainSpawnInterval) {
        // Criar nova gota em posição aleatória no topo
        for (int i = 0; i < MAX_RAINDROPS; i++) {
            if (!stage->raindrops[i].isActive) {
                Vector2 rainPos = {rand() % GetScreenWidth(), 0};
                stage->raindrops[i] = createRaindrop(rainPos);
                stage->raindropCount++;
                break;
            }
        }
        stage->rainSpawnTimer = 0.0f;
    }
    
    // Atualizar todas as gotas
    for (int i = 0; i < MAX_RAINDROPS; i++) {
        if (stage->raindrops[i].isActive) {
            updateRaindrop(&stage->raindrops[i], deltaTime);
            
            // Detectar colisão com player
            if (CheckCollisionRecs(player->hitbox, stage->raindrops[i].hitbox)) {
                // Se sem proteção, aplicar lentidão
                if (player->hasUmbrella == 0) {
                    player->speed = 50.0f;  // Lentidão
                }
                // Gota é consumida
                stage->raindrops[i].isActive = 0;
            }
        }
    }

    ObstacleNode *current = stage->obstacleQueue.front;
    
    while (current != NULL) {
        // Mover obstáculo para esquerda
        current->obstacle.position.x -= stage->scrollSpeed * deltaTime;
        
        // Atualizar hitbox específico do tipo
        switch (current->obstacle.type) {
            case OBSTACLE_POTHOLE:
                current->obstacle.data.pothole.hitbox.x = current->obstacle.position.x;
                break;
                
            case OBSTACLE_PIGEON_POOP: {
                PigeonPoop *poop = &current->obstacle.data.pigeonPoop;
                poop->position.x = current->obstacle.position.x;
                poop->hitbox.x = current->obstacle.position.x;
                
                // Fezes cai
                if (!poop->onGround) {
                    poop->position.y += poop->fallSpeed * deltaTime;
                    poop->hitbox.y = poop->position.y;
                    
                    if (poop->position.y >= GROUND_LEVEL - 20) {
                        poop->position.y = GROUND_LEVEL - 20;
                        poop->onGround = 1;
                    }
                }
                break;
            }
                
            case OBSTACLE_BUS:
                current->obstacle.data.bus.position.x = current->obstacle.position.x;
                current->obstacle.data.bus.hitbox.x = current->obstacle.position.x;
                break;
                
            default:
                break;
        }
        
        current = current->next;
    }

    while (!isObstacleQueueEmpty(&stage->obstacleQueue) &&
           stage->obstacleQueue.front->obstacle.position.x < -150) {
        // Dar pontos por evitar
        player->score += 10 * (int)stage->difficultyMultiplier;
        dequeueObstacle(&stage->obstacleQueue);
    }

    current = stage->obstacleQueue.front;
    while (current != NULL) {
        int hasCollided = 0;
        
        switch (current->obstacle.type) {
            case OBSTACLE_POTHOLE:
                if (CheckCollisionRecs(player->hitbox, current->obstacle.data.pothole.hitbox)) {
                    hasCollided = 1;
                    player->lives -= 1;
                }
                break;
                
            case OBSTACLE_PIGEON_POOP:
                if (CheckCollisionRecs(player->hitbox, current->obstacle.data.pigeonPoop.hitbox)) {
                    // Fezes causam lentidão (não reduz vida)
                    if (player->hasUmbrella == 0) {
                        player->speed = 50.0f;  // Lentidão
                    }
                    hasCollided = 1;
                }
                break;
                
            case OBSTACLE_BUS:
                if (CheckCollisionRecs(player->hitbox, current->obstacle.data.bus.hitbox)) {
                    hasCollided = 1;
                    player->lives -= 1;
                }
                break;
                
            default:
                break;
        }
        
        if (hasCollided) {
            current->obstacle.active = 0;
            current->obstacle.position.x = -200.0f;
        }
        
        current = current->next;
    }

    for (int i = 0; i < 2; i++) {
        if (stage->umbrellas[i].isActive && !stage->umbrellas[i].isCollected) {
            // Verificar colisão com player
            if (CheckCollisionRecs(player->hitbox, stage->umbrellas[i].hitbox)) {
                // Player coleta!
                stage->umbrellas[i].isCollected = 1;
                stage->umbrellas[i].isActive = 0;
                
                // Ativar proteção no player
                player->hasUmbrella = 1;
                player->umbrellaTimer = 5.0f;  // 5 segundos
            }
        }
    }

    if (player->hasUmbrella) {
        player->umbrellaTimer -= deltaTime;
        
        if (player->umbrellaTimer <= 0.0f) {
            player->hasUmbrella = 0;
            player->umbrellaTimer = 0.0f;
        }
    }

    if (stage->distanceTraveled >= 8000.0f) {
        stage->stage1Complete = 1;
    }
}



void drawStage1(Stage1 *stage) {
    if (stage->background.id > 0) {
        DrawTextureEx(stage->background, (Vector2){0, 0}, 0.0f, 1.0f, WHITE);
    }
    
    for (int i = 0; i < 8; i++) {
        if (stage->buildings[i].id > 0) {
            Vector2 position = {stage->roadPosition + i * 200, GetScreenHeight() - stage->buildings[i].height};
            DrawTextureEx(stage->buildings[i], position, 0.0f, 1.0f, WHITE);
        }
    }

    for (int i = 0; i < MAX_RAINDROPS; i++) {
        if (stage->raindrops[i].isActive) {
            drawRaindrop(stage->raindrops[i]);
        }
    }

    for (int i = 0; i < MAX_PIGEONS; i++) {
        if (stage->pigeons[i].isActive) {
            drawPigeon(stage->pigeons[i]);
        }
    }

    ObstacleNode *current = stage->obstacleQueue.front;
    while (current != NULL) {
        if (current->obstacle.active) {
            drawObstacle(current->obstacle, (Texture2D){0}, (Texture2D){0});
        }
        current = current->next;
    }

    for (int i = 0; i < 2; i++) {
        if (stage->umbrellas[i].isActive) {
            drawUmbrella(stage->umbrellas[i]);
        }
    }

    char hud[256];
    
    // Vidas
    sprintf(hud, "Vidas: %d", player->lives);
    DrawText(hud, 10, 10, 16, BLACK);
    
    // Score
    sprintf(hud, "Pontos: %.0f", player->score);
    DrawText(hud, 10, 35, 16, BLACK);
    
    // Tempo
    sprintf(hud, "Tempo: %.1f s", stage->distanceTraveled / stage->scrollSpeed);
    DrawText(hud, 10, 60, 16, BLACK);
    
    // Proteção do guarda-chuva
    if (player->hasUmbrella > 0) {
        sprintf(hud, "Protecao: %.1f s", player->umbrellaTimer);
        DrawText(hud, GetScreenWidth() - 200, 10, 16, GREEN);
    }
    
    // Progresso
    sprintf(hud, "Progresso: %.0f / 8000", stage->distanceTraveled);
    DrawText(hud, GetScreenWidth() - 300, 35, 16, BLACK);
    
    // Barra de progresso
    float progressPercent = stage->distanceTraveled / 8000.0f;
    if (progressPercent > 1.0f) progressPercent = 1.0f;
    
    DrawRectangle(GetScreenWidth() - 300, 55, 260, 20, LIGHTGRAY);
    DrawRectangle(GetScreenWidth() - 300, 55, (int)(260 * progressPercent), 20, GREEN);
    DrawRectangleLinesEx((Rectangle){GetScreenWidth() - 300, 55, 260, 20}, 2, BLACK);
    
    // Dificuldade
    sprintf(hud, "Dificuldade: x%.1f", stage->difficultyMultiplier);
    DrawText(hud, GetScreenWidth() - 300, 80, 14, BLACK);
}

void unloadStage1(Stage1* stage) {
    if (stage->background.id > 0) {
        UnloadTexture(stage->background);
    }
    for (int i = 0; i < 5; i++) {
        if (stage->buildings[i].id > 0) {
            UnloadTexture(stage->buildings[i]);
        }
    }
    freeObstacleQueue(&stage->obstacleQueue);
}