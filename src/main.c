#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steps/stage1.h"
#include "steps/stage3.h"
#include "entities/player.h"
#include "menu.h"
#include "structure/stepList.h"

// ========== CONSTANTES ==========
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define WORLD_WIDTH 800.0f
#define WORLD_HEIGHT 450.0f
#define CAMERA_VERTICAL_LOOKAHEAD 150.0f
#define FPS 60

// ========== RENDERIZAR HUD ==========
void drawGameHUD(Stage1 *stage, Player *player, float totalGameTime, int screenWidth, int screenHeight) {
    const int HUD_Y_START = 10;
    const int HUD_Y_STEP = 25;
    const int HUD_MARGIN = 10;

    // ===== VIDAS =====
    char livesText[64];
    sprintf(livesText, "Vidas: %d / 3", player->lives);
    DrawText(livesText, HUD_MARGIN, HUD_Y_START, 16, BLACK);

    // ===== PONTOS =====
    char scoreText[64];
    sprintf(scoreText, "Pontos: %.0f", player->score);
    DrawText(scoreText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP, 16, BLACK);

    // ===== TEMPO =====
    char timeText[64];
    sprintf(timeText, "Tempo: %.1f s", totalGameTime);
    DrawText(timeText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP * 2, 16, BLACK);

    // ===== DIFICULDADE =====
    char diffText[64];
    sprintf(diffText, "Dificuldade: x%.1f", stage->difficultyMultiplier);
    DrawText(diffText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP * 3, 16, DARKBLUE);

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
        DrawRectangleLinesEx((Rectangle){protectionX, HUD_Y_START + 25, barWidth, 10}, 1, BLACK);
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
    DrawRectangleLinesEx((Rectangle){HUD_MARGIN, progressBarY, progressBarWidth, progressBarHeight}, 2, BLACK);
}

// ========== DEBUG MODE ==========
void drawPlayerDebug(Player player) {
    DrawRectangleLinesEx(player.hitbox, 1, RED);
    DrawCircle(player.position.x, player.position.y, 3, GREEN);

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

// ========== MAIN ==========
int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Deixa Eu");
    SetTargetFPS(FPS);

    // ========== INICIALIZAR STAGE 1 ==========
    Stage1 stage1;
    initStage1(&stage1);

    // ========== INICIALIZAR PLAYER ==========
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    Player player = createPlayer((Vector2){ screenWidth / 2, GROUND_LEVEL }, 150, 3);

    // ========== ESTADOS DO JOGO ==========
    int isGameOver = 0;
    float gameOverTimer = 0.0f;
    float totalGameTime = 0.0f;
    int debugMode = 0;

    // ========== MENU ==========
    Menu menu = createMenu();
    int inMenu = 1;

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

        // ===== ALTERAR TELA CHEIA =====
        if (IsKeyPressed(KEY_F)) {
            ToggleFullscreen();
        }

        // ===== MENU =====
        if (inMenu) {
            updateMenu(&menu);

            if (menu.screen == MENU_MAIN && IsKeyPressed(KEY_ENTER)) {
                if (menu.selectedOption == 0) {
                    // Iniciar jogo
                    inMenu = 0;
                    totalGameTime = 0.0f;
                    isGameOver = 0;

                    // Reinicializar stage
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    int currentScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ currentScreenWidth / 2, GROUND_LEVEL }, 150, 3);
                }
                else if (menu.selectedOption == 2) {
                    // Sair do jogo
                    break;
                }
            }
        }
        // ===== JOGO =====
        else {
            // ===== ATUALIZAR STAGE E PLAYER =====
            updateStage1(&stage1, &player, deltaTime);
            updatePlayer(&player, deltaTime);

            // ===== VERIFICAR GAME OVER =====
            if (player.lives <= 0 && !isGameOver) {
                isGameOver = 1;
                gameOverTimer = 3.0f;
            }

            // ===== GAME OVER SCREEN =====
            if (isGameOver) {
                gameOverTimer -= deltaTime;

                if (IsKeyPressed(KEY_ENTER) || gameOverTimer <= 0) {
                    // Reset do jogo e voltar para o menu
                    int resetScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ resetScreenWidth / 2, GROUND_LEVEL }, 150, 3);
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    isGameOver = 0;
                    totalGameTime = 0.0f;

                    menu = createMenu();
                    inMenu = 1;
                }
            }

            // ===== VICTORY CHECK =====
            if (stage1.stage1Complete && !isGameOver) {
                if (IsKeyPressed(KEY_ENTER)) {
                    // Reset do jogo e voltar para o menu
                    int victoryScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ victoryScreenWidth / 2, GROUND_LEVEL }, 150, 3);
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    totalGameTime = 0.0f;

                    menu = createMenu();
                    inMenu = 1;
                }
            }

            // ===== CONTAR TEMPO =====
            if (!isGameOver && !stage1.stage1Complete) {
                totalGameTime += deltaTime;
            }
        }

        // ========== DESENHO ==========
        BeginDrawing();
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        ClearBackground(SKYBLUE);

        if (inMenu) {
            // ===== DESENHAR MENU =====
            drawMenu(menu);
        }
        else {
            // ===== DESENHAR JOGO =====
            drawStage1(&stage1, &player);
            drawPlayer(player);

            // ===== DESENHAR HUD =====
            drawGameHUD(&stage1, &player, totalGameTime, screenWidth, screenHeight);

            // ===== DEBUG MODE =====
            if (IsKeyPressed(KEY_D)) {
                debugMode = !debugMode;
            }
            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG MODE (D para desativar)", 10, 30, 14, RED);
                DrawFPS(10, screenHeight - 30);
            }

            // ===== DESENHAR GAME OVER =====
            if (isGameOver) {
                // Overlay escuro
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 180});

                // Texto "GAME OVER"
                const char *gameOverText = "GAME OVER";
                int textWidth = MeasureText(gameOverText, 60);
                DrawText(gameOverText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.26f), 60, RED);

                // Pontuação final
                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 20);
                DrawText(finalScoreText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.44f), 20, WHITE);

                // Instruções
                const char *restartText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(restartText, 16);
                DrawText(restartText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.56f), 16, WHITE);

                // Timer de auto-reinício
                if (gameOverTimer > 0) {
                    char timerText[64];
                    sprintf(timerText, "Reiniciando em %.1f segundos", gameOverTimer);
                    textWidth = MeasureText(timerText, 14);
                    DrawText(timerText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.67f), 14, YELLOW);
                }
            }

            // ===== DESENHAR VITÓRIA =====
            if (stage1.stage1Complete && !isGameOver) {
                // Overlay escuro
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 180});

                // Texto "VITÓRIA"
                const char *victoryText = "VITÓRIA!";
                int textWidth = MeasureText(victoryText, 60);
                DrawText(victoryText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.26f), 60, GREEN);

                // Pontuação final
                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 20);
                DrawText(finalScoreText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.44f), 20, WHITE);

                // Instruções
                const char *continueText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(continueText, 16);
                DrawText(continueText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.56f), 16, WHITE);
            }
        }

        EndDrawing();
    }

    // ========== LIMPEZA ==========
    unloadStage1(&stage1);
    unloadPlayerResources(&player);
    if (phaseList != NULL) {
        freePhaseList(phaseList);
    }

    CloseWindow();
    return 0;
}
