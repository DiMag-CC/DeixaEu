#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steps/stage1.h"
#include "steps/stage3.h"
#include "entities/player.h"
#include "menu.h"
#include "structure/stepList.h"
#include <math.h>

#define SCREEN_WIDTH 1920.0f
#define SCREEN_HEIGHT 1080.0f
#define WORLD_WIDTH 800.0f
#define WORLD_HEIGHT 450.0f
#define CAMERA_VERTICAL_LOOKAHEAD 150.0f
#define FPS 60

// Texturas exclusivas da introdução narrativa
static Texture2D txIntroBg;
static Texture2D txIntroPlayer;
static int introTexturesLoaded = 0;

void drawGameHUD(Stage1 *stage, Player *player, float totalGameTime, int screenWidth, int screenHeight) {
    const int HUD_Y_START = 12;
    const int HUD_Y_STEP = 25;
    const int HUD_MARGIN = 12;

    // ===== FONTE DINÂMICA BASEADA EM RESOLUÇÃO =====
    int fontSize = (screenWidth < 1024) ? 14 : 16;

    // ===== VIDAS =====
    char livesText[64];
    sprintf(livesText, "Vidas: %d / 3", player->lives);
    DrawText(livesText, HUD_MARGIN, HUD_Y_START, fontSize, BLACK);

    // ===== PONTOS =====
    char scoreText[64];
    sprintf(scoreText, "Pontos: %.0f", player->score);
    DrawText(scoreText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP, fontSize, BLACK);

    // ===== TEMPO =====
    char timeText[64];
    sprintf(timeText, "Tempo: %.1f s", totalGameTime);
    DrawText(timeText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP * 2, fontSize, BLACK);

    // ===== DIFICULDADE =====
    char diffText[64];
    sprintf(diffText, "Dificuldade: x%.1f", stage->difficultyMultiplier);
    DrawText(diffText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP * 3, fontSize, DARKBLUE);

    // ===== PROTEÇÃO DE GUARDA-CHUVA =====
    if (player->hasUmbrella > 0) {
        char protectionText[64];
        sprintf(protectionText, "Protecao: %.1f s", player->umbrellaTimer);
        int protectionX = screenWidth - 250;
        DrawText(protectionText, protectionX, HUD_Y_START, 16, GREEN);

        int barWidth = 150;
        float barProgress = player->umbrellaTimer / 5.0f;
        if (barProgress > 1.0f) barProgress = 1.0f;

        DrawRectangle(protectionX, HUD_Y_START + 25, barWidth, 10, LIGHTGRAY);
        DrawRectangle(protectionX, HUD_Y_START + 25, (int)(barWidth * barProgress), 10, GREEN);
        DrawRectangleLinesEx((Rectangle){(float)protectionX, (float)HUD_Y_START + 25, (float)barWidth, 10.0f}, 1, BLACK);
    }

    // ===== PROGRESSO DA FASE =====
    float progressPercent = stage->distanceTraveled / STAGE1_TARGET_DISTANCE;
    if (progressPercent > 1.0f) progressPercent = 1.0f;

    int progressBarY = screenHeight - 40;
    int progressBarWidth = screenWidth - 20;
    int progressBarHeight = 20;

    char progressText[64];
    sprintf(progressText, "Progresso: %.0f / %.0f m", stage->distanceTraveled, STAGE1_TARGET_DISTANCE);
    DrawText(progressText, HUD_MARGIN, progressBarY - 25, 14, BLACK);

    DrawRectangle(HUD_MARGIN, progressBarY, progressBarWidth, progressBarHeight, LIGHTGRAY);
    DrawRectangle(HUD_MARGIN, progressBarY, (int)(progressBarWidth * progressPercent), progressBarHeight, GREEN);
    DrawRectangleLinesEx((Rectangle){(float)HUD_MARGIN, (float)progressBarY, (float)progressBarWidth, (float)progressBarHeight}, 2, BLACK);
}

void drawPlayerDebug(Player player) {
    DrawRectangleLinesEx(player.hitbox, 1, RED);
    DrawCircle((int)player.position.x, (int)player.position.y, 3, GREEN);

    if (player.velocity.x != 0 || player.velocity.y != 0) {
        Vector2 velocityEnd = {
            player.position.x + player.velocity.x * 10,
            player.position.y + player.velocity.y * 10
        };
        DrawLineEx(player.position, velocityEnd, 2, YELLOW);
    }

    char debugText[256];
    sprintf(debugText,
            "Player: (%.0f, %.0f) | Vel: (%.1f, %.1f) | Speed: %.0f | Lives: %d",
            player.position.x, player.position.y,
            player.velocity.x, player.velocity.y,
            player.speed, player.lives);
    DrawText(debugText, 10, 80, 14, BLACK);
}

// =========================================================================
// RENDERIZADOR DA CENA CINEMATOGRÁFICA DE INTRODUÇÃO (ABSOLUTO TELA CHEIA)
// =========================================================================
void drawStoryIntroScreen(float storyTime, int screenWidth, int screenHeight) {
    if (!introTexturesLoaded) {
        txIntroBg = LoadTexture("assets/img/landscapeFase1New2.png");
        txIntroPlayer = LoadTexture("assets/img/CharacterStandingR.png");
        introTexturesLoaded = 1;
    }

    if (txIntroBg.id > 0) {
        DrawTexturePro(txIntroBg, 
                       (Rectangle){0, 0, (float)txIntroBg.width, (float)txIntroBg.height},
                       (Rectangle){0, 0, (float)screenWidth, (float)screenHeight}, 
                       (Vector2){0,0}, 0.0f, (Color){110, 110, 125, 255});
    } else {
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){15, 20, 35, 255});
    }

    if (txIntroPlayer.id > 0) {
        float scaleStretch = sinf(storyTime * 3.5f) * 4.0f; 
        float pWidth = 140.0f;
        float pHeight = 175.0f + scaleStretch; 
        Vector2 pPos = { 60.0f, (float)screenHeight * 0.86f - (pHeight / 2.0f) };
        
        DrawTexturePro(txIntroPlayer,
                       (Rectangle){0, 0, (float)txIntroPlayer.width, (float)txIntroPlayer.height},
                       (Rectangle){pPos.x, pPos.y, pWidth, pHeight},
                       (Vector2){0,0}, 0.0f, WHITE);
    }

    int boxX = 260; int boxWidth = screenWidth - 320; int boxHeight = 130;
    int textPaddingX = 30; int textPaddingY = 25;

    const char *txt1_A = "Minha mãe sempre diz o que eu devo fazer, para onde devo ir...";
    const char *txt1_B = "\"Não saia na chuva\", \"A cidade é perigosa\".";
    const char *txt2_A = "Mas eu cansei de apenas assistir à vida passar pela janela.";
    const char *txt2_B = "Hoje eu vou descobrir o recife por conta própria. DEIXA EU!";
    const char *txt3_A = "Lá fora, a tempestade urbana está mais forte e poluída do que nunca.";
    const char *txt3_B = "Para conquistar minha liberdade, precisarei ser mais rápido que o trânsito.";
    const char *txt4_A = "Minha jornada me levará além do asfalto, cruzando as praias";
    const char *txt4_B = "e mergulhando nas profundezas de um oceano poluído. A aventura começa agora...";

    char bufferA[256]; char bufferB[256];

    if (storyTime >= 0.0f) {
        float localTime = storyTime - 0.0f; float alphaProgress = localTime * 2.0f; 
        if (alphaProgress > 1.0f) alphaProgress = 1.0f;
        unsigned char alphaByte = (unsigned char)(alphaProgress * 220);
        int boxY = 60 + (int)((1.0f - alphaProgress) * 15.0f);
        DrawRectangleRounded((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, (Color){40, 55, 75, alphaByte});
        DrawRectangleRoundedLinesEx((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, 3.0f, (Color){102, 191, 255, (unsigned char)(alphaProgress * 255)});
        int charsToDrawA = (int)(localTime * 45.0f); int lenA = strlen(txt1_A); if (charsToDrawA > lenA) charsToDrawA = lenA;
        strncpy(bufferA, txt1_A, charsToDrawA); bufferA[charsToDrawA] = '\0';
        int charsToDrawB = (int)((localTime - 1.0f) * 45.0f); if (charsToDrawB < 0) charsToDrawB = 0;
        int lenB = strlen(txt1_B); if (charsToDrawB > lenB) charsToDrawB = lenB;
        strncpy(bufferB, txt1_B, charsToDrawB); bufferB[charsToDrawB] = '\0';
        DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26, (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});
        if (charsToDrawB > 0) DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26, (Color){200, 200, 200, (unsigned char)(alphaProgress * 255)});
    }

    if (storyTime >= 4.0f) {
        float localTime = storyTime - 4.0f; float alphaProgress = localTime * 2.0f;
        if (alphaProgress > 1.0f) alphaProgress = 1.0f;
        unsigned char alphaByte = (unsigned char)(alphaProgress * 220);
        int boxY = 220 + (int)((1.0f - alphaProgress) * 15.0f);
        DrawRectangleRounded((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, (Color){50, 50, 50, alphaByte});
        DrawRectangleRoundedLinesEx((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, 3.0f, (Color){253, 249, 0, (unsigned char)(alphaProgress * 255)});
        int charsToDrawA = (int)(localTime * 45.0f); int lenA = strlen(txt2_A); if (charsToDrawA > lenA) charsToDrawA = lenA;
        strncpy(bufferA, txt2_A, charsToDrawA); bufferA[charsToDrawA] = '\0';
        int charsToDrawB = (int)((localTime - 1.0f) * 45.0f); if (charsToDrawB < 0) charsToDrawB = 0;
        int lenB = strlen(txt2_B); if (charsToDrawB > lenB) charsToDrawB = lenB;
        strncpy(bufferB, txt2_B, charsToDrawB); bufferB[charsToDrawB] = '\0';
        DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26, (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});
        if (charsToDrawB > 0) DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26, (Color){249, 215, 0, (unsigned char)(alphaProgress * 255)});
    }

    if (storyTime >= 9.0f) {
        float localTime = storyTime - 9.0f; float alphaProgress = localTime * 2.0f;
        if (alphaProgress > 1.0f) alphaProgress = 1.0f;
        unsigned char alphaByte = (unsigned char)(alphaProgress * 220);
        int boxY = 380 + (int)((1.0f - alphaProgress) * 15.0f);
        DrawRectangleRounded((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, (Color){70, 45, 30, alphaByte});
        DrawRectangleRoundedLinesEx((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, 3.0f, (Color){255, 161, 0, (unsigned char)(alphaProgress * 255)});
        int charsToDrawA = (int)(localTime * 45.0f); int lenA = strlen(txt3_A); if (charsToDrawA > lenA) charsToDrawA = lenA;
        strncpy(bufferA, txt3_A, charsToDrawA); bufferA[charsToDrawA] = '\0';
        int charsToDrawB = (int)((localTime - 1.2f) * 45.0f); if (charsToDrawB < 0) charsToDrawB = 0;
        int lenB = strlen(txt3_B); if (charsToDrawB > lenB) charsToDrawB = lenB;
        strncpy(bufferB, txt3_B, charsToDrawB); bufferB[charsToDrawB] = '\0';
        DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26, (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});
        if (charsToDrawB > 0) DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26, (Color){200, 200, 200, (unsigned char)(alphaProgress * 255)});
    }

    if (storyTime >= 14.0f) {
        float localTime = storyTime - 14.0f; float alphaProgress = localTime * 2.0f;
        if (alphaProgress > 1.0f) alphaProgress = 1.0f;
        unsigned char alphaByte = (unsigned char)(alphaProgress * 220);
        int boxY = 540 + (int)((1.0f - alphaProgress) * 15.0f);
        DrawRectangleRounded((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, (Color){35, 60, 45, alphaByte});
        DrawRectangleRoundedLinesEx((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight}, 0.15f, 4, 3.0f, (Color){0, 228, 48, (unsigned char)(alphaProgress * 255)});
        int charsToDrawA = (int)(localTime * 45.0f); int lenA = strlen(txt4_A); if (charsToDrawA > lenA) charsToDrawA = lenA;
        strncpy(bufferA, txt4_A, charsToDrawA); bufferA[charsToDrawA] = '\0';
        int charsToDrawB = (int)((localTime - 1.2f) * 45.0f); if (charsToDrawB < 0) charsToDrawB = 0;
        int lenB = strlen(txt4_B); if (charsToDrawB > lenB) charsToDrawB = lenB;
        strncpy(bufferB, txt4_B, charsToDrawB); bufferB[charsToDrawB] = '\0';
        DrawText(bufferA, boxX + textPaddingX, boxY + textPaddingY, 26, (Color){255, 255, 255, (unsigned char)(alphaProgress * 255)});
        if (charsToDrawB > 0) DrawText(bufferB, boxX + textPaddingX, boxY + textPaddingY + 40, 26, (Color){0, 228, 48, (unsigned char)(alphaProgress * 255)});
    }

    float pulseBtn = (sinf(storyTime * 4.0f) + 1.0f) / 2.0f;
    DrawRectangleRounded((Rectangle){(float)(screenWidth - 280), 30, 250, 45}, 0.2f, 4, (Color){20, 20, 20, 180});
    DrawText("Pular Cena (ENTER)", screenWidth - 240, 42, 18, (Color){255, 255, 255, (unsigned char)(200 + pulseBtn * 55)});

    float timeProgress = storyTime / 20.0f; if (timeProgress > 1.0f) timeProgress = 1.0f;
    DrawText("PROGRESSO DA INTRODUÇÃO:", screenWidth - 360, screenHeight - 95, 16, LIGHTGRAY);
    DrawRectangle(screenWidth - 360, screenHeight - 70, 320, 20, DARKGRAY);
    DrawRectangle(screenWidth - 360, screenHeight - 70, (int)(320 * timeProgress), 20, GREEN);
    DrawRectangleLines(screenWidth - 360, screenHeight - 70, 320, 20, WHITE);
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Deixa Eu");
    InitAudioDevice();
    ToggleFullscreen();
    SetTargetFPS(FPS);

    // ========== INICIALIZAR STAGE 1 ==========
    Stage1 stage1;
    initStage1(&stage1);

    // ========== INICIALIZAR PLAYER ==========
    int screenWidth = GetScreenWidth();
    Player player = createPlayer((Vector2){ screenWidth * 0.18f, GROUND_LEVEL }, 150, 3);

    // ========== ESTADOS DO JOGO ==========
    int isGameOver = 0;
    float gameOverTimer = 0.0f;
    float totalGameTime = 0.0f;
    int debugMode = 0;

    // ========== CONTROLE DA HISTÓRIA INICIAL BOLEANA ==========
    Menu menu = createMenu();
    int exibindoIntro = 1; 
    int inMenu = 0;        
    float storyTimer = 0.0f;

    // ========== FASES (Lista Circular) ==========
    Phase *phaseList = NULL;
    Phase *phase1 = createPhase(1, "Recife Chuvoso");
    insertPhase(&phaseList, phase1);

    Phase *currentPhase = phase1;
    printf("Fase atual: %s (numero %d)\n", currentPhase->phaseName, currentPhase->phaseNumber);
    fflush(stdout);

    // ========== LOOP PRINCIPAL ==========
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_F11) || (IsKeyPressed(KEY_F) && IsKeyDown(KEY_LEFT_ALT))) {
            ToggleFullscreen();
        }

        if (IsKeyPressed(KEY_D)) {
            debugMode = !debugMode;
        }

        // ===== ANIMAÇÃO INICIAL DA INTRODUÇÃO =====
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
        }
        // ===== MENU PRINCIPAL =====
        else if (inMenu) {
            updateMenu(&menu);

            if (menu.screen == MENU_MAIN && IsKeyPressed(KEY_ENTER)) {
                if (menu.selectedOption == 0) {
                    inMenu = 0;
                    totalGameTime = 0.0f;
                    isGameOver = 0;

                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    int currentScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ currentScreenWidth * 0.18f, GROUND_LEVEL }, 150, 3);
                }
                else if (menu.selectedOption == 2) {
                    break;
                }
            }
        }
        // ===== GAMEPLAY =====
        else {
            updateStage1(&stage1, &player, deltaTime);
            updatePlayer(&player, deltaTime);

            if (player.lives <= 0 && !isGameOver) {
                isGameOver = 1;
                gameOverTimer = 3.0f;
            }

            if (isGameOver) {
                gameOverTimer -= deltaTime;

                if (IsKeyPressed(KEY_ENTER) || gameOverTimer <= 0) {
                    int resetScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ resetScreenWidth * 0.18f, GROUND_LEVEL }, 150, 3);
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    isGameOver = 0;
                    totalGameTime = 0.0f;

                    menu = createMenu();
                    inMenu = 1;
                }
            }

            if (stage1.stage1Complete && !isGameOver) {
                if (IsKeyPressed(KEY_ENTER)) {
                    int victoryScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ victoryScreenWidth * 0.18f, GROUND_LEVEL }, 150, 3);
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    totalGameTime = 0.0f;

                    menu = createMenu();
                    inMenu = 1;
                }
            }

            if (!isGameOver && !stage1.stage1Complete) {
                totalGameTime += deltaTime;
            }
        }

        // ========== DESENHO GRAFICO ==========
        BeginDrawing();
        int sWidth = GetScreenWidth();
        int sHeight = GetScreenHeight();

        if (exibindoIntro) {
            ClearBackground((Color){11, 16, 27, 255}); 
        } else {
            ClearBackground(SKYBLUE);
        }

        if (exibindoIntro) {
            drawStoryIntroScreen(storyTimer, sWidth, sHeight);
        } 
        else if (inMenu) {
            drawMenu(menu);
        }
        else {
            drawStage1(&stage1, &player);
            drawGameHUD(&stage1, &player, totalGameTime, sWidth, sHeight);

            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG MODE (D para desativar)", 10, 30, 14, RED);
                DrawFPS(10, sHeight - 30);
            }

            if (isGameOver) {
                DrawRectangle(0, 0, sWidth, sHeight, (Color){0, 0, 0, 180});

                const char *gameOverText = "GAME OVER";
                int textWidth = MeasureText(gameOverText, 60);
                DrawText(gameOverText, (sWidth - textWidth) / 2, (int)(sHeight * 0.26f), 60, RED);

                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 20);
                DrawText(finalScoreText, (sWidth - textWidth) / 2, (int)(sHeight * 0.44f), 20, WHITE);

                const char *restartText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(restartText, 16);
                DrawText(restartText, (sWidth - textWidth) / 2, (int)(sHeight * 0.56f), 16, WHITE);

                if (gameOverTimer > 0) {
                    char timerText[64];
                    sprintf(timerText, "Reiniciando em %.1f segundos", gameOverTimer);
                    textWidth = MeasureText(timerText, 14);
                    DrawText(timerText, (sWidth - textWidth) / 2, (int)(sHeight * 0.67f), 14, YELLOW);
                }
            }

            if (stage1.stage1Complete && !isGameOver) {
                DrawRectangle(0, 0, sWidth, sHeight, (Color){0, 0, 0, 180});

                const char *victoryText = "VITÓRIA!";
                int textWidth = MeasureText(victoryText, 60);
                DrawText(victoryText, (sWidth - textWidth) / 2, (int)(sHeight * 0.26f), 60, GREEN);

                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 20);
                DrawText(finalScoreText, (sWidth - textWidth) / 2, (int)(sHeight * 0.44f), 20, WHITE);

                const char *continueText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(continueText, 16);
                DrawText(continueText, (sWidth - textWidth) / 2, (int)(sHeight * 0.56f), 16, WHITE);
            }
        }

        EndDrawing();
    }

    // ========== LIMPEZA RECURSOS ==========
    if (introTexturesLoaded) {
        UnloadTexture(txIntroBg);
        UnloadTexture(txIntroPlayer);
    }
    unloadStage1(&stage1);
    unloadPlayerResources(&player);
    if (phaseList != NULL) {
        freePhaseList(phaseList);
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}