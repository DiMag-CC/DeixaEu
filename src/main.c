#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entities/player.h"
#include "menu.h"
#include "steps/stage1.h"
#include "steps/stage3.h"
#include "structure/stepList.h"
#include "utils/gameConstants.h"

#define SCREEN_WIDTH 1920.0f
#define SCREEN_HEIGHT 1080.0f
#define WORLD_WIDTH 800.0f
#define WORLD_HEIGHT 450.0f
#define CAMERA_VERTICAL_LOOKAHEAD 150.0f
#define FPS 60
#define WINDOWED_WIDTH 1280
#define WINDOWED_HEIGHT 720

typedef enum { GAME_STAGE_1 = 1, GAME_STAGE_3 = 3 } GameStage;

typedef enum {
  PAUSE_RESUME = 0,
  PAUSE_MENU = 1,
  PAUSE_RESTART = 2
} PauseOption;

static Texture2D hudHeartTexture = {0};

static Texture2D txIntroBg;
static Texture2D txIntroPlayer;
static int introTexturesLoaded = 0;

static int playerReachedStage1Exit(Player *player) {
  float visibleWidth = GetScreenWidth() / STAGE1_CAMERA_ZOOM;
  float rightLimit =
      GetScreenWidth() * 0.5f + visibleWidth * 0.5f - player->width * 0.5f;

  return player->position.x >= rightLimit - 2.0f;
}

static void skipToNextStage(Stage1 *stage1, Stage3 *stage3, Player *player,
                            int *stage1Initialized, int *stage3Initialized,
                            GameStage *activeStage, Phase **currentPhase,
                            Phase *phase3, float *bikeDropOverlayTimer,
                            int *pendingStage3Transition) {
  if (*activeStage != GAME_STAGE_1) {
    return;
  }

  int remainingLives = player->lives;

  if (*stage1Initialized) {
    unloadStage1(stage1);
    *stage1Initialized = 0;
  }

  if (*stage3Initialized) {
    unloadStage3(stage3);
  }

  initStage3(stage3, player);
  *stage3Initialized = 1;

  player->lives = remainingLives;

  *activeStage = GAME_STAGE_3;
  *currentPhase = phase3;
  *bikeDropOverlayTimer = 0.0f;
  *pendingStage3Transition = 0;

  printf("Fase atual: %s (numero %d)\n", (*currentPhase)->phaseName,
         (*currentPhase)->phaseNumber);
  fflush(stdout);
}

void drawHeartIcon(float x, float y, float size, Color color) {
  if (hudHeartTexture.id > 0) {
    Rectangle source = {0.0f, 0.0f, (float)hudHeartTexture.width,
                        (float)hudHeartTexture.height};

    Rectangle dest = {x, y, size, size};

    DrawTexturePro(hudHeartTexture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f,
                   color);

    return;
  }

  DrawCircle((int)(x + size * 0.30f), (int)(y + size * 0.30f), size * 0.22f,
             color);

  DrawCircle((int)(x + size * 0.70f), (int)(y + size * 0.30f), size * 0.22f,
             color);

  DrawTriangle((Vector2){x + size * 0.08f, y + size * 0.38f},
               (Vector2){x + size * 0.92f, y + size * 0.38f},
               (Vector2){x + size * 0.50f, y + size * 0.98f}, color);
}

void drawHudBar(float x, float y, float width, float height, float percent,
                Color fillColor) {
  if (percent < 0.0f) {
    percent = 0.0f;
  }

  if (percent > 1.0f) {
    percent = 1.0f;
  }

  Rectangle bg = {x, y, width, height};

  Rectangle fill = {x, y, width * percent, height};

  DrawRectangleRounded(bg, 0.45f, 8, (Color){28, 35, 46, 210});

  DrawRectangleRounded(fill, 0.45f, 8, fillColor);

  DrawRectangleRoundedLines(bg, 0.45f, 8, (Color){255, 255, 255, 120});
}

void drawGameHUD(Stage1 *stage, Player *player, float totalGameTime,
                 int screenWidth, int screenHeight) {
  (void)screenHeight;

  float progressPercent = stage->distanceTraveled / STAGE1_TARGET_DISTANCE;

  if (progressPercent > 1.0f) {
    progressPercent = 1.0f;
  }

  if (progressPercent < 0.0f) {
    progressPercent = 0.0f;
  }

  float panelWidth = screenWidth < 760 ? screenWidth - 24.0f : 620.0f;

  float panelHeight = 116.0f;
  float panelX = 12.0f;
  float panelY = 12.0f;

  Rectangle panel = {panelX, panelY, panelWidth, panelHeight};

  DrawRectangleRounded(panel, 0.08f, 10, (Color){7, 18, 32, 185});

  DrawRectangleRoundedLines(panel, 0.08f, 10, (Color){255, 255, 255, 95});

  DrawText("Fase 1", (int)(panelX + 18), (int)(panelY + 14), 18, RAYWHITE);

  for (int i = 0; i < 3; i++) {
    Color heartColor = (i < player->lives) ? (Color){226, 48, 70, 255}
                                           : (Color){81, 88, 101, 230};

    drawHeartIcon(panelX + 90.0f + i * 30.0f, panelY + 12.0f, 24.0f,
                  heartColor);
  }

  char scoreText[64];
  sprintf(scoreText, "%.0f pts", player->score);

  DrawText(scoreText,
           (int)(panelX + panelWidth - MeasureText(scoreText, 18) - 18),
           (int)(panelY + 14), 18, RAYWHITE);

  DrawText("Progresso", (int)(panelX + 18), (int)(panelY + 52), 14,
           (Color){215, 225, 235, 255});

  drawHudBar(panelX + 100.0f, panelY + 53.0f, panelWidth - 118.0f, 16.0f,
             progressPercent, (Color){64, 197, 112, 255});

  char timeText[64];
  sprintf(timeText, "Tempo %.1fs", totalGameTime);

  DrawText(timeText, (int)(panelX + 18), (int)(panelY + 84), 14,
           (Color){215, 225, 235, 255});

  char diffText[64];
  sprintf(diffText, "Dificuldade x%.1f", stage->difficultyMultiplier);

  DrawText(diffText, (int)(panelX + 150), (int)(panelY + 84), 14,
           (Color){215, 225, 235, 255});

  if (player->hasUmbrella > 0) {
    float umbrellaPercent = player->umbrellaTimer / 8.0f;

    DrawText("Protecao", (int)(panelX + panelWidth - 210), (int)(panelY + 84),
             14, (Color){215, 225, 235, 255});

    drawHudBar(panelX + panelWidth - 132.0f, panelY + 86.0f, 112.0f, 12.0f,
               umbrellaPercent, (Color){89, 167, 255, 255});
  }
}

void drawStage3HUD(Stage3 *stage, Player *player, float totalGameTime,
                   int screenWidth) {
  float panelWidth = screenWidth < 620 ? screenWidth - 24.0f : 500.0f;

  float panelX = 12.0f;
  float panelY = 12.0f;

  Rectangle panel = {panelX, panelY, panelWidth, 86.0f};

  DrawRectangleRounded(panel, 0.08f, 10, (Color){7, 18, 32, 185});

  DrawRectangleRoundedLines(panel, 0.08f, 10, (Color){255, 255, 255, 95});

  DrawText("Fase 3", (int)(panelX + 18), (int)(panelY + 14), 18, RAYWHITE);

  for (int i = 0; i < 3; i++) {
    Color heartColor = (i < player->lives) ? (Color){226, 48, 70, 255}
                                           : (Color){81, 88, 101, 230};

    drawHeartIcon(panelX + 90.0f + i * 30.0f, panelY + 12.0f, 24.0f,
                  heartColor);
  }

  if (stage->state == STAGE3_CLIMBING) {
    char errorText[32];

    sprintf(errorText, "Erros %d/%d", getStage3ClimbMissCount(),
            getStage3ClimbMaxMisses());

    DrawText(errorText, (int)(panelX + 190.0f), (int)(panelY + 16.0f), 16,
             (Color){255, 214, 92, 255});
  }

  char scoreText[64];
  sprintf(scoreText, "%.0f pts", player->score);

  DrawText(scoreText,
           (int)(panelX + panelWidth - MeasureText(scoreText, 18) - 18),
           (int)(panelY + 14), 18, RAYWHITE);

  char timeText[64];
  sprintf(timeText, "Tempo %.1fs", totalGameTime);

  DrawText(timeText, (int)(panelX + 18), (int)(panelY + 54), 15,
           (Color){215, 225, 235, 255});

  DrawText("Chegue ate a torre", (int)(panelX + panelWidth - 152),
           (int)(panelY + 54), 15, (Color){215, 225, 235, 255});
}

void drawPauseMenu(int selectedOption, int screenWidth, int screenHeight) {
  const char *options[] = {"Voltar", "Menu", "Reiniciar"};

  int panelWidth = 360;
  int panelHeight = 300;
  int panelX = (screenWidth - panelWidth) / 2;
  int panelY = (screenHeight - panelHeight) / 2;

  DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 145});

  DrawRectangleRounded((Rectangle){panelX, panelY, panelWidth, panelHeight},
                       0.08f, 10, (Color){8, 18, 32, 235});

  DrawRectangleRoundedLines(
      (Rectangle){panelX, panelY, panelWidth, panelHeight}, 0.08f, 10,
      (Color){255, 255, 255, 120});

  const char *title = "PAUSE";
  int titleSize = 42;

  DrawText(title, panelX + (panelWidth - MeasureText(title, titleSize)) / 2,
           panelY + 32, titleSize, YELLOW);

  for (int i = 0; i < 3; i++) {
    int fontSize = 28;
    int textWidth = MeasureText(options[i], fontSize);
    int y = panelY + 112 + i * 58;

    Rectangle hitbox = {panelX + 52.0f, y - 8.0f, panelWidth - 104.0f, 44.0f};

    if (selectedOption == i) {
      DrawRectangleRounded(hitbox, 0.25f, 8, (Color){255, 255, 255, 36});

      DrawText(">", panelX + 72, y, fontSize, YELLOW);
    }

    DrawText(options[i], panelX + (panelWidth - textWidth) / 2, y, fontSize,
             selectedOption == i ? YELLOW : RAYWHITE);
  }
}

int updatePauseMenu(int selectedOption) {
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    selectedOption--;
  }

  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    selectedOption++;
  }

  if (selectedOption < 0) {
    selectedOption = 2;
  }

  if (selectedOption > 2) {
    selectedOption = 0;
  }

  Vector2 mouse = GetMousePosition();

  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  int panelX = (screenWidth - 360) / 2;
  int panelY = (screenHeight - 300) / 2;

  for (int i = 0; i < 3; i++) {
    Rectangle hitbox = {panelX + 52.0f, panelY + 112.0f + i * 58.0f - 8.0f,
                        256.0f, 44.0f};

    if (CheckCollisionPointRec(mouse, hitbox)) {
      selectedOption = i;
    }
  }

  return selectedOption;
}

void drawBikeDropOverlay(float timer, int screenWidth, int screenHeight) {
  float alphaFactor = timer > 1.0f ? 1.0f : timer;
  unsigned char alpha = (unsigned char)(170.0f * alphaFactor);

  DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, alpha});

  const char *title = "Bicicleta deixada para tras";
  const char *subtitle = "Agora e a pe ate a torre";

  int titleSize = 38;
  int subtitleSize = 22;

  int titleX = (screenWidth - MeasureText(title, titleSize)) / 2;

  int subtitleX = (screenWidth - MeasureText(subtitle, subtitleSize)) / 2;

  int centerY = screenHeight / 2;

  DrawText(title, titleX, centerY - 42, titleSize,
           (Color){255, 238, 117, (unsigned char)(255.0f * alphaFactor)});

  DrawText(subtitle, subtitleX, centerY + 12, subtitleSize,
           (Color){235, 242, 250, (unsigned char)(240.0f * alphaFactor)});
}

void refreshStage1Layout(Stage1 *stage) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  GLOBAL_WORLD_SCALE = (float)screenHeight / BASE_SCREEN_HEIGHT;
  GLOBAL_GROUND_LEVEL = screenHeight * GROUND_Y_RATIO;

  stage->groundLevel = GLOBAL_GROUND_LEVEL;

  stage->camera.target = (Vector2){screenWidth * 0.5f, screenHeight * 0.5f};

  stage->camera.offset = (Vector2){screenWidth * 0.5f, screenHeight * 0.5f};

  stage->camera.zoom = STAGE1_CAMERA_ZOOM;
}

void centerWindowOnCurrentMonitor(int width, int height) {
  int monitor = GetCurrentMonitor();

  int monitorWidth = GetMonitorWidth(monitor);
  int monitorHeight = GetMonitorHeight(monitor);

  int monitorX = GetMonitorPosition(monitor).x;
  int monitorY = GetMonitorPosition(monitor).y;

  SetWindowPosition(monitorX + (monitorWidth - width) / 2,
                    monitorY + (monitorHeight - height) / 2);
}

void toggleGameFullscreen(void) {
  if (!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) {
    int monitor = GetCurrentMonitor();

    int monitorX = (int)GetMonitorPosition(monitor).x;
    int monitorY = (int)GetMonitorPosition(monitor).y;

    SetWindowPosition(monitorX, monitorY);
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));

    SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
  } else {
    ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    SetWindowSize(WINDOWED_WIDTH, WINDOWED_HEIGHT);
    centerWindowOnCurrentMonitor(WINDOWED_WIDTH, WINDOWED_HEIGHT);
  }
}

void drawPlayerDebug(Player player) {
  DrawRectangleLinesEx(player.hitbox, 1, RED);
  DrawCircle((int)player.position.x, (int)player.position.y, 3, GREEN);

  if (player.velocity.x != 0 || player.velocity.y != 0) {
    Vector2 velocityEnd = {player.position.x + player.velocity.x * 10,
                           player.position.y + player.velocity.y * 10};

    DrawLineEx(player.position, velocityEnd, 2, YELLOW);
  }

  char debugText[256];

  sprintf(debugText,
          "Player: (%.0f, %.0f) | Vel: (%.1f, %.1f) | Speed: %.0f | Lives: %d",
          player.position.x, player.position.y, player.velocity.x,
          player.velocity.y, player.speed, player.lives);

  DrawText(debugText, 10, 80, 14, BLACK);
}

void drawStoryIntroScreen(float storyTime, int screenWidth, int screenHeight) {
  if (!introTexturesLoaded) {
    txIntroBg = LoadTexture("assets/img/landscapeFase1New2.png");
    txIntroPlayer = LoadTexture("assets/img/CharacterStandingR.png");
    introTexturesLoaded = 1;
  }

  if (txIntroBg.id > 0) {
    DrawTexturePro(
        txIntroBg,
        (Rectangle){0, 0, (float)txIntroBg.width, (float)txIntroBg.height},
        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
        (Vector2){0, 0}, 0.0f, (Color){110, 110, 125, 255});
  } else {
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){15, 20, 35, 255});
  }

  if (txIntroPlayer.id > 0) {
    float scaleStretch = sinf(storyTime * 3.5f) * 4.0f;

    float pWidth = 140.0f;
    float pHeight = 175.0f + scaleStretch;

    Vector2 pPos = {60.0f, (float)screenHeight * 0.86f - (pHeight / 2.0f)};

    DrawTexturePro(txIntroPlayer,
                   (Rectangle){0, 0, (float)txIntroPlayer.width,
                               (float)txIntroPlayer.height},
                   (Rectangle){pPos.x, pPos.y, pWidth, pHeight},
                   (Vector2){0, 0}, 0.0f, WHITE);
  }

  int boxX = 260;
  int boxWidth = screenWidth - 320;
  int boxHeight = 130;
  int textPaddingX = 30;
  int textPaddingY = 25;

  const char *txt1_A =
      "Minha mãe sempre diz o que eu devo fazer, para onde devo ir...";

  const char *txt1_B = "\"Não saia na chuva\", \"A cidade é perigosa\".";

  const char *txt2_A =
      "Mas eu cansei de apenas assistir à vida passar pela janela.";

  const char *txt2_B =
      "Hoje eu vou descobrir o recife por conta própria. DEIXA EU!";

  const char *txt3_A =
      "Lá fora, a tempestade urbana está mais forte e poluída do que nunca.";

  const char *txt3_B = "Para conquistar minha liberdade, precisarei ser mais "
                       "rápido que o trânsito.";

  const char *txt4_A =
      "Minha jornada me levará além do asfalto, cruzando as praias";

  const char *txt4_B = "e mergulhando nas profundezas de um oceano poluído. A "
                       "aventura começa agora...";

  char bufferA[256];
  char bufferB[256];

  if (storyTime >= 0.0f) {
    float localTime = storyTime - 0.0f;
    float alphaProgress = localTime * 2.0f;

    if (alphaProgress > 1.0f) {
      alphaProgress = 1.0f;
    }

    unsigned char alphaByte = (unsigned char)(alphaProgress * 220);

    int boxY = 60 + (int)((1.0f - alphaProgress) * 15.0f);

    DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4,
                         (Color){40, 55, 75, alphaByte});

    DrawRectangleRoundedLinesEx(
        (Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4, 3.0f,
        (Color){102, 191, 255, (unsigned char)(alphaProgress * 255)});

    int charsToDrawA = (int)(localTime * 45.0f);
    int lenA = strlen(txt1_A);

    if (charsToDrawA > lenA) {
      charsToDrawA = lenA;
    }

    strncpy(bufferA, txt1_A, charsToDrawA);
    bufferA[charsToDrawA] = '\0';

    int charsToDrawB = (int)((localTime - 1.0f) * 45.0f);

    if (charsToDrawB < 0) {
      charsToDrawB = 0;
    }

    int lenB = strlen(txt1_B);

    if (charsToDrawB > lenB) {
      charsToDrawB = lenB;
    }

    strncpy(bufferB, txt1_B, charsToDrawB);
    bufferB[charsToDrawB] = '\0';

    DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26,
             (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});

    if (charsToDrawB > 0) {
      DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26,
               (Color){200, 200, 200, (unsigned char)(alphaProgress * 255)});
    }
  }

  if (storyTime >= 4.0f) {
    float localTime = storyTime - 4.0f;
    float alphaProgress = localTime * 2.0f;

    if (alphaProgress > 1.0f) {
      alphaProgress = 1.0f;
    }

    unsigned char alphaByte = (unsigned char)(alphaProgress * 220);

    int boxY = 220 + (int)((1.0f - alphaProgress) * 15.0f);

    DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4,
                         (Color){50, 50, 50, alphaByte});

    DrawRectangleRoundedLinesEx(
        (Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4, 3.0f,
        (Color){253, 249, 0, (unsigned char)(alphaProgress * 255)});

    int charsToDrawA = (int)(localTime * 45.0f);
    int lenA = strlen(txt2_A);

    if (charsToDrawA > lenA) {
      charsToDrawA = lenA;
    }

    strncpy(bufferA, txt2_A, charsToDrawA);
    bufferA[charsToDrawA] = '\0';

    int charsToDrawB = (int)((localTime - 1.0f) * 45.0f);

    if (charsToDrawB < 0) {
      charsToDrawB = 0;
    }

    int lenB = strlen(txt2_B);

    if (charsToDrawB > lenB) {
      charsToDrawB = lenB;
    }

    strncpy(bufferB, txt2_B, charsToDrawB);
    bufferB[charsToDrawB] = '\0';

    DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26,
             (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});

    if (charsToDrawB > 0) {
      DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26,
               (Color){249, 215, 0, (unsigned char)(alphaProgress * 255)});
    }
  }

  if (storyTime >= 9.0f) {
    float localTime = storyTime - 9.0f;
    float alphaProgress = localTime * 2.0f;

    if (alphaProgress > 1.0f) {
      alphaProgress = 1.0f;
    }

    unsigned char alphaByte = (unsigned char)(alphaProgress * 220);

    int boxY = 380 + (int)((1.0f - alphaProgress) * 15.0f);

    DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4,
                         (Color){70, 45, 30, alphaByte});

    DrawRectangleRoundedLinesEx(
        (Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4, 3.0f,
        (Color){255, 161, 0, (unsigned char)(alphaProgress * 255)});

    int charsToDrawA = (int)(localTime * 45.0f);
    int lenA = strlen(txt3_A);

    if (charsToDrawA > lenA) {
      charsToDrawA = lenA;
    }

    strncpy(bufferA, txt3_A, charsToDrawA);
    bufferA[charsToDrawA] = '\0';

    int charsToDrawB = (int)((localTime - 1.2f) * 45.0f);

    if (charsToDrawB < 0) {
      charsToDrawB = 0;
    }

    int lenB = strlen(txt3_B);

    if (charsToDrawB > lenB) {
      charsToDrawB = lenB;
    }

    strncpy(bufferB, txt3_B, charsToDrawB);
    bufferB[charsToDrawB] = '\0';

    DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26,
             (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});

    if (charsToDrawB > 0) {
      DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26,
               (Color){200, 200, 200, (unsigned char)(alphaProgress * 255)});
    }
  }

  if (storyTime >= 14.0f) {
    float localTime = storyTime - 14.0f;
    float alphaProgress = localTime * 2.0f;

    if (alphaProgress > 1.0f) {
      alphaProgress = 1.0f;
    }

    unsigned char alphaByte = (unsigned char)(alphaProgress * 220);

    int boxY = 540 + (int)((1.0f - alphaProgress) * 15.0f);

    DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4,
                         (Color){35, 60, 45, alphaByte});

    DrawRectangleRoundedLinesEx(
        (Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.15f, 4, 3.0f,
        (Color){0, 228, 48, (unsigned char)(alphaProgress * 255)});

    int charsToDrawA = (int)(localTime * 45.0f);
    int lenA = strlen(txt4_A);

    if (charsToDrawA > lenA) {
      charsToDrawA = lenA;
    }

    strncpy(bufferA, txt4_A, charsToDrawA);
    bufferA[charsToDrawA] = '\0';

    int charsToDrawB = (int)((localTime - 1.2f) * 45.0f);

    if (charsToDrawB < 0) {
      charsToDrawB = 0;
    }

    int lenB = strlen(txt4_B);

    if (charsToDrawB > lenB) {
      charsToDrawB = lenB;
    }

    strncpy(bufferB, txt4_B, charsToDrawB);
    bufferB[charsToDrawB] = '\0';

    DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26,
             (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});

    if (charsToDrawB > 0) {
      DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26,
               (Color){0, 228, 48, (unsigned char)(alphaProgress * 255)});
    }
  }

  float pulseBtn = (sinf(storyTime * 4.0f) + 1.0f) / 2.0f;

  DrawRectangleRounded((Rectangle){screenWidth - 280, 30, 250, 45}, 0.2f, 4,
                       (Color){20, 20, 20, 180});

  DrawText("Pular Cena (ENTER)", screenWidth - 240, 42, 18,
           (Color){255, 255, 255, (unsigned char)(200 + pulseBtn * 55)});

  float timeProgress = storyTime / 20.0f;

  if (timeProgress > 1.0f) {
    timeProgress = 1.0f;
  }

  DrawText("PROGRESSO DA INTRODUÇÃO:", screenWidth - 360, screenHeight - 95, 16,
           LIGHTGRAY);

  DrawRectangle(screenWidth - 360, screenHeight - 70, 320, 20, DARKGRAY);

  DrawRectangle(screenWidth - 360, screenHeight - 70, (int)(320 * timeProgress),
                20, GREEN);

  DrawRectangleLines(screenWidth - 360, screenHeight - 70, 320, 20, WHITE);
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(WINDOWED_WIDTH, WINDOWED_HEIGHT, "Deixa Eu");
  InitAudioDevice();
  SetExitKey(KEY_NULL);
  toggleGameFullscreen();

  SetTargetFPS(FPS);

  Stage1 stage1;
  initStage1(&stage1);
  int stage1Initialized = 1;

  Stage3 stage3 = {0};
  int stage3Initialized = 0;

  int screenWidth = GetScreenWidth();

  Player player =
      createPlayer((Vector2){screenWidth * 0.18f, GROUND_LEVEL}, 150, 3);

  hudHeartTexture = LoadTexture("assets/img/HealthHeart.png");

  int isGameOver = 0;
  float gameOverTimer = 0.0f;
  float totalGameTime = 0.0f;
  int debugMode = 0;

  GameStage activeStage = GAME_STAGE_1;

  int isPaused = 0;
  int pauseSelectedOption = PAUSE_RESUME;

  float bikeDropOverlayTimer = 0.0f;
  int pendingStage3Transition = 0;

  Menu menu = createMenu();

  int exibindoIntro = 1;
  int inMenu = 0;

  float storyTimer = 0.0f;

  Phase *phaseList = NULL;
  Phase *phase1 = createPhase(1, "Recife Chuvoso");
  Phase *phase3 = createPhase(3, "Torre Final");

  insertPhase(&phaseList, phase1);
  insertPhase(&phaseList, phase3);

  Phase *currentPhase = phase1;

  printf("Fase atual: %s (numero %d)\n", currentPhase->phaseName,
         currentPhase->phaseNumber);

  fflush(stdout);

  while (!WindowShouldClose()) {
    float deltaTime = GetFrameTime();

    if (IsKeyPressed(KEY_F11) ||
        (IsKeyPressed(KEY_F) && IsKeyDown(KEY_LEFT_ALT))) {
      toggleGameFullscreen();
    }

    if (IsWindowResized() || IsKeyPressed(KEY_F11) ||
        (IsKeyPressed(KEY_F) && IsKeyDown(KEY_LEFT_ALT))) {
      if (stage1Initialized) {
        refreshStage1Layout(&stage1);
      }

      if (activeStage == GAME_STAGE_1) {
        player.position.x = GetScreenWidth() * 0.18f;
      }
    }

    if (IsKeyPressed(KEY_F3)) {
      debugMode = !debugMode;
    }

    if (exibindoIntro) {
      storyTimer += deltaTime;

      if (storyTimer >= 20.0f || IsKeyPressed(KEY_ENTER)) {
        exibindoIntro = 0;
        inMenu = 1;

        if (introTexturesLoaded) {
          UnloadTexture(txIntroBg);
          UnloadTexture(txIntroPlayer);
          introTexturesLoaded = 0;
        }
      }
    } else if (inMenu) {
      updateMenu(&menu);

      if (menu.screen == MENU_MAIN && menu.confirmPressed) {
        if (menu.selectedOption == 0) {
          inMenu = 0;
          totalGameTime = 0.0f;
          isGameOver = 0;
          isPaused = 0;
          bikeDropOverlayTimer = 0.0f;
          pendingStage3Transition = 0;
          activeStage = GAME_STAGE_1;
          currentPhase = phase1;

          if (stage3Initialized) {
            unloadStage3(&stage3);
            stage3Initialized = 0;
          }

          if (stage1Initialized) {
            unloadStage1(&stage1);
          }

          initStage1(&stage1);
          stage1Initialized = 1;

          int currentScreenWidth = GetScreenWidth();

          unloadPlayerResources(&player);

          player = createPlayer(
              (Vector2){currentScreenWidth * 0.18f, GROUND_LEVEL}, 150, 3);
        } else if (menu.selectedOption == 2) {
          break;
        }
      }
    } else {
      if (IsKeyPressed(KEY_ESCAPE) && !isGameOver &&
          !(activeStage == GAME_STAGE_3 && stage3.state == STAGE3_FINISHED)) {
        isPaused = !isPaused;
        pauseSelectedOption = PAUSE_RESUME;
      }

      if (isPaused) {
        pauseSelectedOption = updatePauseMenu(pauseSelectedOption);

        bool clickedPauseOption = false;
        Vector2 mouse = GetMousePosition();

        int panelX = (GetScreenWidth() - 360) / 2;
        int panelY = (GetScreenHeight() - 300) / 2;

        for (int i = 0; i < 3; i++) {
          Rectangle hitbox = {panelX + 52.0f,
                              panelY + 112.0f + i * 58.0f - 8.0f, 256.0f,
                              44.0f};

          if (CheckCollisionPointRec(mouse, hitbox) &&
              IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            clickedPauseOption = true;
          }
        }

        if (IsKeyPressed(KEY_ENTER) || clickedPauseOption) {
          if (pauseSelectedOption == PAUSE_RESUME) {
            isPaused = 0;
          } else {
            unloadPlayerResources(&player);

            player = createPlayer(
                (Vector2){GetScreenWidth() * 0.18f, GROUND_LEVEL}, 150, 3);

            if (stage1Initialized) {
              unloadStage1(&stage1);
            }

            if (stage3Initialized) {
              unloadStage3(&stage3);
              stage3Initialized = 0;
            }

            initStage1(&stage1);
            stage1Initialized = 1;

            activeStage = GAME_STAGE_1;
            currentPhase = phase1;

            isGameOver = 0;
            totalGameTime = 0.0f;
            bikeDropOverlayTimer = 0.0f;
            pendingStage3Transition = 0;
            isPaused = 0;

            if (pauseSelectedOption == PAUSE_MENU) {
              menu = createMenu();
              inMenu = 1;
            }
          }
        }
      }

      if (!isPaused && !inMenu) {
        if (!isGameOver && IsKeyPressed(KEY_P)) {
          skipToNextStage(&stage1, &stage3, &player, &stage1Initialized,
                          &stage3Initialized, &activeStage, &currentPhase,
                          phase3, &bikeDropOverlayTimer,
                          &pendingStage3Transition);
        }

        if (activeStage == GAME_STAGE_1) {
          updateStage1(&stage1, &player, deltaTime);
          updatePlayer(&player, deltaTime);

          if (stage1.stage1Complete &&
              stage1.distanceTraveled >= STAGE1_TARGET_DISTANCE &&
              playerReachedStage1Exit(&player) && !isGameOver &&
              !pendingStage3Transition) {
            pendingStage3Transition = 1;
            bikeDropOverlayTimer = 2.2f;
            player.velocity = (Vector2){0.0f, 0.0f};
          }

          if (pendingStage3Transition) {
            if (bikeDropOverlayTimer > 0.0f) {
              bikeDropOverlayTimer -= deltaTime;
            }

            if (bikeDropOverlayTimer <= 0.0f) {
              int remainingLives = player.lives;

              unloadStage1(&stage1);
              stage1Initialized = 0;

              initStage3(&stage3, &player);
              stage3Initialized = 1;

              player.lives = remainingLives;

              activeStage = GAME_STAGE_3;
              currentPhase = phase3;

              bikeDropOverlayTimer = 0.0f;
              pendingStage3Transition = 0;

              printf("Fase atual: %s (numero %d)\n", currentPhase->phaseName,
                     currentPhase->phaseNumber);

              fflush(stdout);
            }
          }
        } else if (activeStage == GAME_STAGE_3) {
          updateStage3(&stage3, &player, deltaTime);
        }

        if (bikeDropOverlayTimer > 0.0f && !pendingStage3Transition) {
          bikeDropOverlayTimer -= deltaTime;

          if (bikeDropOverlayTimer < 0.0f) {
            bikeDropOverlayTimer = 0.0f;
          }
        }

        if (player.lives <= 0 && !isGameOver) {
          isGameOver = 1;
          gameOverTimer = 3.0f;
        }

        if (isGameOver) {
          gameOverTimer -= deltaTime;

          if (IsKeyPressed(KEY_ENTER) || gameOverTimer <= 0) {
            int resetScreenWidth = GetScreenWidth();

            unloadPlayerResources(&player);

            player = createPlayer(
                (Vector2){resetScreenWidth * 0.18f, GROUND_LEVEL}, 150, 3);

            if (stage1Initialized) {
              unloadStage1(&stage1);
            }

            if (stage3Initialized) {
              unloadStage3(&stage3);
              stage3Initialized = 0;
            }

            initStage1(&stage1);
            stage1Initialized = 1;

            activeStage = GAME_STAGE_1;
            currentPhase = phase1;

            isGameOver = 0;
            totalGameTime = 0.0f;
            isPaused = 0;
            bikeDropOverlayTimer = 0.0f;
            pendingStage3Transition = 0;

            menu = createMenu();
            inMenu = 1;
          }
        }

        if (activeStage == GAME_STAGE_3 && stage3.state == STAGE3_FINISHED &&
            !isGameOver) {
          if (IsKeyPressed(KEY_ENTER)) {
            int victoryScreenWidth = GetScreenWidth();

            unloadPlayerResources(&player);

            player = createPlayer(
                (Vector2){victoryScreenWidth * 0.18f, GROUND_LEVEL}, 150, 3);

            if (stage1Initialized) {
              unloadStage1(&stage1);
            }

            if (stage3Initialized) {
              unloadStage3(&stage3);
              stage3Initialized = 0;
            }

            initStage1(&stage1);
            stage1Initialized = 1;

            activeStage = GAME_STAGE_1;
            currentPhase = phase1;

            totalGameTime = 0.0f;
            isPaused = 0;
            bikeDropOverlayTimer = 0.0f;
            pendingStage3Transition = 0;

            menu = createMenu();
            inMenu = 1;
          }
        }

        if (!isGameOver &&
            !(activeStage == GAME_STAGE_3 && stage3.state == STAGE3_FINISHED)) {
          totalGameTime += deltaTime;
        }
      }
    }

    BeginDrawing();

    int currentScreenWidth = GetScreenWidth();
    int currentScreenHeight = GetScreenHeight();

    if (exibindoIntro) {
      ClearBackground((Color){11, 16, 27, 255});
    } else if (inMenu) {
      ClearBackground((Color){164, 88, 48, 255});
    } else if (activeStage == GAME_STAGE_3) {
      ClearBackground((Color){176, 96, 48, 255});
    } else {
      ClearBackground(SKYBLUE);
    }

    if (exibindoIntro) {
      drawStoryIntroScreen(storyTimer, currentScreenWidth, currentScreenHeight);
    } else if (inMenu) {
      drawMenu(menu);
    } else {
      if (activeStage == GAME_STAGE_1) {
        drawStage1(&stage1, &player);

        drawGameHUD(&stage1, &player, totalGameTime, currentScreenWidth,
                    currentScreenHeight);
      } else if (activeStage == GAME_STAGE_3) {
        drawStage3(&stage3, &player);

        drawStage3HUD(&stage3, &player, totalGameTime, currentScreenWidth);
      }

      if (bikeDropOverlayTimer > 0.0f) {
        drawBikeDropOverlay(bikeDropOverlayTimer, currentScreenWidth,
                            currentScreenHeight);
      }

      if (isPaused) {
        drawPauseMenu(pauseSelectedOption, currentScreenWidth,
                      currentScreenHeight);
      }

      if (debugMode) {
        drawPlayerDebug(player);
        DrawText("DEBUG MODE (F3 para desativar)", 10, 30, 14, RED);
        DrawFPS(10, currentScreenHeight - 30);
      }

      if (isGameOver) {
        DrawRectangle(0, 0, currentScreenWidth, currentScreenHeight,
                      (Color){0, 0, 0, 180});

        const char *gameOverText = "GAME OVER";
        int textWidth = MeasureText(gameOverText, 60);

        DrawText(gameOverText, (currentScreenWidth - textWidth) / 2,
                 (int)(currentScreenHeight * 0.26f), 60, RED);

        char finalScoreText[128];

        sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score,
                totalGameTime);

        textWidth = MeasureText(finalScoreText, 20);

        DrawText(finalScoreText, (currentScreenWidth - textWidth) / 2,
                 (int)(currentScreenHeight * 0.44f), 20, WHITE);

        const char *restartText = "Pressione ENTER para voltar ao menu";

        textWidth = MeasureText(restartText, 16);

        DrawText(restartText, (currentScreenWidth - textWidth) / 2,
                 (int)(currentScreenHeight * 0.56f), 16, WHITE);

        if (gameOverTimer > 0) {
          char timerText[64];

          sprintf(timerText, "Reiniciando em %.1f segundos", gameOverTimer);

          textWidth = MeasureText(timerText, 14);

          DrawText(timerText, (currentScreenWidth - textWidth) / 2,
                   (int)(currentScreenHeight * 0.67f), 14, YELLOW);
        }
      }

      if (activeStage == GAME_STAGE_3 && stage3.state == STAGE3_FINISHED &&
          !isGameOver) {
        DrawRectangle(0, 0, currentScreenWidth, currentScreenHeight,
                      (Color){0, 0, 0, 180});

        const char *victoryText = "VITÓRIA!";
        int textWidth = MeasureText(victoryText, 60);

        DrawText(victoryText, (currentScreenWidth - textWidth) / 2,
                 (int)(currentScreenHeight * 0.26f), 60, GREEN);

        char finalScoreText[128];

        sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score,
                totalGameTime);

        textWidth = MeasureText(finalScoreText, 20);

        DrawText(finalScoreText, (currentScreenWidth - textWidth) / 2,
                 (int)(currentScreenHeight * 0.44f), 20, WHITE);

        const char *continueText = "Pressione ENTER para voltar ao menu";

        textWidth = MeasureText(continueText, 16);

        DrawText(continueText, (currentScreenWidth - textWidth) / 2,
                 (int)(currentScreenHeight * 0.56f), 16, WHITE);
      }
    }

    EndDrawing();
  }

  if (introTexturesLoaded) {
    UnloadTexture(txIntroBg);
    UnloadTexture(txIntroPlayer);
  }

  if (stage1Initialized) {
    unloadStage1(&stage1);
  }

  if (stage3Initialized) {
    unloadStage3(&stage3);
  }

  unloadPlayerResources(&player);

  if (phaseList != NULL) {
    freePhaseList(phaseList);
  }

  if (hudHeartTexture.id > 0) {
    UnloadTexture(hudHeartTexture);
  }

  CloseAudioDevice();
  CloseWindow();

  return 0;
}