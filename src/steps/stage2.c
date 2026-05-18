// stage2.c
// Implementação da Fase 2 — Boa Viagem.
//
// Organização:
//   - init/unload: setup e limpeza
//   - update: dispatcher que chama updateSand / updateTransition / updateSea
//   - draw:   dispatcher que chama drawSand / drawTransition / drawSea
//   - cada modo tem suas funções estáticas de spawn e colisão

#include "stage2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ============================================================
//                    HELPERS INTERNOS
// ============================================================

// Tenta carregar uma textura; se o arquivo não existir o raylib retorna id=0
// e a função drawSand/drawSea cai pro placeholder.
static Texture2D safeLoadTexture(const char *path) {
    Texture2D t = LoadTexture(path);
    return t;
}

// Spawna um obstáculo da Parte 1 (areia) por probabilidade.
static void spawnSandObstacle(Stage2 *stage) {
    int roll = rand() % 100;
    Stage2ObstacleType type;
    Vector2 pos = { GetScreenWidth(), GROUND_LEVEL + 20 };

    if (roll < 25) {
        type = S2_OBS_CRAB;
    } else if (roll < 45) {
        type = S2_OBS_SANDCASTLE;
    } else if (roll < 60) {
        type = S2_OBS_UMBRELLA_BEACH;
        pos.y = GROUND_LEVEL - 20;  // mais alto
    } else if (roll < 75) {
        type = S2_OBS_VENDOR;
        pos.y = GROUND_LEVEL - 10;
    } else if (roll < 85) {
        type = S2_OBS_BEACH_WAVE;
        pos.y = GROUND_LEVEL + 30;
    } else if (roll < 95) {
        type = S2_OBS_KITE;
        pos.y = 80 + (rand() % 100);   // vem do alto
    } else {
        // 5% — power-up
        type = S2_OBS_COCONUT;
        pos.y = GROUND_LEVEL - 40;
    }

    Stage2Obstacle obs = createStage2Obstacle(pos, type);
    enqueueStage2(&stage->obstacleQueue, obs);
}

// Spawna um obstáculo da Parte 2 (mar) por probabilidade.
static void spawnSeaObstacle(Stage2 *stage) {
    int roll = rand() % 100;
    Stage2ObstacleType type;
    Vector2 pos = { GetScreenWidth(), 0 };

    if (roll < 35) {
        type = S2_OBS_JELLYFISH;
        pos.y = 120 + (rand() % 200);
    } else if (roll < 60) {
        type = S2_OBS_TRASH;
        pos.y = 100 + (rand() % 250);
    } else if (roll < 75) {
        type = S2_OBS_CURRENT;
        pos.y = 100 + (rand() % 200);
    } else if (roll < 92) {
        type = S2_OBS_FISHNET;
        // ocupa quase toda a coluna, com brecha em y aleatório
        pos.y = (rand() % 2 == 0) ? 60.0f : 250.0f;
    } else {
        // 8% — onda gigante (só se ainda não tiver uma)
        if (countStage2ObstaclesByType(&stage->obstacleQueue, S2_OBS_BIG_WAVE) > 0) {
            return;  // pula spawn nesse ciclo
        }
        type = S2_OBS_BIG_WAVE;
        pos.y = 50.0f;
    }

    Stage2Obstacle obs = createStage2Obstacle(pos, type);
    enqueueStage2(&stage->obstacleQueue, obs);
}

// Aplica colisão entre player e obstáculos da fila.
// Retorna 1 se colidiu com algo que deve ser consumido (lixo, coco, etc).
static void handleSandCollisions(Stage2 *stage, Player *player) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        if (o->active && CheckCollisionRecs(player->hitbox, o->hitbox)) {
            switch (o->type) {
                case S2_OBS_CRAB:
                case S2_OBS_SANDCASTLE:
                case S2_OBS_UMBRELLA_BEACH:
                case S2_OBS_VENDOR:
                case S2_OBS_KITE:
                    player->lives -= 1;
                    o->active = 0;
                    o->position.x = -300.0f;
                    break;
                case S2_OBS_BEACH_WAVE:
                    // onda atrasa mas não tira vida
                    player->speed = 60.0f;
                    o->active = 0;
                    o->position.x = -300.0f;
                    break;
                case S2_OBS_COCONUT:
                    stage->hasCoconutBuff = 1;
                    stage->coconutBuffTimer = STAGE2_COCO_DURATION;
                    o->active = 0;
                    o->position.x = -300.0f;
                    break;
                default:
                    break;
            }
        }
        cur = cur->next;
    }
}

static void handleSeaCollisions(Stage2 *stage, Player *player) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        if (!o->active) { cur = cur->next; continue; }

        if (CheckCollisionRecs(player->hitbox, o->hitbox)) {
            switch (o->type) {
                case S2_OBS_JELLYFISH:
                    player->lives -= 1;
                    o->active = 0;
                    o->position.x = -300.0f;
                    break;
                case S2_OBS_TRASH:
                    player->speed *= 0.7f;
                    o->active = 0;
                    o->position.x = -300.0f;
                    break;
                case S2_OBS_FISHNET:
                    // prende: empurra player de volta (tubarão se aproxima)
                    player->position.x -= 80;
                    player->lives -= 1;
                    o->active = 0;
                    o->position.x = -300.0f;
                    break;
                case S2_OBS_CURRENT:
                    // empurra player pra baixo enquanto sobrepõe
                    stage->currentPushY += 100.0f;
                    break;
                case S2_OBS_BIG_WAVE:
                    // só causa dano se o player estiver na metade de cima
                    if (player->position.y < STAGE2_SEA_BOTTOM - 100) {
                        player->lives -= 1;
                        player->position.x -= 50;
                    }
                    break;
                default:
                    break;
            }
        }
        cur = cur->next;
    }

    // Tubarão (fora da fila)
    if (stage->sharkActive &&
        CheckCollisionRecs(player->hitbox, stage->sharkHitbox)) {
        player->lives -= 1;
        player->position.x += 60;   // empurra player pra frente
    }
}

// Avança e remove obstáculos que saíram da tela. Soma pontuação.
static void scrollAndCleanObstacles(Stage2 *stage, Player *player, float deltaTime) {
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        Stage2Obstacle *o = &cur->obstacle;
        o->position.x -= stage->scrollSpeed * deltaTime;

        // Movimentos especiais
        switch (o->type) {
            case S2_OBS_KITE:
                // pipa cai diagonalmente
                o->position.y += 30.0f * deltaTime;
                break;
            case S2_OBS_JELLYFISH:
                // sobe e desce
                o->verticalPhase += 2.5f * deltaTime;
                o->position.y += sinf(o->verticalPhase) * 25.0f * deltaTime;
                break;
            case S2_OBS_TRASH:
                o->verticalPhase += 1.5f * deltaTime;
                o->position.y += sinf(o->verticalPhase) * 10.0f * deltaTime;
                break;
            default:
                break;
        }

        // Atualiza hitbox pra acompanhar posição
        o->hitbox.x = o->position.x;
        o->hitbox.y = o->position.y;

        // Animação genérica (frame counter pra placeholders piscarem)
        o->animTimer += deltaTime;
        if (o->animTimer >= 0.15f) {
            o->animFrame = (o->animFrame + 1) % 6;
            o->animTimer = 0.0f;
        }

        cur = cur->next;
    }

    // Limpeza em lote + pontuação por passagem
    int sizeBefore = stage2QueueSize(&stage->obstacleQueue);
    removeOffscreenStage2(&stage->obstacleQueue, -200.0f);
    int sizeAfter = stage2QueueSize(&stage->obstacleQueue);
    int passed = sizeBefore - sizeAfter;
    if (passed > 0) {
        player->score += 10.0f * passed * stage->difficultyMultiplier;
    }
}

// ============================================================
//                       INIT / UNLOAD
// ============================================================

void initStage2(Stage2 *stage) {
    stage->mode = STAGE2_MODE_SAND;
    stage->modeTimer = 0.0f;
    stage->scrollSpeed = 180.0f;
    stage->distanceTraveled = 0.0f;
    stage->spawnInterval = 1.4f;
    stage->obstacleSpawnTimer = 0.0f;
    stage->difficultyMultiplier = 1.0f;
    stage->stage2Complete = 0;
    stage->backgroundScroll = 0.0f;

    initStage2Queue(&stage->obstacleQueue);

    stage->hasCoconutBuff = 0;
    stage->coconutBuffTimer = 0.0f;

    stage->sharkPosition = (Vector2){ STAGE2_SHARK_BASE_X, 200.0f };
    stage->sharkHitbox = (Rectangle){ STAGE2_SHARK_BASE_X, 200.0f, 120.0f, 60.0f };
    stage->sharkActive = 0;

    stage->breath = STAGE2_BREATH_MAX;
    stage->lightningTimer = 5.0f;
    stage->lightningFlash = 0.0f;
    stage->stormActive = 0;
    stage->currentPushY = 0.0f;

    // Texturas — pode falhar silenciosamente
    stage->bgSand = safeLoadTexture("assets/img/background2_sand.png");
    stage->bgSea  = safeLoadTexture("assets/img/background2_sea.png");
}

void unloadStage2(Stage2 *stage) {
    if (stage->bgSand.id > 0) UnloadTexture(stage->bgSand);
    if (stage->bgSea.id  > 0) UnloadTexture(stage->bgSea);
    freeStage2Queue(&stage->obstacleQueue);
}

// ============================================================
//                      UPDATE — POR MODO
// ============================================================

static void updateSand(Stage2 *stage, Player *player, float deltaTime) {
    // Dificuldade crescente
    stage->difficultyMultiplier = 1.0f + (stage->distanceTraveled / 8000.0f) * 0.5f;
    stage->spawnInterval = 1.4f / stage->difficultyMultiplier;

    // Buff do coco acelera o cenário
    float effectiveScroll = stage->scrollSpeed;
    if (stage->hasCoconutBuff) effectiveScroll *= 1.6f;

    stage->distanceTraveled += effectiveScroll * deltaTime;
    stage->backgroundScroll += effectiveScroll * 0.5f * deltaTime;
    if (stage->backgroundScroll >= GetScreenWidth()) stage->backgroundScroll = 0.0f;

    // Spawn
    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        spawnSandObstacle(stage);
        stage->obstacleSpawnTimer = 0.0f;
    }

    // Movimento dos obstáculos e limpeza
    float realScroll = stage->scrollSpeed;
    stage->scrollSpeed = effectiveScroll;
    scrollAndCleanObstacles(stage, player, deltaTime);
    stage->scrollSpeed = realScroll;

    // Colisões
    handleSandCollisions(stage, player);

    // Buff timer
    if (stage->hasCoconutBuff) {
        stage->coconutBuffTimer -= deltaTime;
        if (stage->coconutBuffTimer <= 0.0f) {
            stage->hasCoconutBuff = 0;
            stage->coconutBuffTimer = 0.0f;
        }
    }

    // Fim da Parte 1 → transição
    if (stage->distanceTraveled >= STAGE2_SAND_DISTANCE) {
        stage->mode = STAGE2_MODE_TRANSITION;
        stage->modeTimer = 0.0f;
        freeStage2Queue(&stage->obstacleQueue);  // limpa pra próxima parte
        initStage2Queue(&stage->obstacleQueue);
    }
}

static void updateTransition(Stage2 *stage, Player *player, float deltaTime) {
    (void)player;
    stage->modeTimer += deltaTime;
    if (stage->modeTimer >= STAGE2_TRANSITION_TIME) {
        stage->mode = STAGE2_MODE_SEA;
        stage->modeTimer = 0.0f;
        stage->distanceTraveled = 0.0f;
        stage->scrollSpeed = 160.0f;
        stage->breath = STAGE2_BREATH_MAX;
        // Tubarão entra em cena
        stage->sharkActive = 1;
        stage->sharkPosition = (Vector2){ STAGE2_SHARK_BASE_X, 200.0f };
        stage->stormActive = 1;
    }
}

static void updateSea(Stage2 *stage, Player *player, float deltaTime) {
    // Dificuldade
    stage->difficultyMultiplier = 1.0f + (stage->distanceTraveled / 10000.0f) * 0.4f;
    stage->spawnInterval = 1.6f / stage->difficultyMultiplier;

    stage->distanceTraveled += stage->scrollSpeed * deltaTime;
    stage->backgroundScroll += stage->scrollSpeed * 0.3f * deltaTime;
    if (stage->backgroundScroll >= GetScreenWidth()) stage->backgroundScroll = 0.0f;

    // Anula gravidade do player no mar — controle livre em Y
    player->velocity.y = 0;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        player->position.y -= 220.0f * deltaTime;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        player->position.y += 220.0f * deltaTime;
    }

    // Aplica empurrão de correntes acumulado
    player->position.y += stage->currentPushY * deltaTime;
    stage->currentPushY = 0.0f;  // reseta a cada frame

    // Limites do mar
    if (player->position.y < STAGE2_SEA_TOP)    player->position.y = STAGE2_SEA_TOP;
    if (player->position.y > STAGE2_SEA_BOTTOM) player->position.y = STAGE2_SEA_BOTTOM;

    // Fôlego
    if (player->position.y <= STAGE2_SEA_TOP + 20.0f) {
        stage->breath += STAGE2_BREATH_RECOVER * deltaTime;
        if (stage->breath > STAGE2_BREATH_MAX) stage->breath = STAGE2_BREATH_MAX;
    } else {
        stage->breath -= STAGE2_BREATH_DRAIN * deltaTime;
        if (stage->breath <= 0.0f) {
            stage->breath = 30.0f;
            player->lives -= 1;
        }
    }

    // Tubarão persegue na vertical
    if (stage->sharkActive) {
        float dy = player->position.y - stage->sharkPosition.y;
        stage->sharkPosition.y += dy * 1.5f * deltaTime;
        // aproxima lentamente em X
        stage->sharkPosition.x += 8.0f * deltaTime;
        if (stage->sharkPosition.x > STAGE2_SHARK_MAX_X) {
            stage->sharkPosition.x = STAGE2_SHARK_MAX_X;
        }
        stage->sharkHitbox.x = stage->sharkPosition.x;
        stage->sharkHitbox.y = stage->sharkPosition.y;
    }

    // Spawn
    stage->obstacleSpawnTimer += deltaTime;
    if (stage->obstacleSpawnTimer >= stage->spawnInterval) {
        spawnSeaObstacle(stage);
        stage->obstacleSpawnTimer = 0.0f;
    }

    // Movimento e colisões
    scrollAndCleanObstacles(stage, player, deltaTime);
    handleSeaCollisions(stage, player);

    // Relâmpagos durante tempestade
    if (stage->stormActive) {
        stage->lightningTimer -= deltaTime;
        if (stage->lightningTimer <= 0.0f) {
            stage->lightningFlash = 0.15f;
            stage->lightningTimer = 3.0f + (rand() % 4);
        }
        if (stage->lightningFlash > 0.0f) {
            stage->lightningFlash -= deltaTime;
        }
    }

    // Fim da fase
    if (stage->distanceTraveled >= STAGE2_SEA_DISTANCE) {
        stage->stage2Complete = 1;
        stage->mode = STAGE2_MODE_FINISHED;
    }
}

// ============================================================
//                  UPDATE PRINCIPAL (DISPATCHER)
// ============================================================

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
            // Aguarda main pegar a flag stage2Complete
            break;
    }
}

// ============================================================
//                        DRAW — POR MODO
// ============================================================

// Desenha um obstáculo da fila como placeholder colorido.
// Quando a arte chegar, troca cada case por DrawTextureRec.
static void drawStage2Obstacle(Stage2Obstacle obs) {
    if (!obs.active) return;

    Color color = GRAY;
    switch (obs.type) {
        case S2_OBS_CRAB:           color = RED;       break;
        case S2_OBS_UMBRELLA_BEACH: color = ORANGE;    break;
        case S2_OBS_VENDOR:         color = BROWN;     break;
        case S2_OBS_SANDCASTLE:     color = BEIGE;     break;
        case S2_OBS_BEACH_WAVE:     color = SKYBLUE;   break;
        case S2_OBS_KITE:           color = MAGENTA;   break;
        case S2_OBS_COCONUT:        color = GREEN;     break;
        case S2_OBS_JELLYFISH:      color = (Color){200, 100, 200, 180}; break;
        case S2_OBS_TRASH:          color = LIGHTGRAY; break;
        case S2_OBS_FISHNET:        color = DARKBROWN; break;
        case S2_OBS_BIG_WAVE:       color = DARKBLUE;  break;
        case S2_OBS_CURRENT:        color = (Color){0, 120, 255, 80}; break;
    }
    DrawRectangleRec(obs.hitbox, color);
    DrawRectangleLinesEx(obs.hitbox, 2, BLACK);

    // Detalhe pra coco pulsar
    if (obs.type == S2_OBS_COCONUT) {
        float pulse = 4.0f * sinf(obs.animTimer * 20.0f);
        DrawCircle(obs.position.x + 15, obs.position.y + 15, 10 + pulse, YELLOW);
    }
}

static void drawSand(Stage2 *stage) {
    // Céu (gradient placeholder)
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GROUND_LEVEL + 40,
                           SKYBLUE, (Color){255, 230, 180, 255});

    // Mar ao fundo (fina faixa)
    DrawRectangle(0, GROUND_LEVEL - 80, GetScreenWidth(), 40, BLUE);

    // Areia
    DrawRectangle(0, GROUND_LEVEL + 40, GetScreenWidth(),
                  GetScreenHeight() - (GROUND_LEVEL + 40), BEIGE);

    // Faixa do calçadão (aparece após metade da fase)
    if (stage->distanceTraveled > STAGE2_SAND_DISTANCE * 0.5f) {
        DrawRectangle(0, GROUND_LEVEL + 40, GetScreenWidth(), 20, LIGHTGRAY);
    }

    // Obstáculos
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle);
        cur = cur->next;
    }

    // Efeito visual do coco ativo
    if (stage->hasCoconutBuff) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      (Color){255, 230, 0, 40});
    }
}

static void drawTransition(Stage2 *stage) {
    // Fade de areia → mar
    float t = stage->modeTimer / STAGE2_TRANSITION_TIME;
    if (t > 1.0f) t = 1.0f;

    Color sky = SKYBLUE;
    Color sea = DARKBLUE;
    Color blend = {
        (unsigned char)(sky.r * (1 - t) + sea.r * t),
        (unsigned char)(sky.g * (1 - t) + sea.g * t),
        (unsigned char)(sky.b * (1 - t) + sea.b * t),
        255
    };
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), blend);

    const char *msg = "Mergulhando...";
    int w = MeasureText(msg, 30);
    DrawText(msg, (GetScreenWidth() - w) / 2, GetScreenHeight() / 2, 30, WHITE);
}

static void drawSea(Stage2 *stage, Player *player) {
    // Fundo do mar
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(),
                           (Color){30, 100, 180, 255},
                           (Color){5, 30, 90, 255});

    // Algas / fundo
    DrawRectangle(0, GetScreenHeight() - 30, GetScreenWidth(), 30,
                  (Color){50, 80, 40, 255});

    // Obstáculos
    Stage2Node *cur = stage->obstacleQueue.front;
    while (cur != NULL) {
        drawStage2Obstacle(cur->obstacle);
        cur = cur->next;
    }

    // Tubarão
    if (stage->sharkActive) {
        DrawRectangleRec(stage->sharkHitbox, DARKGRAY);
        DrawRectangleLinesEx(stage->sharkHitbox, 2, BLACK);
        // Olho
        DrawCircle(stage->sharkPosition.x + 95, stage->sharkPosition.y + 18,
                   4, RED);
        // Boca
        DrawTriangle(
            (Vector2){stage->sharkPosition.x + 110, stage->sharkPosition.y + 30},
            (Vector2){stage->sharkPosition.x + 90,  stage->sharkPosition.y + 38},
            (Vector2){stage->sharkPosition.x + 90,  stage->sharkPosition.y + 22},
            WHITE);
    }

    // Flash de relâmpago
    if (stage->lightningFlash > 0.0f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      (Color){255, 255, 255, 180});
    }

    // Overlay escuro de tempestade
    if (stage->stormActive && stage->lightningFlash <= 0.0f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      (Color){0, 0, 0, 80});
    }

    (void)player;  // hud do player desenhada externamente pelo main
}

static void drawFinished(Stage2 *stage) {
    (void)stage;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    const char *msg = "FASE 2 COMPLETA";
    int w = MeasureText(msg, 40);
    DrawText(msg, (GetScreenWidth() - w) / 2, 180, 40, GOLD);

    const char *sub = "É agora...";
    w = MeasureText(sub, 24);
    DrawText(sub, (GetScreenWidth() - w) / 2, 240, 24, WHITE);
}

// ============================================================
//                  DRAW PRINCIPAL (DISPATCHER)
// ============================================================

void drawStage2(Stage2 *stage, Player *player) {
    switch (stage->mode) {
        case STAGE2_MODE_SAND:       drawSand(stage); break;
        case STAGE2_MODE_TRANSITION: drawTransition(stage); break;
        case STAGE2_MODE_SEA:        drawSea(stage, player); break;
        case STAGE2_MODE_FINISHED:   drawFinished(stage); break;
    }

    // HUD específica da Fase 2 (modo SEA tem barra de fôlego)
    if (stage->mode == STAGE2_MODE_SEA) {
        DrawText("Folego", 10, 90, 14, WHITE);
        DrawRectangle(10, 110, 150, 12, (Color){60, 60, 60, 200});
        float pct = stage->breath / STAGE2_BREATH_MAX;
        Color breathColor = (pct > 0.3f) ? SKYBLUE : RED;
        DrawRectangle(10, 110, (int)(150 * pct), 12, breathColor);
        DrawRectangleLines(10, 110, 150, 12, WHITE);
    }

    if (stage->hasCoconutBuff) {
        char buf[32];
        sprintf(buf, "COCO! %.1fs", stage->coconutBuffTimer);
        DrawText(buf, GetScreenWidth() - 150, 90, 16, YELLOW);
    }
}