#include "stage2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../utils/utils.h"

#define S2_CHAO_Y 395.0f          
#define S2_GRAVIDADE 1300.0f      
#define S2_FORCA_PULO -520.0f     

static Texture2D texturaPersonagem;
static Texture2D texturaCaranguejo; 
static Texture2D texturaBuraco;      

static void spawnSandObstacle(Stage2 *stage) {
    int roll = rand() % 100;
    Stage2ObstacleType type = (roll < 50) ? S2_OBS_CRAB : S2_OBS_TRASH;

    Vector2 pos = { 840.0f, S2_CHAO_Y }; 

    Stage2Obstacle obs = createStage2Obstacle(pos, type);
    
    if (type == S2_OBS_CRAB) {
        obs.hitbox.width = 30.0f;          
        obs.hitbox.height = 20.0f;         
        obs.position.y = S2_CHAO_Y - 20.0f; 
    } else if (type == S2_OBS_TRASH) { 
        obs.hitbox.width = 35.0f;          
        obs.hitbox.height = 10.0f;         
        obs.position.y = S2_CHAO_Y - 5.0f;  
    }
    
    obs.hitbox.x = obs.position.x;
    obs.hitbox.y = obs.position.y;

    enqueueStage2(&stage->obstacleQueue, obs);
}

static void spawnSeaObstacle(Stage2 *stage) {
    int roll = rand() % 100;
    Stage2ObstacleType type = (roll < 50) ? S2_OBS_JELLYFISH : S2_OBS_CURRENT;
    Vector2 pos = { 840.0f, 120.0f + (rand() % 200) };

    Stage2Obstacle obs = createStage2Obstacle(pos, type);
    enqueueStage2(&stage->obstacleQueue, obs);
}

static void handleSandCollisions(Stage2 *stage, Player *player) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        
        if (o->active && CheckCollisionRecs(player->hitbox, o->hitbox)) {
            if (o->type == S2_OBS_CRAB) {
                stage->breath = 0.0f; 
                player->lives = 0;
                o->active = 0;
                o->position.x = -300.0f;
                printf("[COLISÃO] Caranguejo detetado! Morte imediata.\n");
            } 
            else if (o->type == S2_OBS_TRASH) {
                stage->breath -= 30.0f; 
                if (stage->breath <= 0.0f) {
                    stage->breath = 0.0f;
                    player->lives = 0;
                }
                o->active = 0;
                o->position.x = -300.0f;
                printf("[COLISÃO] Caiu num buraco! -30 de Vida.\n");
            }
            fflush(stdout);
        }
        cur = cur->next;
    }
}

static void handleSeaCollisions(Stage2 *stage, Player *player) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        if (o->active && CheckCollisionRecs(player->hitbox, o->hitbox)) {
            if (o->type == S2_OBS_JELLYFISH) {
                stage->breath -= 25.0f;
                if (stage->breath <= 0.0f) player->lives = 0;
                o->active = 0;
                o->position.x = -300.0f;
            }
        }
        cur = cur->next;
    }
}

static void scrollAndCleanObstacles(Stage2 *stage, Player *player, float deltaTime) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        o->position.x -= stage->scrollSpeed * deltaTime;
        o->hitbox.x = o->position.x;
        cur = cur->next;
    }

    int sizeBefore = stage2QueueSize(&stage->obstacleQueue);
    removeOffscreenStage2(&stage->obstacleQueue, -200.0f);
    int sizeAfter = stage2QueueSize(&stage->obstacleQueue);
    
    if (sizeBefore - sizeAfter > 0) {
        player->score += 10.0f;
    }
}

void initStage2(Stage2 *stage) {
    stage->mode = STAGE2_MODE_SAND;
    stage->modeTimer = 0.0f;
    stage->scrollSpeed = 260.0f; 
    stage->distanceTraveled = 0.0f;
    stage->spawnInterval = 1.4f;  
    stage->obstacleSpawnTimer = 0.0f;
    stage->difficultyMultiplier = 1.0f;
    stage->stage2Complete = 0;
    stage->backgroundScroll = 0.0f;

    initStage2Queue(&stage->obstacleQueue);

    stage->hasCoconutBuff = 0;
    stage->coconutBuffTimer = 0.0f;
    stage->sharkActive = 0;
    stage->breath = 100.0f; 
    
    stage->lightningTimer = 5.0f;
    stage->lightningFlash = 0.0f;
    stage->stormActive = 0;
    stage->currentPushY = 0.0f;

    stage->bgSand = LoadTexture("assets/img/landscapeLevel2.png");
    stage->bgSea  = (Texture2D){0};

    texturaPersonagem = LoadTexture("assets/img/characterMovingR1.png");
    texturaCaranguejo = LoadTexture("assets/img/crab1.png");
    texturaBuraco = LoadTexture("assets/img/hole.png");

    if (stage->bgSand.id == 0 || texturaCaranguejo.id == 0 || texturaBuraco.id == 0 || texturaPersonagem.id == 0) {
        printf("[AVISO] Erro crítico: Falha ao carregar uma ou mais texturas na Fase 2!\n");
    }
}

void unloadStage2(Stage2 *stage) {
    freeStage2Queue(&stage->obstacleQueue);
    UnloadTexture(stage->bgSand);
    UnloadTexture(texturaPersonagem);
    UnloadTexture(texturaCaranguejo);
    UnloadTexture(texturaBuraco); 
}

static void updateSand(Stage2 *stage, Player *player, float deltaTime) {
    float fatorDificuldade = 1.0f + (stage->distanceTraveled / 300.0f) * 0.15f;
    if (fatorDificuldade > 2.0f) fatorDificuldade = 2.0f;

    stage->distanceTraveled += 25.0f * deltaTime; 

    if (stage->distanceTraveled >= 1500.0f) {
        stage->distanceTraveled = 1500.0f;
        stage->mode = STAGE2_MODE_TRANSITION;
        stage->modeTimer = 0.0f;
        stage->breath = 100.0f; 
        freeStage2Queue(&stage->obstacleQueue);
        initStage2Queue(&stage->obstacleQueue);
        return; 
    }

    float velocidadeAtual = 260.0f * fatorDificuldade;
    stage->spawnInterval = 1.4f / fatorDificuldade;

    if (stage->breath <= 0.0f) {
        player->lives = 0;
    }

    player->width = 30.0f;
    player->height = 40.0f;

    float limiteChao = S2_CHAO_Y - player->height;

    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && player->position.y >= limiteChao - 5.0f) {
        player->velocity.y = S2_FORCA_PULO; 
    }

    player->velocity.y += S2_GRAVIDADE * deltaTime;
    player->position.y += player->velocity.y * deltaTime;

    if (player->position.y >= limiteChao) {
        player->position.y = limiteChao;
        player->velocity.y = 0.0f;
    }

    player->position.x = 150.0f;

    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        spawnSandObstacle(stage);
        stage->obstacleSpawnTimer = 0.0f;
    }

    float velocidadeAntiga = stage->scrollSpeed;
    stage->scrollSpeed = velocidadeAtual; 
    scrollAndCleanObstacles(stage, player, deltaTime);
    stage->scrollSpeed = velocidadeAntiga;

    handleSandCollisions(stage, player);
}

static void updateTransition(Stage2 *stage, Player *player, float deltaTime) {
    (void)player;
    stage->modeTimer += deltaTime;
    if (stage->modeTimer >= STAGE2_TRANSITION_TIME) {
        stage->mode = STAGE2_MODE_SEA;
        stage->modeTimer = 0.0f;
        stage->distanceTraveled = 0.0f;
        stage->scrollSpeed = 160.0f;
    }
}

static void updateSea(Stage2 *stage, Player *player, float deltaTime) {
    stage->distanceTraveled += stage->scrollSpeed * deltaTime;

    player->position.x = 150.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) player->position.y -= 220.0f * deltaTime;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) player->position.y += 220.0f * deltaTime;

    if (player->position.y < 50.0f)  player->position.y = 50.0f;
    if (player->position.y > 400.0f) player->position.y = 400.0f;

    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        spawnSeaObstacle(stage);
        stage->obstacleSpawnTimer = 0.0f;
    }

    scrollAndCleanObstacles(stage, player, deltaTime);
    handleSeaCollisions(stage, player);

    if (stage->distanceTraveled >= STAGE2_SEA_DISTANCE) {
        stage->stage2Complete = 1;
        stage->mode = STAGE2_MODE_FINISHED;
    }
}

void updateStage2(Stage2 *stage, Player *player, float deltaTime) {
    switch (stage->mode) {
        case STAGE2_MODE_SAND:       updateSand(stage, player, deltaTime); break;
        case STAGE2_MODE_TRANSITION: updateTransition(stage, player, deltaTime); break;
        case STAGE2_MODE_SEA:        updateSea(stage, player, deltaTime); break;
        case STAGE2_MODE_FINISHED:   break;
    }
}

static void drawCrabObstacle(Stage2Obstacle obs) {
    if (texturaCaranguejo.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)texturaCaranguejo.width, (float)texturaCaranguejo.height };
        Rectangle dest = { obs.position.x - 2.0f, obs.position.y - 2.0f, 34.0f, 24.0f };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(texturaCaranguejo, source, dest, origin, 0.0f, WHITE);
    } else {
        DrawRectangleRec(obs.hitbox, RED);
    }
}

static void drawHoleObstacle(Stage2Obstacle obs) {
    if (texturaBuraco.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)texturaBuraco.width, (float)texturaBuraco.height };
        Rectangle dest = { obs.position.x - 2.5f, obs.position.y - 3.0f, 40.0f, 16.0f };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(texturaBuraco, source, dest, origin, 0.0f, WHITE);
    } else {
        DrawEllipse(obs.position.x + (obs.hitbox.width / 2.0f), obs.position.y + (obs.hitbox.height / 2.0f), 
                    obs.hitbox.width / 2.0f, obs.hitbox.height / 2.0f, BLACK);
    }
}

static void drawStage2Obstacle(Stage2Obstacle obs) {
    if (!obs.active) return;

    if (obs.type == S2_OBS_CRAB) {
        drawCrabObstacle(obs);
    } 
    else if (obs.type == S2_OBS_TRASH) {
        drawHoleObstacle(obs);
    } 
    else {
        DrawRectangleRec(obs.hitbox, PURPLE);
    }
}

static void drawSand(Stage2 *stage) {
    if (stage->bgSand.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)stage->bgSand.width, (float)stage->bgSand.height };
        Rectangle dest = { 0.0f, 0.0f, 800.0f, 450.0f }; 
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(stage->bgSand, source, dest, origin, 0.0f, WHITE);
    }

    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle);
        cur = cur->next;
    }

    DrawText("VIDA:", 15, 75, 14, DARKGRAY);
    DrawRectangle(65, 75, 150, 14, (Color){60, 60, 60, 200}); 
    float pct = stage->breath / 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    Color corBarra = (pct > 0.4f) ? GREEN : RED; 
    DrawRectangle(65, 75, (int)(150 * pct), 14, corBarra);
    DrawRectangleLines(65, 75, 150, 14, BLACK); 

    char distBuf[64];
    sprintf(distBuf, "Progresso Areia: %.0f / 1500m", stage->distanceTraveled);
    DrawText(distBuf, 15, 15, 18, DARKGRAY);
}

static void drawTransition(Stage2 *stage) {
    (void)stage;
    DrawRectangle(0, 0, 800, 450, DARKBLUE);
    const char *msg = "Mergulhando no mar...";
    int w = MeasureText(msg, 30);
    DrawText(msg, (800 - w) / 2, 210, 30, WHITE);
}

static void drawSea(Stage2 *stage, Player *player) {
    (void)player;
    DrawRectangleGradientV(0, 0, 800, 450, (Color){30, 100, 180, 255}, (Color){5, 30, 90, 255});

    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle);
        cur = cur->next;
    }

    DrawText("FOLEGO:", 15, 75, 14, WHITE);
    DrawRectangle(85, 75, 150, 14, (Color){60, 60, 60, 200});
    float pct = stage->breath / 100.0f;
    DrawRectangle(85, 75, (int)(150 * pct), 14, SKYBLUE);
    DrawRectangleLines(85, 75, 150, 14, WHITE);
}

static void drawFinished(Stage2 *stage) {
    (void)stage;
    DrawRectangle(0, 0, 800, 450, BLACK);
    DrawText("FASE 2 COMPLETA!", 240, 200, 35, GOLD);
}

void drawStage2(Stage2 *stage, Player *player) {
    switch (stage->mode) {
        case STAGE2_MODE_SAND:       drawSand(stage); break;
        case STAGE2_MODE_TRANSITION: drawTransition(stage); break;
        case STAGE2_MODE_SEA:        drawSea(stage, player); break;
        case STAGE2_MODE_FINISHED:   break;
    }

    player->width = 30.0f;
    player->height = 40.0f;
    player->hitbox.width = player->width;
    player->hitbox.height = player->height;
    player->hitbox.x = player->position.x;
    player->hitbox.y = player->position.y;

    if (texturaPersonagem.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)texturaPersonagem.width, (float)texturaPersonagem.height };
        Rectangle dest = { 
            player->position.x - 15.0f, 
            player->position.y - 40.0f, 
            60.0f, 
            80.0f 
        };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(texturaPersonagem, source, dest, origin, 0.0f, WHITE);
    } else {
        DrawRectangleRec(player->hitbox, BLUE); 
    }
}