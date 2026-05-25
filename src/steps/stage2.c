#include "stage2.h"
#include "../utils/gameConstants.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Texturas dos obstáculos
static Texture2D texturaCaranguejo;
static Texture2D texturaBuraco;

// Texturas de animação do personagem
static Texture2D txMoverDireita;
static Texture2D txMoverEsquerda;
static Texture2D txPuloDireita;
static Texture2D txPuloEsquerda;

// NOVAS TEXTURAS PARA O CENÁRIO SEPARADO
static Texture2D plataformaAreia;
static int bgLoaded = 0;
static int platformLoaded = 0;

// Variável estática para lembrar para onde o jogador olhou por último (1 = Direita, 0 = Esquerda)
static int olhandoParaDireita = 1;

#define S2_AREIA_Y      975.0f  
#define S2_GRAVIDADE   3200.0f
#define S2_FORCA_PULO -1220.0f  

static void spawnSandObstacle(Stage2 *stage) {
    int roll = rand() % 100;
    Stage2ObstacleType type = (roll < 50) ? S2_OBS_CRAB : S2_OBS_TRASH;

    Vector2 pos = { (float)GetRenderWidth() + 100.0f, S2_AREIA_Y };

    Stage2Obstacle obs = createStage2Obstacle(pos, type);

    if (type == S2_OBS_CRAB) {
        obs.hitbox.width = 80.0f;          
        obs.hitbox.height = 35.0f;         
        obs.position.y = S2_AREIA_Y - 60.0f; 
    } else if (type == S2_OBS_TRASH) {
        obs.hitbox.width = 90.0f;          
        obs.hitbox.height = 15.0f;         
        obs.position.y = S2_AREIA_Y - 30.0f; 
    }

    obs.hitbox.x = obs.position.x + 10.0f; 
    obs.hitbox.y = obs.position.y + (type == S2_OBS_CRAB ? 25.0f : 15.0f);

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
                printf("[COLISÃO] Caranguejo detectado de verdade!\n");
            }
            else if (o->type == S2_OBS_TRASH) {
                stage->breath -= 30.0f; 
                if (stage->breath <= 0.0f) {
                    stage->breath = 0.0f;
                    player->lives = 0;
                }
                o->active = 0;
                o->position.x = -300.0f;
                printf("[COLISÃO] Buraco detectado de verdade!\n");
            }
            fflush(stdout);
        }
        cur = cur->next;
    }
}

static void scrollAndCleanObstacles(Stage2 *stage, Player *player, float deltaTime) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        o->position.x -= stage->scrollSpeed * deltaTime;
        o->hitbox.x = o->position.x + 10.0f;

        cur = cur->next;
    }

    int sizeBefore = stage2QueueSize(&stage->obstacleQueue);
    removeOffscreenStage2(&stage->obstacleQueue, -200.0f);
    int sizeAfter = stage2QueueSize(&stage->obstacleQueue);
    int passed = sizeBefore - sizeAfter;
    if (passed > 0) {
        player->score += 10.0f * passed * stage->difficultyMultiplier;
    }
}

void initStage2(Stage2 *stage) {
    stage->mode = STAGE2_MODE_SAND;
    stage->modeTimer = 0.0f;
    stage->scrollSpeed = 380.0f; 
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

    stage->breath = STAGE2_BREATH_MAX;
    stage->lightningTimer = 5.0f;
    stage->lightningFlash = 0.0f;
    stage->stormActive = 0;
    stage->currentPushY = 0.0f;

    bgLoaded = 0;
    stage->bgSand = LoadTexture("assets/img/landscapeLevel2.png");
    if (stage->bgSand.id != 0) bgLoaded = 1;

    platformLoaded = 0;
    plataformaAreia = LoadTexture("assets/img/plataformLevel2.png");
    if (plataformaAreia.id != 0) platformLoaded = 1;

    stage->bgSea  = (Texture2D){0};

    texturaCaranguejo = LoadTexture("assets/img/crab1.png");
    texturaBuraco     = LoadTexture("assets/img/hole.png");
    
    txMoverDireita  = LoadTexture("assets/img/characterMovingR1.png");
    txMoverEsquerda = LoadTexture("assets/img/characterMovingL1.png");
    txPuloDireita   = LoadTexture("assets/img/CharacterJumpingR.png");
    txPuloEsquerda  = LoadTexture("assets/img/CharacterJumpingL.png");

    olhandoParaDireita = 1; 

    if (!bgLoaded || !platformLoaded || texturaCaranguejo.id == 0 || texturaBuraco.id == 0) {
        printf("[AVISO] Erro crítico: Falha ao carregar texturas de cenário ou obstáculos na Stage 2!\n");
    }
}

void unloadStage2(Stage2 *stage) {
    if (bgLoaded) {
        UnloadTexture(stage->bgSand);
        bgLoaded = 0;
    }
    if (platformLoaded) {
        UnloadTexture(plataformaAreia);
        platformLoaded = 0;
    }
    if (stage->bgSea.id  > 0) UnloadTexture(stage->bgSea);
    
    UnloadTexture(texturaCaranguejo);
    UnloadTexture(texturaBuraco);
    UnloadTexture(txMoverDireita);
    UnloadTexture(txMoverEsquerda);
    UnloadTexture(txPuloDireita);
    UnloadTexture(txPuloEsquerda);
    
    freeStage2Queue(&stage->obstacleQueue);
}

static void updateSand(Stage2 *stage, Player *player, float deltaTime) {
    if (player->lives <= 0) {
        if (IsKeyPressed(KEY_ENTER)) {
            player->lives = 3;
            player->score = 0.0f;
            stage->breath = 100.0f;
            stage->distanceTraveled = 0.0f;
            stage->obstacleSpawnTimer = 0.0f;
            stage->backgroundScroll = 0.0f; 
            freeStage2Queue(&stage->obstacleQueue);
            initStage2Queue(&stage->obstacleQueue);
            olhandoParaDireita = 1;
            printf("[STAGE 2] Reset interno executado!\n");
        }
        return; 
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        olhandoParaDireita = 1;
    } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        olhandoParaDireita = 0;
    }

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

    float velocidadAtual = 450.0f * fatorDificuldade; 
    stage->spawnInterval = 1.3f / fatorDificuldade;

    stage->backgroundScroll += velocidadAtual * deltaTime;

    player->width = 140.0f;
    player->height = 175.0f;

    float limiteChao = S2_AREIA_Y - player->height;

    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && player->position.y >= limiteChao - 5.0f) {
        player->velocity.y = S2_FORCA_PULO;
    }

    player->velocity.y += S2_GRAVIDADE * deltaTime;
    player->position.y += player->velocity.y * deltaTime;

    if (player->position.y >= limiteChao) {
        player->position.y = limiteChao;
        player->velocity.y = 0.0f;
    }

    player->position.x = 300.0f;

    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        spawnSandObstacle(stage);
        stage->obstacleSpawnTimer = 0.0f;
    }

    float velocidadeAntiga = stage->scrollSpeed;
    stage->scrollSpeed = velocidadAtual;
    scrollAndCleanObstacles(stage, player, deltaTime);
    stage->scrollSpeed = velocidadeAntiga;

    player->hitbox.width = player->width - 40.0f; 
    player->hitbox.height = player->height - 10.0f;
    player->hitbox.x = player->position.x + 20.0f; 
    player->hitbox.y = player->position.y;

    handleSandCollisions(stage, player);
}

static void updateTransition(Stage2 *stage, Player *player, float deltaTime) {
    (void)player;
    stage->modeTimer += deltaTime;
    if (stage->modeTimer >= STAGE2_TRANSITION_TIME) {
        stage->mode = STAGE2_MODE_SEA;
        stage->modeTimer = 0.0f;
    }
}

static void updateSea(Stage2 *stage, Player *player, float deltaTime) {
    (void)stage;
    (void)player;
    (void)deltaTime;
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
        Rectangle dest = { obs.position.x, obs.position.y, 100.0f, 60.0f };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(texturaCaranguejo, source, dest, origin, 0.0f, WHITE);
    } else {
        DrawRectangleRec(obs.hitbox, RED);
    }
}

static void drawHoleObstacle(Stage2Obstacle obs) {
    if (texturaBuraco.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)texturaBuraco.width, (float)texturaBuraco.height };
        Rectangle dest = { obs.position.x, obs.position.y, 110.0f, 30.0f };
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
    } else if (obs.type == S2_OBS_TRASH) {
        drawHoleObstacle(obs);
    }
}

static void drawSand(Stage2 *stage) {
    int screenWidth = GetRenderWidth();
    int screenHeight = GetRenderHeight();

    // =========================================================
    // CAMADA 1 — BACKGROUND (CÉU / PARTE DISTANTE) LENTO
    // =========================================================
    if (bgLoaded && stage->bgSand.id > 0) {
        float bgScroll = fmod(stage->backgroundScroll * 0.25f, screenWidth);
        Rectangle source = { 0.0f, 0.0f, (float)stage->bgSand.width, (float)stage->bgSand.height };

        for (int i = 0; i < 2; i++) {
            Rectangle dest = { (i * screenWidth) - bgScroll, 0.0f, (float)screenWidth, (float)screenHeight };
            DrawTexturePro(stage->bgSand, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }

    // =========================================================
    // CAMADA 2 — PLATAFORMA (ONDE O BONECO PISA) RÁPIDA
    // CORREÇÃO: Alinhando a areia na parte de baixo da tela
    // =========================================================
    if (platformLoaded && plataformaAreia.id > 0) {
        float platformWidth = (float)screenWidth; 
        float platformScroll = fmod(stage->backgroundScroll, platformWidth);
        Rectangle source = { 0.0f, 0.0f, (float)plataformaAreia.width, (float)plataformaAreia.height };

        for (int i = 0; i < 2; i++) {
            // Posicionado na parte inferior cobrindo a pista
            Rectangle dest = { 
                (i * platformWidth) - platformScroll, 
                0.0f, 
                platformWidth, 
                (float)screenHeight 
            };
            DrawTexturePro(plataformaAreia, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        }
    } else {
        DrawRectangle(0, S2_AREIA_Y, screenWidth, screenHeight - S2_AREIA_Y, DARKBROWN);
    }

    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle);
        cur = cur->next;
    }

    DrawText("VIDA:", 15, 120, 20, DARKGRAY);
    DrawRectangle(85, 120, 200, 20, (Color){60, 60, 60, 200});
    float pct = stage->breath / 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    Color corBarra = (pct > 0.4f) ? GREEN : RED;
    DrawRectangle(85, 120, (int)(200 * pct), 20, corBarra);
    DrawRectangleLines(85, 120, 200, 20, BLACK);
}

static void drawTransition(Stage2 *stage) {
    (void)stage;
    DrawRectangle(0, 0, (float)GetRenderWidth(), (float)GetRenderHeight(), DARKBLUE);
    const char *msg = "Mergulhando no mar...";
    int w = MeasureText(msg, 50);
    DrawText(msg, ((float)GetRenderWidth() - w) / 2, (float)GetRenderHeight() / 2, 50, WHITE);
}

static void drawSea(Stage2 *stage, Player *player) {
    (void)player;
    DrawRectangleGradientV(0, 0, (float)GetRenderWidth(), (float)GetRenderHeight(), (Color){30, 100, 180, 255}, (Color){5, 30, 90, 255});

    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle);
        cur = cur->next;
    }

    DrawText("FOLEGO:", 15, 120, 20, WHITE);
    DrawRectangle(115, 120, 200, 20, (Color){60, 60, 60, 200});
    float pct = stage->breath / 100.0f;
    DrawRectangle(115, 120, (int)(200 * pct), 20, SKYBLUE);
    DrawRectangleLines(115, 120, 200, 20, WHITE);
}

void drawStage2(Stage2 *stage, Player *player) {
    switch (stage->mode) {
        case STAGE2_MODE_SAND:       drawSand(stage); break;
        case STAGE2_MODE_TRANSITION: drawTransition(stage); break;
        case STAGE2_MODE_SEA:        drawSea(stage, player); break;
        case STAGE2_MODE_FINISHED:   break;
    }

    if (stage->mode == STAGE2_MODE_SAND) {
        Texture2D texturaAtual = txMoverDireita; 
        float limiteChao = S2_AREIA_Y - player->height;
        int noAr = (player->position.y < limiteChao - 5.0f);

        if (noAr) {
            texturaAtual = olhandoParaDireita ? txPuloDireita : txPuloEsquerda;
        } else {
            texturaAtual = olhandoParaDireita ? txMoverDireita : txMoverEsquerda;
        }

        if (texturaAtual.id > 0) {
            Rectangle source = { 0.0f, 0.0f, (float)texturaAtual.width, (float)texturaAtual.height };
            Rectangle dest = {
                player->position.x,
                player->position.y, 
                player->width,
                player->height
            };
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(texturaAtual, source, dest, origin, 0.0f, WHITE);
        } else {
            DrawRectangleRec(player->hitbox, BLUE);
        }
    }
}