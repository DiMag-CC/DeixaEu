#include "stage2.h"
#include "../entities/crab.h" // Garanta que o cabeçalho do caranguejo esteja incluído aqui
#include "../utils/gameConstants.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Protótipos antecipados para o GCC aceitar a ordem de chamada das funções
static void drawStage2Obstacle(Stage2Obstacle obs);
static void drawTransition(Stage2 *stage);
static void updateSand(Stage2 *stage, Player *player, float deltaTime);
static void updateTransition(Stage2 *stage, Player *player, float deltaTime);
static void updateSea(Stage2 *stage, Player *player, float deltaTime);

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

// Array de texturas do caranguejo (Frames separados)
static Texture2D crabTextures[2];
static Texture2D texturaBuraco;

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

// Texturas de animação do personagem (Modo Mar - Mergulho)
static Texture2D txNadarDireitaAtivo;
static Texture2D txNadarDireitaParado;
static Texture2D txNadarEsquerdaAtivo;
static Texture2D txNadarEsquerdaParado;

// NOVAS TEXTURAS PARA O CENÁRIO SEPARADO
static Texture2D bgOceano;
static Texture2D plataformaAreia;
static int bgLoaded = 0;
static int bgOceanLoaded = 0;
static int platformLoaded = 0;

// Variável estática para lembrar a orientação do jogador (1 = Direita, 0 =
// Esquerda)
static int olhandoParaDireita = 1;

// Cronômetro estático interno para controlar a duração do efeito de lentidão da
// rede
static float netDebuffTimer = 0.0f;

#define S2_AREIA_Y 975.0f
#define S2_GRAVIDADE 3200.0f
#define S2_FORCA_PULO -1220.0f

// Constante de velocidade de nado livre no mar
#define S2_VELOCIDADE_NADO 400.0f

// =========================================================================
// REQUISITO 4: ALGORITMO DE ORDENAÇÃO (INSERTION SORT)
// Ordena a lista encadeada de obstáculos baseada na posição X (mais próximos
// primeiro)
// =========================================================================
static void sortStage2Obstacles(Stage2Queue *queue) {
  if (queue->front == NULL || queue->front->next == NULL)
    return;

  Stage2Node *sorted = NULL;
  Stage2Node *current = queue->front;

  while (current != NULL) {
    Stage2Node *next = current->next;

    // Inserção ordenada na lista auxiliar
    if (sorted == NULL ||
        sorted->obstacle.position.x >= current->obstacle.position.x) {
      current->next = sorted;
      sorted = current;
    } else {
      Stage2Node *temp = sorted;
      while (temp->next != NULL &&
             temp->next->obstacle.position.x < current->obstacle.position.x) {
        temp = temp->next;
      }
      current->next = temp->next;
      temp->next = current;
    }
    current = next;
  }
  queue->front = sorted;

  // Atualiza o ponteiro do 'rear' (fim da fila) reconstruindo a referência
  Stage2Node *temp = queue->front;
  while (temp != NULL && temp->next != NULL) {
    temp = temp->next;
  }
  queue->rear = temp;
}

// =========================================================================
// REQUISITO 5 & 6: FUNÇÕES DA ESTRUTURA DE DADOS ADOTADA
// Mapeamento das funções dinâmicas da fila encadeada FIFO
// GESTÃO DE OBSTÁCULOS E SISTEMA FIFO
// =========================================================================

// Geração e Enfileiramento (Enqueue) de Obstáculos na Areia
static void spawnSandObstacle(Stage2 *stage) {
  int roll = rand() % 100;
  int type = (roll < 50) ? S2_OBS_CRAB : S2_OBS_TRASH;

  Vector2 pos = {(float)GetRenderWidth() + 100.0f, S2_AREIA_Y};
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

// Geração de Inimigos no Mar (Tubarão, Água-viva e Rede de Pesca)
static void spawnSeaObstacle(Stage2 *stage) {
  int roll = rand() % 100;
  int type;

  if (roll < 35)
    type = S2_OBS_SHARK;
  else if (roll < 70)
    type = S2_OBS_JELLYFISH;
  else
    type = S2_OBS_NET;

  Vector2 pos = {0};
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
  } else if (type == S2_OBS_JELLYFISH) {
    pos.x = (float)GetRenderWidth() + 150.0f;
    pos.y = (float)(rand() % (screenH - 250) + 100);
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
  } else if (type == S2_OBS_JELLYFISH) {
    obs.hitbox.width = 70.0f;
    obs.hitbox.height = 130.0f;
  } else if (type == S2_OBS_NET) {
    obs.hitbox.width = 110.0f;
    obs.hitbox.height = 110.0f;
  }

  if (type == S2_OBS_SHARK) {
    obs.hitbox.x = obs.position.x + 60.0f;
    obs.hitbox.y = fabsf(obs.position.y) + 40.0f;
  } else if (type == S2_OBS_JELLYFISH) {
    obs.hitbox.x = obs.position.x + 35.0f;
    obs.hitbox.y = obs.position.y + 20.0f;
  } else {
    obs.hitbox.x = obs.position.x + 20.0f;
    obs.hitbox.y = obs.position.y + 20.0f;
  }

  enqueueStage2(&stage->obstacleQueue, obs);
}

// Colisões e penalidades de vida
static void handleCollisionsStage2(Stage2 *stage, Player *player) {
  Stage2Node *cur = stage->obstacleQueue.front;
  while (cur != NULL) {
    Stage2Obstacle *o = &cur->obstacle;

    if (o->active && CheckCollisionRecs(player->hitbox, o->hitbox)) {
      if (o->type == S2_OBS_CRAB) {
        stage->breath = 0.0f;
        player->lives = 0;
        o->active = 0;
      } else if (o->type == S2_OBS_TRASH) {
        stage->breath -= 30.0f;
        if (stage->breath <= 0.0f) {
          stage->breath = 0.0f;
          player->lives = 0;
        }
        o->active = 0;
      } else if (o->type == S2_OBS_SHARK) {
        player->lives = 0;
        stage->breath = 0.0f;
        o->active = 0;
      } else if (o->type == S2_OBS_JELLYFISH) {
        stage->breath -= 30.0f;
        if (stage->breath <= 0.0f) {
          stage->breath = 0.0f;
          player->lives = 0;
        }
        o->active = 0;
      } else if (o->type == S2_OBS_NET) {
        netDebuffTimer = 5.0f;
        o->active = 0;
        printf("[DEBUFF] Preso na rede! 5 segundos de lentidão.\n");
      }
    }
    cur = cur->next;
  }
}

// Funções da ED 3, 4 e 5: Modificação, Captura de Tamanho e Desenfileiramento
// (Dequeue) Movimentação dos obstáculos ativos na tela
static void scrollAndCleanObstacles(Stage2 *stage, Player *player,
                                    float deltaTime) {
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
// Função da ED 6: Inicialização Construtora
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

  bgLoaded = 0;
  stage->bgSand = LoadTexture("assets/img/landscapeLevel2.png");
  if (stage->bgSand.id != 0)
    bgLoaded = 1;

  bgOceanLoaded = 0;
  bgOceano = LoadTexture("assets/img/landscapeOceanlevel2.png");
  if (bgOceano.id != 0)
    bgOceanLoaded = 1;

  platformLoaded = 0;
  plataformaAreia = LoadTexture("assets/img/plataformLevel2.png");
  if (plataformaAreia.id != 0)
    platformLoaded = 1;

  stage->bgSea = (Texture2D){0};

  // CORREÇÃO: Alimenta o array estático com os dois frames separados
  crabTextures[0] = LoadTexture("assets/img/crab1.png");
  crabTextures[1] = LoadTexture("assets/img/crab2.png");
  texturaBuraco = LoadTexture("assets/img/hole.png");

  txSharkR1 = LoadTexture("assets/img/sharkR1.png");
  txSharkR2 = LoadTexture("assets/img/sharkR2.png");
  txSharkL1 = LoadTexture("assets/img/sharkL1.png");
  txSharkL2 = LoadTexture("assets/img/sharkL2.png");
  txJelly1 = LoadTexture("assets/img/jellyfish1.png");
  txJelly2 = LoadTexture("assets/img/jellyfish2.png");
  txFishingNet = LoadTexture("assets/img/FishingNet.png");

  txMoverDireita = LoadTexture("assets/img/characterMovingR1.png");
  txMoverEsquerda = LoadTexture("assets/img/characterMovingL1.png");
  txPuloDireita = LoadTexture("assets/img/CharacterJumpingR.png");
  txPuloEsquerda = LoadTexture("assets/img/CharacterJumpingL.png");

  txNadarDireitaAtivo = LoadTexture("assets/img/CharacterSwimmingR2.png");
  txNadarDireitaParado = LoadTexture("assets/img/CharacterSwimmingR1.png");
  txNadarEsquerdaAtivo = LoadTexture("assets/img/CharacterSwimmingL1.png");
  txNadarEsquerdaParado = LoadTexture("assets/img/CharacterSwimmingL2.png");

  olhandoParaDireita = 1;

  if (!bgLoaded || !platformLoaded || crabTextures[0].id == 0 ||
      texturaBuraco.id == 0) {
    printf("[AVISO] Erro crítico: Falha ao carregar texturas de cenário ou "
           "obstáculos na Stage 2!\n");
  }
}

// Função da ED 7: Destruição e Desalocação (Clear/Free)
void unloadStage2(Stage2 *stage) {
  if (bgLoaded) {
    UnloadTexture(stage->bgSand);
    bgLoaded = 0;
  }
  if (bgOceanLoaded) {
    UnloadTexture(bgOceano);
    bgOceanLoaded = 0;
  }
  if (platformLoaded) {
    UnloadTexture(plataformaAreia);
    platformLoaded = 0;
  }
  if (stage->bgSea.id > 0) {
    UnloadTexture(stage->bgSea);
  }

  UnloadTexture(crabTextures[0]);
  UnloadTexture(crabTextures[1]);
  UnloadTexture(texturaBuraco);
  UnloadTexture(txMoverDireita);
  UnloadTexture(txMoverEsquerda);
  UnloadTexture(txPuloDireita);
  UnloadTexture(txPuloEsquerda);

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

  freeStage2Queue(&stage->obstacleQueue);
}

void updateStage2(Stage2 *stage, Player *player, float deltaTime) {
  switch (stage->mode) {
  case STAGE2_MODE_SAND:
    updateSand(stage, player, deltaTime);
    break;
  case STAGE2_MODE_TRANSITION:
    updateTransition(stage, player, deltaTime);
    break;
  case STAGE2_MODE_SEA:
    updateSea(stage, player, deltaTime);
    break;
  case STAGE2_MODE_FINISHED:
    break;
  }
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

  if (IsKeyPressed(KEY_R)) {
    stage->distanceTraveled = 1500.0f;
  }

  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    olhandoParaDireita = 1;
  else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    olhandoParaDireita = 0;

  float fatorDificuldade = 1.0f + (stage->distanceTraveled / 300.0f) * 0.15f;
  if (fatorDificuldade > 2.0f)
    fatorDificuldade = 2.0f;

  stage->distanceTraveled += 25.0f * deltaTime;

  if (stage->distanceTraveled >= 1500.0f) {
    stage->distanceTraveled = 1500.0f;
    stage->mode = STAGE2_MODE_TRANSITION;
    stage->modeTimer = 0.0f;
    stage->breath = 100.0f;
    freeStage2Queue(&stage->obstacleQueue);
    initStage2Queue(&stage->obstacleQueue);

    player->position.x = 200.0f;
    player->position.y = (float)GetScreenHeight() * 0.5f;
    return;
  }

  float velocidadAtual = 450.0f * fatorDificuldade;
  stage->spawnInterval = 1.3f / fatorDificuldade;
  stage->backgroundScroll += velocidadAtual * deltaTime;

  player->width = 140.0f;
  player->height = 175.0f;

  float limiteChao = S2_AREIA_Y - player->height;

  if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) ||
       IsKeyPressed(KEY_W)) &&
      player->position.y >= limiteChao - 5.0f) {
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

  // Ordena a fila dinamicamente antes de mover
  sortStage2Obstacles(&stage->obstacleQueue);

  float velocidadeAntiga = stage->scrollSpeed;
  stage->scrollSpeed = velocidadAtual;
  scrollAndCleanObstacles(stage, player, deltaTime);
  stage->scrollSpeed = velocidadeAntiga;

  player->hitbox.width = player->width - 40.0f;
  player->hitbox.height = player->height - 10.0f;
  player->hitbox.x = player->position.x + 20.0f;
  player->hitbox.y = player->position.y;

  handleCollisionsStage2(stage, player);
}

static void updateTransition(Stage2 *stage, Player *player, float deltaTime) {
  (void)player;
  stage->modeTimer += deltaTime;
  if (stage->modeTimer >= STAGE2_TRANSITION_TIME) {
    stage->mode = STAGE2_MODE_SEA;
    stage->modeTimer = 0.0f;
    stage->obstacleSpawnTimer = 0.0f;
    stage->spawnInterval = 1.2f;
  }
}

static void updateSea(Stage2 *stage, Player *player, float deltaTime) {
  if (player->lives <= 0) {
    if (IsKeyPressed(KEY_ENTER)) {
      player->lives = 3;
      stage->breath = 100.0f;
      player->position.x = 200.0f;
      player->position.y = (float)GetScreenHeight() * 0.5f;
      freeStage2Queue(&stage->obstacleQueue);
      initStage2Queue(&stage->obstacleQueue);
      netDebuffTimer = 0.0f;
    }
    return;
  }

  float modificadorVelocidade = 1.0f;
  if (netDebuffTimer > 0.0f) {
    netDebuffTimer -= deltaTime;
    modificadorVelocidade = 0.5f;
  }

  float velocidadeAtualNado = S2_VELOCIDADE_NADO * modificadorVelocidade;

  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
    player->position.x += velocidadeAtualNado * deltaTime;
    olhandoParaDireita = 1;
  }
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
    player->position.x -= velocidadeAtualNado * deltaTime;
    olhandoParaDireita = 0;
  }
  if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
    player->position.y -= velocidadeAtualNado * deltaTime;
  }
  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
    player->position.y += velocidadeAtualNado * deltaTime;
  }

  int screenW = GetRenderWidth() > 0 ? GetRenderWidth() : 800;
  int screenH = GetScreenHeight() > 0 ? GetScreenHeight() : 600;

  if (player->position.x < 0)
    player->position.x = 0;
  if (player->position.x > (float)screenW - player->width)
    player->position.x = (float)screenW - player->width;
  if (player->position.y < 0)
    player->position.y = 0;
  if (player->position.y > (float)screenH - player->height)
    player->position.y = (float)screenH - player->height;

  player->hitbox.width = player->width - 40.0f;
  player->hitbox.height = player->height - 40.0f;
  player->hitbox.x = player->position.x + 20.0f;
  player->hitbox.y = player->position.y + 20.0f;

  stage->obstacleSpawnTimer += deltaTime;
  if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
    spawnSeaObstacle(stage);
    stage->obstacleSpawnTimer = 0.0f;
  }

  sortStage2Obstacles(&stage->obstacleQueue);
  scrollAndCleanObstacles(stage, player, deltaTime);
  handleCollisionsStage2(stage, player);

  stage->breath -= 4.0f * deltaTime;
  if (stage->breath <= 0) {
    stage->breath = 0;
    player->lives = 0;
  }

  stage->backgroundScroll += 150.0f * deltaTime;
}

// =========================================================================
// RENDERS INDIVIDUAIS E EXCLUSIVOS
// =========================================================================

// CORREÇÃO: Utiliza o novo método drawCrab do seu crab.c para animar
static void drawCrabObstacle(Stage2Obstacle obs) {
  if (crabTextures[0].id > 0 && crabTextures[1].id > 0) {
    // Criamos uma entidade temporária de controle baseada na posição do nó na
    // fila
    Crab tempCrab = createCrab(obs.position, 6);

    // Sincroniza o frame baseado no tempo de execução global
    int frameCalculado = ((int)(GetTime() * 5)) % 2;
    tempCrab.currentFrame = frameCalculado;

    // Desenha usando o seu próprio escopo do crab.c
    drawCrab(tempCrab, crabTextures);
  } else {
    DrawRectangleRec(obs.hitbox, RED);
  }
}

static void drawHoleObstacle(Stage2Obstacle obs) {
  if (texturaBuraco.id > 0) {
    Rectangle source = {0.0f, 0.0f, (float)texturaBuraco.width,
                        (float)texturaBuraco.height};
    Rectangle dest = {obs.position.x, obs.position.y, 145.0f, 40.0f};
    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(texturaBuraco, source, dest, origin, 0.0f, WHITE);
  } else {
    DrawEllipse(obs.position.x + (obs.hitbox.width / 2.0f),
                obs.position.y + (obs.hitbox.height / 2.0f),
                obs.hitbox.width / 2.0f, obs.hitbox.height / 2.0f, BLACK);
  }
}

static void drawStage2Obstacle(Stage2Obstacle obs) {
  if (!obs.active)
    return;

  int frame = ((int)(GetTime() * 4)) % 2;

  if (obs.type == S2_OBS_CRAB) {
    drawCrabObstacle(obs);
  } else if (obs.type == S2_OBS_TRASH) {
    drawHoleObstacle(obs);
  } else if (obs.type == S2_OBS_SHARK) {
    if (txSharkR1.id > 0 && txSharkL1.id > 0) {
      Texture2D txShark = txSharkR1;
      if (obs.position.y >= 0) {
        txShark = (frame == 0) ? txSharkR1 : txSharkR2;
      } else {
        txShark = (frame == 0) ? txSharkL1 : txSharkL2;
      }
      Rectangle source = {0.0f, 0.0f, (float)txShark.width,
                          (float)txShark.height};
      Rectangle dest = {obs.position.x, fabsf(obs.position.y), 320.0f, 150.0f};
      Vector2 origin = {0.0f, 0.0f};
      DrawTexturePro(txShark, source, dest, origin, 0.0f, WHITE);
    } else {
      DrawRectangleRec(obs.hitbox, RED);
    }
  } else if (obs.type == S2_OBS_JELLYFISH) {
    if (txJelly1.id > 0) {
      Texture2D txJelly = (frame == 0) ? txJelly1 : txJelly2;
      Rectangle source = {0.0f, 0.0f, (float)txJelly.width,
                          (float)txJelly.height};
      Rectangle dest = {obs.position.x, obs.position.y, 140.0f, 180.0f};
      Vector2 origin = {0.0f, 0.0f};
      DrawTexturePro(txJelly, source, dest, origin, 0.0f, WHITE);
    } else {
      DrawCircle((int)obs.position.x + 70, (int)obs.position.y + 90, 70,
                 PURPLE);
    }
  } else if (obs.type == S2_OBS_NET) {
    if (txFishingNet.id > 0) {
      Rectangle source = {0.0f, 0.0f, (float)txFishingNet.width,
                          (float)txFishingNet.height};
      Rectangle dest = {obs.position.x, obs.position.y, 150.0f, 150.0f};
      Vector2 origin = {0.0f, 0.0f};
      DrawTexturePro(txFishingNet, source, dest, origin, 0.0f, WHITE);
    } else {
      DrawRectangleLines((int)obs.hitbox.x, (int)obs.hitbox.y,
                         (int)obs.hitbox.width, (int)obs.hitbox.height, GREEN);
    }
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
    Rectangle source = {0.0f, 0.0f, (float)stage->bgSand.width,
                        (float)stage->bgSand.height};

    for (int i = 0; i < 2; i++) {
      Rectangle dest = {(i * screenWidth) - bgScroll, 0.0f, (float)screenWidth,
                        (float)screenHeight};
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
    Rectangle source = {0.0f, 0.0f, (float)plataformaAreia.width,
                        (float)plataformaAreia.height};

    for (int i = 0; i < 2; i++) {
      // Posicionado na parte inferior cobrindo a pista
      Rectangle dest = {(i * platformWidth) - platformScroll, 0.0f,
                        platformWidth, (float)screenHeight};
      DrawTexturePro(plataformaAreia, source, dest, (Vector2){0, 0}, 0.0f,
                     WHITE);
    }
  } else {
    DrawRectangle(0, S2_AREIA_Y, screenWidth, screenHeight - S2_AREIA_Y,
                  DARKBROWN);
  }

  Stage2Node *cur = stage->obstacleQueue.front;
  while (cur != NULL) {
    drawStage2Obstacle(cur->obstacle);
    cur = cur->next;
  }

  // Interface HUD
  DrawText("VIDA:", 15, 120, 20, DARKGRAY);
  DrawRectangle(85, 120, 200, 20, (Color){60, 60, 60, 200});
  float pct = stage->breath / 100.0f;
  if (pct < 0.0f)
    pct = 0.0f;
  Color corBarra = (pct > 0.4f) ? GREEN : RED;
  DrawRectangle(85, 120, (int)(200 * pct), 20, corBarra);
  DrawRectangleLines(85, 120, 200, 20, BLACK);
}

static void drawTransition(Stage2 *stage) {
  (void)stage;
  DrawRectangle(0, 0, (float)GetRenderWidth(), (float)GetRenderHeight(),
                DARKBLUE);
  const char *msg = "Mergulhando no mar...";
  int w = MeasureText(msg, 50);
  DrawText(msg, ((float)GetRenderWidth() - w) / 2, (float)GetRenderHeight() / 2,
           50, WHITE);
}

static void drawSea(Stage2 *stage, Player *player) {
  (void)player;
  int screenWidth = GetRenderWidth();
  int screenHeight = GetRenderHeight();

  if (bgOceanLoaded && bgOceano.id > 0) {
    float oceanScroll = fmod(stage->backgroundScroll * 0.25f, screenWidth);
    Rectangle source = {0.0f, 0.0f, (float)bgOceano.width,
                        (float)bgOceano.height};

    for (int i = 0; i < 2; i++) {
      Rectangle dest = {(i * screenWidth) - oceanScroll, 0.0f,
                        (float)screenWidth, (float)screenHeight};
      Vector2 origin = {0.0f, 0.0f};
      DrawTexturePro(bgOceano, source, dest, origin, 0.0f, WHITE);
    }
  } else {
    DrawRectangleGradientV(0, 0, (float)screenWidth, (float)screenHeight,
                           (Color){30, 100, 180, 255}, (Color){5, 30, 90, 255});
  }

  Stage2Node *cur = stage->obstacleQueue.front;
  while (cur != NULL) {
    drawStage2Obstacle(cur->obstacle);
    cur = cur->next;
  }

  DrawText("FOLEGO:", 15, 120, 20, WHITE);
  DrawRectangle(115, 120, 200, 20, (Color){60, 60, 60, 200});
  float pct = stage->breath / 100.0f;
  if (pct < 0.0f)
    pct = 0.0f;
  DrawRectangle(115, 120, (int)(200 * pct), 20, SKYBLUE);
  DrawRectangleLines(115, 120, 200, 20, WHITE);

  if (netDebuffTimer > 0.0f) {
    DrawText("PRESO NA REDE!", screenWidth / 2 - 80, 20, 20, RED);
  }
}

void drawStage2(Stage2 *stage, Player *player) {
  switch (stage->mode) {
  case STAGE2_MODE_SAND:
    drawSand(stage);
    break;
  case STAGE2_MODE_TRANSITION:
    drawTransition(stage);
    break;
  case STAGE2_MODE_SEA:
    drawSea(stage, player);
    break;
  case STAGE2_MODE_FINISHED:
    break;
  }

  Texture2D texturaAtual = txMoverDireita;

  if (stage->mode == STAGE2_MODE_SAND) {
    float limiteChao = S2_AREIA_Y - player->height;
    int noAr = (player->position.y < limiteChao - 5.0f);

    if (noAr) {
      texturaAtual = olhandoParaDireita ? txPuloDireita : txPuloEsquerda;
    } else {
      texturaAtual = olhandoParaDireita ? txMoverDireita : txMoverEsquerda;
    }
  } else if (stage->mode == STAGE2_MODE_SEA) {
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

  if (texturaAtual.id > 0) {
    Rectangle source = {0.0f, 0.0f, (float)texturaAtual.width,
                        (float)texturaAtual.height};
    Rectangle dest = {player->position.x, player->position.y, player->width,
                      player->height};
    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(texturaAtual, source, dest, origin, 0.0f, WHITE);
  } else {
    DrawRectangleRec(player->hitbox, BLUE);
  }
}