#include "stage2.h"
#include "../utils/gameConstants.h"
#include "../entities/crab.h" 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Protótipos antecipados para o GCC aceitar a ordem de chamada das funções
static void drawStage2Obstacle(Stage2Obstacle obs, Stage2 *stage);
static void drawTransition(Stage2 *stage);
static void updateSand(Stage2 *stage, Player *player, float deltaTime);
static void updateTransition(Stage2 *stage, Player *player, float deltaTime);
static void updateSea(Stage2 *stage, Player *player, float deltaTime);
static void handleBreathRecovery(Stage2 *stage, Player *player, float deltaTime);
static void updateAndDrawBubbles(float deltaTime);

// =========================================================================
// GESTÃO DE COMPATIBILIDADE DE ENUMS DO MAR
// =========================================================================
#ifndef S2_OBS_SHARK
  #define S2_OBS_SHARK 2
#endif
#ifndef S2_OBS_JELLYFISH
  #define S2_OBS_JELLYFISH 3
#endif
#ifndef S2_OBS_NET
  #define S2_OBS_NET 4
#endif

// Struct interna para gerenciar as partículas de bolhas
typedef struct {
    Vector2 position;
    float speed;
    float scale;
    float wobbleSpeed;
    float wobbleRange;
    float alpha;
    int active;
} BubbleParticle;

#define MAX_BUBBLES 30
static BubbleParticle bubbles[MAX_BUBBLES];

// Array de texturas do caranguejo (Frames separados)
static Texture2D crabTextures[2];
static Texture2D texturaBuraco;
static Texture2D texturaSacola; 
static Texture2D texturaBolhas; 

// Texturas dos obstáculos (Modo Mar)
static Texture2D txSharkR1;
static Texture2D txSharkR2;
static Texture2D txSharkL1;
static Texture2D txSharkL2;
static Texture2D txJelly1;
static Texture2D txJelly2;
static Texture2D txFishingNet; 

// Texturas de animação do personagem (Modo Areia)
static Texture2D txMoverDireita;
static Texture2D txMoverEsquerda;
static Texture2D txPuloDireita;
static Texture2D txPuloEsquerda;
static Texture2D txParadoDireita; 
static Texture2D txParadoEsquerda; 

// Texturas de animação do personagem (Modo Mar - Mergulho)
static Texture2D txNadarDireitaAtivo;   
static Texture2D txNadarDireitaParado;  
static Texture2D txNadarEsquerdaAtivo;  
static Texture2D txNadarEsquerdaParado; 

// Texturas de personagem cansado (Fôlego Crítico)
static Texture2D txCansado1;
static Texture2D txCansado2;

// Texturas do personagem preso na rede (Tangled)
static Texture2D txPresoDireitaParado;
static Texture2D txPresoDireitaMovendo;
static Texture2D txPresoEsquerdaParado;
static Texture2D txPresoEsquerdaMovendo;

// Texturas estáticas para a transição cinematográfica
static Texture2D txWaveBig;
static Texture2D txWaveSmall;
static Texture2D txSharkSign;
static Texture2D bgSuperficieMar; 

// NOVAS TEXTURAS PARA O CENÁRIO SEPARADO
static Texture2D bgOceano; 
static int bgLoaded = 0;
static int bgOceanLoaded = 0;
static int bgSurfaceLoaded = 0; 

// Variável estática para controlar a barra de vida localmente na Fase 2
int playerHealth = 100;

// Variável estática para lembrar a orientação do jogador (1 = Direita, 0 = Esquerda)
static int olhandoParaDireita = 1;

// Cronômetro estático interno para controlar a duração do efeito de lentidão da rede
static float netDebuffTimer = 0.0f;

#define S2_AREIA_Y      975.0f  
#define S2_GRAVIDADE   3200.0f
#define S2_FORCA_PULO -1220.0f  

// Constante de velocidade de nado livre no mar
#define S2_VELOCIDADE_NADO 400.0f

// =========================================================================
// REQUISITO 4: ALGORITMO DE ORDENAÇÃO (INSERTION SORT)
// =========================================================================
static void sortStage2Obstacles(Stage2Queue *queue) {
    if (queue->front == NULL || queue->front->next == NULL) return;

    Stage2Node *sorted = NULL;
    Stage2Node *current = queue->front;

    while (current != NULL) {
        Stage2Node *next = current->next;

        if (sorted == NULL || sorted->obstacle.position.x >= current->obstacle.position.x) {
            current->next = sorted;
            sorted = current;
        } else {
            Stage2Node *temp = sorted;
            while (temp->next != NULL && temp->next->obstacle.position.x < current->obstacle.position.x) {
                temp = temp->next;
            }
            current->next = temp->next;
            temp->next = current;
        }
        current = next;
    }
    queue->front = sorted;

    Stage2Node *temp = queue->front;
    while (temp != NULL && temp->next != NULL) {
        temp = temp->next;
    }
    queue->rear = temp;
}

// =========================================================================
// GESTÃO DE OBSTÁCULOS E SISTEMA FIFO
// =========================================================================

// Geração de Obstáculos na Areia
static void spawnSandObstacle(Stage2 *stage) {
    int roll = rand() % 100;
    int type = (roll < 50) ? S2_OBS_CRAB : S2_OBS_TRASH;

    Vector2 pos = { (float)GetRenderWidth() + 100.0f, S2_AREIA_Y };
    Stage2Obstacle obs = createStage2Obstacle(pos, type);
    obs.type = type; 

    if (type == S2_OBS_CRAB) {
        obs.hitbox.width = 90.0f;          
        obs.hitbox.height = 40.0f;          
        obs.position.y = S2_AREIA_Y - 75.0f; 
    } else if (type == S2_OBS_TRASH) {
        obs.hitbox.width = 100.0f;          
        obs.hitbox.height = 20.0f;          
        obs.position.y = S2_AREIA_Y - 40.0f; 
    }

    obs.hitbox.x = obs.position.x + 10.0f; 
    obs.hitbox.y = obs.position.y + (type == S2_OBS_CRAB ? 25.0f : 15.0f);

    enqueueStage2(&stage->obstacleQueue, obs); 
}

static void spawnSeaObstacle(Stage2 *stage) {
    int roll = rand() % 100;
    int type;
    
    if (roll < 25) type = S2_OBS_SHARK;
    else if (roll < 50) type = S2_OBS_JELLYFISH;
    else if (roll < 75) type = S2_OBS_NET;
    else type = S2_OBS_TRASH; 
    
    Vector2 pos = { 0 };
    int screenH = GetScreenHeight() > 0 ? GetScreenHeight() : 600;
    int veioDaEsquerda = 1;

    if (type == S2_OBS_SHARK) {
        veioDaEsquerda = (rand() % 2 == 0);
        if (veioDaEsquerda) {
            pos.x = -350.0f; 
        } else {
            pos.x = (float)GetRenderWidth() + 350.0f; 
        }
        float rawY = (float)(rand() % (screenH - 300) + 100);
        pos.y = veioDaEsquerda ? rawY : -rawY; 
    } else {
        pos.x = (float)GetRenderWidth() + 150.0f;
        pos.y = (float)(rand() % (screenH - 250) + 100);
    }

    Stage2Obstacle obs = createStage2Obstacle(pos, type);
    obs.type = type; 
    obs.active = 1;

    if (type == S2_OBS_SHARK) {
        obs.hitbox.width = 200.0f;   
        obs.hitbox.height = 70.0f;   
        obs.hitbox.x = obs.position.x + 60.0f;
        obs.hitbox.y = fabsf(obs.position.y) + 40.0f;
    } else if (type == S2_OBS_JELLYFISH) {
        obs.hitbox.width = 70.0f;    
        obs.hitbox.height = 130.0f;  
        obs.hitbox.x = obs.position.x + 35.0f;
        obs.hitbox.y = obs.position.y + 20.0f;
    } else if (type == S2_OBS_NET) {
        obs.hitbox.width = 110.0f;   
        obs.hitbox.height = 110.0f;  
        obs.hitbox.x = obs.position.x + 20.0f;
        obs.hitbox.y = obs.position.y + 20.0f;
    } else if (type == S2_OBS_TRASH) {
        obs.hitbox.width = 60.0f;
        obs.hitbox.height = 60.0f;
        obs.hitbox.x = obs.position.x + 10.0f;
        obs.hitbox.y = obs.position.y + 10.0f;
    }

    enqueueStage2(&stage->obstacleQueue, obs);
}

// Colisões e penalidades de vida/fôlego
static void handleCollisionsStage2(Stage2 *stage, Player *player) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;

        if (o->active && CheckCollisionRecs(player->hitbox, o->hitbox)) {
            if (o->type == S2_OBS_CRAB) {
                stage->breath = 0.0f;
                playerHealth = 0; 
                player->lives = 0; 
                o->active = 0;
            }
            else if (o->type == S2_OBS_TRASH) {
                if (stage->mode == STAGE2_MODE_SEA) {
                    stage->breath -= 30.0f; 
                    if (stage->breath <= 0.0f) {
                        stage->breath = 0.0f;
                        playerHealth = 0;
                        player->lives = 0;
                    }
                } else {
                    playerHealth -= 30; 
                    if (playerHealth <= 0) {
                        playerHealth = 0;
                        player->lives = 0;
                    }
                }
                o->active = 0;
            }
            else if (o->type == S2_OBS_SHARK) {
                player->lives = 0; 
                playerHealth = 0;
                stage->breath = 0.0f;
                o->active = 0;
            }
            else if (o->type == S2_OBS_JELLYFISH) {
                playerHealth -= 30; 
                if (playerHealth <= 0) {
                    playerHealth = 0;
                    player->lives = 0;
                }
                o->active = 0;
            }
            else if (o->type == S2_OBS_NET) {
                netDebuffTimer = 5.0f; 
                o->active = 0; 
            }
        }
        cur = cur->next;
    }
}

// Movimentação dos obstáculos ativos na tela
static void scrollAndCleanObstacles(Stage2 *stage, Player *player, float deltaTime) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        
        if (stage->mode == STAGE2_MODE_SEA) {
            if (o->type == S2_OBS_SHARK) {
                if (o->position.y >= 0) {
                    o->position.x += 600.0f * deltaTime; 
                } else {
                    o->position.x -= 600.0f * deltaTime; 
                }
            } else if (o->type == S2_OBS_JELLYFISH) {
                o->position.x -= 380.0f * deltaTime;
            } else if (o->type == S2_OBS_NET) {
                o->position.x -= 240.0f * deltaTime; 
            } else if (o->type == S2_OBS_TRASH) {
                o->position.x -= 300.0f * deltaTime; 
            }
        } else {
            o->position.x -= stage->scrollSpeed * deltaTime;
        }
        
        if (stage->mode == STAGE2_MODE_SEA) {
            if (o->type == S2_OBS_SHARK) {
                o->hitbox.x = o->position.x + 60.0f;
                o->hitbox.y = fabsf(o->position.y) + 40.0f;
            } else if (o->type == S2_OBS_JELLYFISH) {
                o->hitbox.x = o->position.x + 35.0f;
                o->hitbox.y = o->position.y + 20.0f;
            } else if (o->type == S2_OBS_NET) {
                o->hitbox.x = o->position.x + 20.0f;
                o->hitbox.y = o->position.y + 20.0f;
            } else if (o->type == S2_OBS_TRASH) {
                o->hitbox.x = o->position.x + 10.0f;
                o->hitbox.y = o->position.y + 10.0f;
            }
        } else {
            o->hitbox.x = o->position.x + 10.0f;
            o->hitbox.y = o->position.y + (o->type == S2_OBS_CRAB ? 25.0f : 15.0f);
        }

        cur = cur->next;
    }

    int sizeBefore = stage2QueueSize(&stage->obstacleQueue);               
    removeOffscreenStage2(&stage->obstacleQueue, -600.0f);                 
    int sizeAfter = stage2QueueSize(&stage->obstacleQueue);
    int passed = sizeBefore - sizeAfter;
    if (passed > 0) {
        player->score += 10.0f * passed * stage->difficultyMultiplier;
    }
}

// =========================================================================
// INICIALIZAÇÃO E CARREGAMENTO DE TEXTURAS
// =========================================================================
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

    netDebuffTimer = 0.0f; 
    playerHealth = 100;

    for (int i = 0; i < MAX_BUBBLES; i++) {
        bubbles[i].active = 0;
    }

    bgLoaded = 0;
    stage->bgSand = LoadTexture("assets/img/landscapeLevel2.png");
    if (stage->bgSand.id != 0) bgLoaded = 1;

    bgOceanLoaded = 0;
    bgOceano = LoadTexture("assets/img/landscapeOceanlevel2.png");
    if (bgOceano.id != 0) bgOceanLoaded = 1;

    stage->bgSea  = (Texture2D){0};

    crabTextures[0] = LoadTexture("assets/img/crab1.png");
    crabTextures[1] = LoadTexture("assets/img/crab2.png");
    texturaBuraco   = LoadTexture("assets/img/hole.png");
    texturaSacola   = LoadTexture("assets/img/PlasticBag.png"); 
    texturaBolhas   = LoadTexture("assets/img/Bubbles.png"); 
    
    txSharkR1 = LoadTexture("assets/img/sharkR1.png");
    txSharkR2 = LoadTexture("assets/img/sharkR2.png");
    txSharkL1 = LoadTexture("assets/img/sharkL1.png");
    txSharkL2 = LoadTexture("assets/img/sharkL2.png");
    txJelly1  = LoadTexture("assets/img/jellyfish1.png");
    txJelly2  = LoadTexture("assets/img/jellyfish2.png");
    txFishingNet = LoadTexture("assets/img/FishingNet.png"); 

    txMoverDireita  = LoadTexture("assets/img/characterMovingR1.png");
    txMoverEsquerda = LoadTexture("assets/img/characterMovingL1.png");
    txPuloDireita   = LoadTexture("assets/img/CharacterJumpingR.png");
    txPuloEsquerda  = LoadTexture("assets/img/CharacterJumpingL.png");

    txParadoDireita  = LoadTexture("assets/img/CharacterStandingR.png");
    txParadoEsquerda = LoadTexture("assets/img/CharacterStandingL.png");

    txNadarDireitaAtivo   = LoadTexture("assets/img/CharacterSwimmingR2.png");
    txNadarDireitaParado  = LoadTexture("assets/img/CharacterSwimmingR1.png");
    txNadarEsquerdaAtivo  = LoadTexture("assets/img/CharacterSwimmingL1.png");
    txNadarEsquerdaParado = LoadTexture("assets/img/CharacterSwimmingL2.png");

    txCansado1 = LoadTexture("assets/img/CharacterTired1.png");
    txCansado2 = LoadTexture("assets/img/CharacterTired2.png");

    txPresoEsquerdaParado  = LoadTexture("assets/img/CharacterTangledL1.png");
    txPresoEsquerdaMovendo = LoadTexture("assets/img/CharacterTangledL2.png");
    txPresoDireitaParado   = LoadTexture("assets/img/CharacterTangledR1.png");
    txPresoDireitaMovendo  = LoadTexture("assets/img/CharacterTangledR2.png");

    // Carregamento dos assets da Transição
    txWaveBig   = LoadTexture("assets/img/waveBig.png");
    txWaveSmall = LoadTexture("assets/img/waveSmall.png");
    txSharkSign = LoadTexture("assets/img/SharkSign.png");
    
    bgSurfaceLoaded = 0;
    bgSuperficieMar = LoadTexture("assets/img/landscapeOceanlevel2Surface (1).png");
    if (bgSuperficieMar.id != 0) bgSurfaceLoaded = 1;

    olhandoParaDireita = 1; 
}

void unloadStage2(Stage2 *stage) {
    if (bgLoaded) {
        UnloadTexture(stage->bgSand);
        bgLoaded = 0;
    }
    if (bgOceanLoaded) {
        UnloadTexture(bgOceano);
        bgOceanLoaded = 0;
    }
    if (stage->bgSea.id > 0) UnloadTexture(stage->bgSea);
    if (bgSurfaceLoaded) {
        UnloadTexture(bgSuperficieMar);
        bgSurfaceLoaded = 0;
    }
    
    UnloadTexture(crabTextures[0]);
    UnloadTexture(crabTextures[1]);
    UnloadTexture(texturaBuraco);
    UnloadTexture(texturaSacola);
    UnloadTexture(texturaBolhas);
    UnloadTexture(txMoverDireita);
    UnloadTexture(txMoverEsquerda);
    UnloadTexture(txPuloDireita);
    UnloadTexture(txPuloEsquerda);
    UnloadTexture(txParadoDireita);
    UnloadTexture(txParadoEsquerda);

    UnloadTexture(txNadarDireitaAtivo);
    UnloadTexture(txNadarDireitaParado);
    UnloadTexture(txNadarEsquerdaAtivo);
    UnloadTexture(txNadarEsquerdaParado);
    UnloadTexture(txSharkR1);
    UnloadTexture(txSharkR2);
    UnloadTexture(txSharkL1);
    UnloadTexture(txSharkL2);
    UnloadTexture(txJelly1);
    UnloadTexture(txJelly2);
    UnloadTexture(txFishingNet); 
    UnloadTexture(txCansado1);
    UnloadTexture(txCansado2);

    UnloadTexture(txPresoEsquerdaParado);
    UnloadTexture(txPresoEsquerdaMovendo);
    UnloadTexture(txPresoDireitaParado);
    UnloadTexture(txPresoDireitaMovendo);

    UnloadTexture(txWaveBig);
    UnloadTexture(txWaveSmall);
    UnloadTexture(txSharkSign);

    freeStage2Queue(&stage->obstacleQueue);
}

void updateStage2(Stage2 *stage, Player *player, float deltaTime) {
    switch (stage->mode) {
        case STAGE2_MODE_SAND:       updateSand(stage, player, deltaTime); break;
        case STAGE2_MODE_TRANSITION: updateTransition(stage, player, deltaTime); break;
        case STAGE2_MODE_SEA:        updateSea(stage, player, deltaTime); break;
        case STAGE2_MODE_FINISHED:   break;
    }
}

static void updateSand(Stage2 *stage, Player *player, float deltaTime) {
    if (player->lives <= 0 || playerHealth <= 0) {
        if (IsKeyPressed(KEY_ENTER)) {
            player->lives = 3;
            player->score = 0.0f;
            playerHealth = 100;
            stage->breath = 100.0f;
            stage->distanceTraveled = 0.0f;
            stage->obstacleSpawnTimer = 0.0f;
            stage->backgroundScroll = 0.0f; 
            freeStage2Queue(&stage->obstacleQueue);
            initStage2Queue(&stage->obstacleQueue);
            olhandoParaDireita = 1;
        }
        return; 
    }

    if (IsKeyPressed(KEY_R)) {
        stage->distanceTraveled = 1500.0f;
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) olhandoParaDireita = 1;
    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) olhandoParaDireita = 0;

    float fDificuldade = 1.0f + (stage->distanceTraveled / 300.0f) * 0.15f;
    if (fDificuldade > 2.0f) fDificuldade = 2.0f;

    stage->distanceTraveled += 25.0f * deltaTime;

    if (stage->distanceTraveled >= 1500.0f) {
        stage->distanceTraveled = 0.0f; 
        stage->mode = STAGE2_MODE_TRANSITION;
        stage->modeTimer = 0.0f;
        stage->breath = 100.0f;
        freeStage2Queue(&stage->obstacleQueue);
        initStage2Queue(&stage->obstacleQueue);
        
        player->position.x = 200.0f;
        player->position.y = (float)GetScreenHeight() * 0.5f; 
        return;
    }

    float vAtual = 450.0f * fDificuldade; 
    stage->spawnInterval = 1.3f / fDificuldade;
    stage->backgroundScroll += vAtual * deltaTime;

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

    sortStage2Obstacles(&stage->obstacleQueue);

    float vAntiga = stage->scrollSpeed;
    stage->scrollSpeed = vAtual;
    scrollAndCleanObstacles(stage, player, deltaTime);
    stage->scrollSpeed = vAntiga;

    player->hitbox.width = player->width - 40.0f; 
    player->hitbox.height = player->height - 10.0f;
    player->hitbox.x = player->position.x + 20.0f; 
    player->hitbox.y = player->position.y;

    handleCollisionsStage2(stage, player);
}

static void updateTransition(Stage2 *stage, Player *player, float deltaTime) {
    stage->modeTimer += deltaTime;

    player->width = 140.0f;
    player->height = 175.0f;

    float velocidadeNadoTransicao = 450.0f; 
    player->position.x += velocidadeNadoTransicao * deltaTime;

    // Alinha o boneco no terço inferior da tela, garantindo a sensação de submersão real
    float destinoY = (float)GetScreenHeight() * 0.82f - (player->height / 2.0f);
    player->position.y += (destinoY - player->position.y) * 5.0f * deltaTime;

    stage->backgroundScroll += 300.0f * deltaTime;

    if (player->position.x >= (float)GetScreenWidth()) {
        stage->mode = STAGE2_MODE_SEA;
        stage->modeTimer = 0.0f;
        stage->obstacleSpawnTimer = 0.0f;
        stage->spawnInterval = 1.2f; 
        
        player->position.x = 80.0f;
        player->position.y = (float)GetScreenHeight() * 0.5f;
    }
}

static void handleBreathRecovery(Stage2 *stage, Player *player, float deltaTime) {
    if (player->position.y <= 5.0f) {
        stage->breath += STAGE2_BREATH_MAX * deltaTime; 
        if (stage->breath > STAGE2_BREATH_MAX) {
            stage->breath = STAGE2_BREATH_MAX;
        }
    }
}

static void updateSea(Stage2 *stage, Player *player, float deltaTime) {
    if (player->lives <= 0 || playerHealth <= 0) {
        if (IsKeyPressed(KEY_ENTER)) {
            player->lives = 3;
            playerHealth = 100;
            stage->breath = 100.0f;
            stage->distanceTraveled = 0.0f;
            player->position.x = 200.0f;
            player->position.y = (float)GetScreenHeight() * 0.5f;
            freeStage2Queue(&stage->obstacleQueue);
            initStage2Queue(&stage->obstacleQueue);
            netDebuffTimer = 0.0f;
        }
        return;
    }

    float mVelocidade = 1.0f;
    if (netDebuffTimer > 0.0f) {
        netDebuffTimer -= deltaTime;
        mVelocidade = 0.5f; 
    }

    float velocidadAtualNado = S2_VELOCIDADE_NADO * mVelocidade;

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player->position.x += velocidadAtualNado * deltaTime;
        olhandoParaDireita = 1;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player->position.x -= velocidadAtualNado * deltaTime;
        olhandoParaDireita = 0;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        player->position.y -= velocidadAtualNado * deltaTime;
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        player->position.y += velocidadAtualNado * deltaTime;
    }

    stage->distanceTraveled += 15.0f * deltaTime; 
    if (stage->distanceTraveled >= 600.0f) {
        stage->distanceTraveled = 600.0f;
        stage->mode = STAGE2_MODE_FINISHED; 
        stage->stage2Complete = 1;
        return;
    }

    int screenW = GetRenderWidth() > 0 ? GetRenderWidth() : 800;
    int screenH = GetScreenHeight() > 0 ? GetScreenHeight() : 600;

    if (player->position.x < 0) player->position.x = 0;
    if (player->position.x > (float)screenW - player->width) player->position.x = (float)screenW - player->width;
    if (player->position.y < 0) player->position.y = 0;
    if (player->position.y > (float)screenH - player->height) player->position.y = (float)screenH - player->height;

    player->hitbox.width = player->width - 40.0f;
    player->hitbox.height = player->height - 40.0f;
    player->hitbox.x = player->position.x + 20.0f; 
    player->hitbox.y = player->position.y + 20.0f;

    handleBreathRecovery(stage, player, deltaTime);

    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        spawnSeaObstacle(stage);
        stage->obstacleSpawnTimer = 0.0f;
    }

    sortStage2Obstacles(&stage->obstacleQueue);
    scrollAndCleanObstacles(stage, player, deltaTime);
    handleCollisionsStage2(stage, player);

    if (player->position.y > 5.0f) {
        stage->breath -= 4.0f * deltaTime; 
    }
    
    if (stage->breath <= 0) {
        stage->breath = 0;
        player->lives = 0;
    }

    stage->backgroundScroll += 150.0f * deltaTime;
}

static void updateAndDrawBubbles(float deltaTime) {
    int screenW = GetRenderWidth();
    int screenH = GetScreenHeight();

    if (texturaBolhas.id == 0) return;

    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (!bubbles[i].active && (rand() % 100 < 2)) {
            bubbles[i].position.x = (float)(rand() % screenW);
            bubbles[i].position.y = (float)(screenH + 50);
            bubbles[i].speed = (float)(rand() % 100 + 80);
            bubbles[i].scale = (float)(rand() % 4 + 1) / 10.0f; 
            bubbles[i].wobbleSpeed = (float)(rand() % 4 + 2);
            bubbles[i].wobbleRange = (float)(rand() % 15 + 5);
            bubbles[i].alpha = (float)(rand() % 5 + 4) / 10.0f; 
            bubbles[i].active = 1;
        }

        if (bubbles[i].active) {
            bubbles[i].position.y -= bubbles[i].speed * deltaTime;
            
            float wobble = sinf(GetTime() * bubbles[i].wobbleSpeed) * bubbles[i].wobbleRange * deltaTime;
            bubbles[i].position.x += wobble;

            if (bubbles[i].position.y < 150.0f) {
                bubbles[i].alpha -= 1.5f * deltaTime;
            }

            if (bubbles[i].position.y < -50.0f || bubbles[i].alpha <= 0.0f) {
                bubbles[i].active = 0;
                continue;
            }

            Rectangle source = { 0.0f, 0.0f, (float)texturaBolhas.width, (float)texturaBolhas.height };
            Rectangle dest = { 
                bubbles[i].position.x, 
                bubbles[i].position.y, 
                texturaBolhas.width * bubbles[i].scale, 
                texturaBolhas.height * bubbles[i].scale 
            };
            
            unsigned char a = (unsigned char)(bubbles[i].alpha * 255);
            DrawTexturePro(texturaBolhas, source, dest, (Vector2){0, 0}, 0.0f, (Color){255, 255, 255, a});
        }
    }
}

// =========================================================================
// RENDERS INDIVIDUAIS E EXCLUSIVOS
// =========================================================================
static void drawCrabObstacle(Stage2Obstacle obs) {
    if (crabTextures[0].id > 0 && crabTextures[1].id > 0) {
        Crab tempCrab = createCrab(obs.position, 6);
        int frameCalculado = ((int)(GetTime() * 5)) % 2; 
        tempCrab.currentFrame = frameCalculado;
        drawCrab(tempCrab, crabTextures);
    } else {
        DrawRectangleRec(obs.hitbox, RED);
    }
}

static void drawHoleObstacle(Stage2Obstacle obs) {
    if (texturaBuraco.id == 0) {
        texturaBuraco = LoadTexture("assets/img/hole.png");
    }

    if (texturaBuraco.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)texturaBuraco.width, (float)texturaBuraco.height };
        Rectangle dest = { obs.position.x - 15.0f, obs.position.y - 20.0f, 130.0f, 55.0f }; 
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(texturaBuraco, source, dest, origin, 0.0f, WHITE);
    } else {
        DrawRectangleRec(obs.hitbox, BLACK);
    }
}

static void drawPlasticBagObstacle(Stage2Obstacle obs) {
    if (texturaSacola.id == 0) {
        texturaSacola = LoadTexture("assets/img/PlasticBag.png");
    }

    if (texturaSacola.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)texturaSacola.width, (float)texturaSacola.height };
        Rectangle dest = { obs.position.x, obs.position.y, 80.0f, 80.0f }; 
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(texturaSacola, source, dest, origin, 0.0f, WHITE);
    } else {
        DrawRectangleRec(obs.hitbox, WHITE);
    }
}

static void drawStage2Obstacle(Stage2Obstacle obs, Stage2 *stage) {
    if (!obs.active) return;

    int frame = ((int)(GetTime() * 4)) % 2; 

    if (obs.type == S2_OBS_CRAB) {
        drawCrabObstacle(obs);
    } else if (obs.type == S2_OBS_TRASH) {
        if (stage->mode == STAGE2_MODE_SEA) {
            drawPlasticBagObstacle(obs); 
        } else {
            drawHoleObstacle(obs);       
        }
    } 
    else if (obs.type == S2_OBS_SHARK) {
        if (txSharkR1.id > 0 && txSharkR2.id > 0) {
            Texture2D txShark = txSharkR1;
            if (obs.position.y >= 0) {
                txShark = (frame == 0) ? txSharkR1 : txSharkR2; 
            } else {
                txShark = (frame == 0) ? txSharkL1 : txSharkL2; 
            }
            Rectangle source = { 0.0f, 0.0f, (float)txShark.width, (float)txShark.height };
            Rectangle dest = { obs.position.x, fabsf(obs.position.y), 420.0f, 200.0f }; 
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(txShark, source, dest, origin, 0.0f, WHITE);
        } else {
            DrawRectangleRec(obs.hitbox, RED); 
        }
    } 
    else if (obs.type == S2_OBS_JELLYFISH) {
        if (txJelly1.id > 0) {
            Texture2D txJelly = (frame == 0) ? txJelly1 : txJelly2; 
            Rectangle source = { 0.0f, 0.0f, (float)txJelly.width, (float)txJelly.height };
            Rectangle dest = { obs.position.x, obs.position.y, 140.0f, 180.0f }; 
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(txJelly, source, dest, origin, 0.0f, WHITE);
        } else {
            DrawCircle((int)obs.position.x + 70, (int)obs.position.y + 90, 70, PURPLE);
        }
    }
    else if (obs.type == S2_OBS_NET) {
        if (txFishingNet.id > 0) {
            Rectangle source = { 0.0f, 0.0f, (float)txFishingNet.width, (float)txFishingNet.height };
            Rectangle dest = { obs.position.x, obs.position.y, 150.0f, 150.0f }; 
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(txFishingNet, source, dest, origin, 0.0f, WHITE);
        } else {
            DrawRectangleLines((int)obs.hitbox.x, (int)obs.hitbox.y, (int)obs.hitbox.width, (int)obs.hitbox.height, GREEN);
        }
    }
}

static void drawHUD(Stage2 *stage, int isSeaMode) {
    DrawText("VIDA:", 15, 120, 20, isSeaMode ? WHITE : DARKGRAY);
    DrawRectangle(85, 120, 200, 20, (Color){60, 60, 60, 200});
    
    float pctVida = playerHealth / 100.0f;
    if (pctVida < 0.0f) pctVida = 0.0f;
    Color corVida = (pctVida > 0.4f) ? GREEN : RED;
    DrawRectangle(85, 120, (int)(200 * pctVida), 20, corVida);
    DrawRectangleLines(85, 120, 200, 20, isSeaMode ? WHITE : BLACK);

    if (isSeaMode) {
        DrawText("FOLEGO:", 15, 155, 20, WHITE);
        DrawRectangle(115, 155, 200, 20, (Color){60, 60, 60, 200});
        
        float pctFolego = stage->breath / 100.0f;
        if (pctFolego < 0.0f) pctFolego = 0.0f;
        DrawRectangle(115, 155, (int)(200 * pctFolego), 20, SKYBLUE);
        DrawRectangleLines(115, 155, 200, 20, WHITE);
    }
}

static void drawSand(Stage2 *stage) {
    int screenWidth = GetRenderWidth();
    int screenHeight = GetRenderHeight();

    if (bgLoaded && stage->bgSand.id > 0) {
        float bgScroll = fmod(stage->backgroundScroll * 0.25f, screenWidth);
        Rectangle source = { 0.0f, 0.0f, (float)stage->bgSand.width, (float)stage->bgSand.height };

        for (int i = 0; i < 2; i++) {
            Rectangle dest = { (i * screenWidth) - bgScroll, 0.0f, (float)screenWidth, (float)screenHeight };
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(stage->bgSand, source, dest, origin, 0.0f, WHITE);
        }
    }

    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle, stage); 
        cur = cur->next;
    }

    drawHUD(stage, 0);
}

static void drawSea(Stage2 *stage, Player *player) {
    (void)player;
    int screenWidth = GetRenderWidth();
    int screenHeight = GetRenderHeight();

    if (bgOceanLoaded && bgOceano.id > 0) {
        float oceanScroll = fmod(stage->backgroundScroll * 0.25f, screenWidth);
        Rectangle source = { 0.0f, 0.0f, (float)bgOceano.width, (float)bgOceano.height };

        for (int i = 0; i < 2; i++) {
            Rectangle dest = { (i * screenWidth) - oceanScroll, 0.0f, (float)screenWidth, (float)screenHeight };
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(bgOceano, source, dest, origin, 0.0f, WHITE);
        }
    } else {
        DrawRectangleGradientV(0, 0, (float)screenWidth, (float)screenHeight, (Color){30, 100, 180, 255}, (Color){5, 30, 90, 255});
    }

    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle, stage); 
        cur = cur->next;
    }

    updateAndDrawBubbles(GetFrameTime());

    drawHUD(stage, 1);

    if (netDebuffTimer > 0.0f) {
        DrawText("PRESO NA REDE!", screenWidth / 2 - 80, 20, 20, RED);
    }
}

// RESTAURADO COM SCROLL DE SUPERFÍCIE, SINAL DE ALERTA ALPHABYTE E REQUISITOS DE BOLD MANUAL
static void drawTransition(Stage2 *stage) {
    int screenWidth = GetRenderWidth();
    int screenHeight = GetRenderHeight();
    
    if (bgSurfaceLoaded && bgSuperficieMar.id > 0) {
        float scrollSurf = fmod(stage->backgroundScroll * 0.25f, screenWidth);
        Rectangle source = { 0.0f, 0.0f, (float)bgSuperficieMar.width, (float)bgSuperficieMar.height };

        for (int i = 0; i < 2; i++) {
            Rectangle dest = { (i * screenWidth) - scrollSurf, 0.0f, (float)screenWidth, (float)screenHeight };
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(bgSuperficieMar, source, dest, origin, 0.0f, WHITE);
        }
    } else {
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){15, 60, 140, 255});
    }

    float frequenciaPisca = 8.0f; 
    float alphaPisca = (sinf(stage->modeTimer * frequenciaPisca) + 1.0f) / 2.0f; 
    unsigned char alphaByte = (unsigned char)(alphaPisca * 255);

    if (txSharkSign.id > 0) {
        float signWidth = 200.0f;
        float signHeight = 200.0f;
        float signX = ((float)screenWidth / 2.0f) - (signWidth / 2.0f);
        float signY = ((float)screenHeight * 0.30f) - (signHeight / 2.0f);

        Rectangle source = { 0.0f, 0.0f, (float)txSharkSign.width, (float)txSharkSign.height };
        Rectangle dest = { signX, signY, signWidth, signHeight };
        DrawTexturePro(txSharkSign, source, dest, (Vector2){0,0}, 0.0f, (Color){255, 255, 255, alphaByte});
    }

    const char *warningMsg = "CUIDADO: TERRITÓRIO DE TUBARÕES!";
    int fontSize = 35;
    int textWidth = MeasureText(warningMsg, fontSize);
    int posX = (screenWidth - textWidth) / 2;
    int posY = (int)(screenHeight * 0.48f);

    DrawText(warningMsg, posX + 1, posY, fontSize, (Color){255, 60, 60, alphaByte});
    DrawText(warningMsg, posX - 1, posY, fontSize, (Color){255, 60, 60, alphaByte});
    DrawText(warningMsg, posX, posY + 1, fontSize, (Color){255, 60, 60, alphaByte});
    DrawText(warningMsg, posX, posY - 1, fontSize, (Color){255, 60, 60, alphaByte});
    DrawText(warningMsg, posX, posY, fontSize, (Color){255, 60, 60, alphaByte});
}

void drawStage2(Stage2 *stage, Player *player) {
    if (stage->mode == STAGE2_MODE_FINISHED) return;

    switch (stage->mode) {
        case STAGE2_MODE_SAND:       drawSand(stage); break;
        case STAGE2_MODE_TRANSITION: drawTransition(stage); break;
        case STAGE2_MODE_SEA:        drawSea(stage, player); break;
        case STAGE2_MODE_FINISHED:   break;
    }

    Texture2D texturaAtual = txParadoDireita; 
    int frameGlobal = ((int)(GetTime() * 5)) % 2; 

    if (stage->mode == STAGE2_MODE_SAND) {
        float limiteChao = S2_AREIA_Y - player->height;
        int noAr = (player->position.y < limiteChao - 5.0f);

        if (noAr) {
            texturaAtual = olhandoParaDireita ? txPuloDireita : txPuloEsquerda;
        } else {
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                texturaAtual = (frameGlobal == 0) ? txParadoDireita : txMoverDireita;
            } 
            else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                texturaAtual = (frameGlobal == 0) ? txParadoEsquerda : txMoverEsquerda;
            } 
            else {
                if (olhandoParaDireita) {
                    texturaAtual = (frameGlobal == 0) ? txParadoDireita : txMoverDireita;
                } else {
                    texturaAtual = (frameGlobal == 0) ? txParadoEsquerda : txMoverEsquerda;
                }
            }
        }
    } 
    else if (stage->mode == STAGE2_MODE_TRANSITION) {
        // AJUSTADO: Garante que a animação de nado seja exibida durante a transição
        texturaAtual = (frameGlobal == 0) ? txNadarDireitaParado : txNadarDireitaAtivo;
    }
    else if (stage->mode == STAGE2_MODE_SEA) {
        if (netDebuffTimer > 0.0f) {
            if (olhandoParaDireita) {
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                    texturaAtual = txPresoDireitaMovendo; 
                } else {
                    texturaAtual = txPresoDireitaParado;  
                }
            } else {
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                    texturaAtual = txPresoEsquerdaMovendo; 
                } else {
                    texturaAtual = txPresoEsquerdaParado;  
                }
            }
        }
        else if (stage->breath <= 20.0f) {
            texturaAtual = (frameGlobal == 0) ? txCansado1 : txCansado2;
        } 
        else {
            if (olhandoParaDireita) {
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                    texturaAtual = txNadarDireitaAtivo;
                } else {
                    texturaAtual = txNadarDireitaParado;
                }
            } else {
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                    texturaAtual = txNadarEsquerdaAtivo;
                } else {
                    texturaAtual = txNadarEsquerdaParado;
                }
            }
        }
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