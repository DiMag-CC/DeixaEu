#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steps/stage1.h"
#include "steps/stage2.h"  // Incluído para a Fase 2
#include "steps/stage3.h"
#include "entities/player.h"
#include "menu.h"
#include "structure/stepList.h"
#include "utils/utils.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define WORLD_WIDTH 800.0f
#define WORLD_HEIGHT 450.0f
#define CAMERA_VERTICAL_LOOKAHEAD 150.0f
#define FPS 60

void drawGameHUD(Stage1 *stage, Player *player, float totalGameTime, int screenWidth, int screenHeight) {
    const int HUD_Y_START = 10;
    const int HUD_Y_STEP = 25;
    const int HUD_MARGIN = 10;

    int fontSize = (screenWidth < 1024) ? 14 : 16;

    char livesText[64];
    sprintf(livesText, "Vidas: %d / 3", player->lives);
    DrawText(livesText, HUD_MARGIN, HUD_Y_START, fontSize, BLACK);

    char scoreText[64];
    sprintf(scoreText, "Pontos: %.0f", player->score);
    DrawText(scoreText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP, fontSize, BLACK);

    char timeText[64];
    sprintf(timeText, "Tempo: %.1f s", totalGameTime);
    DrawText(timeText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP * 2, fontSize, BLACK);

    char diffText[64];
    sprintf(diffText, "Dificuldade: x%.1f", stage->difficultyMultiplier);
    DrawText(diffText, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP * 3, fontSize, DARKBLUE);

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

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Deixa Eu");
    SetWindowState(FLAG_WINDOW_HIGHDPI);
    // ToggleFullscreen(); // Comentado para evitar que o VS Code mude de tamanho sozinhho
    SetTargetFPS(FPS);

    // ========== INICIALIZAR ESTRUTURAS DAS FASES ==========
    Stage1 stage1;
    initStage1(&stage1);

    Stage2 stage2;
    initStage2(&stage2);

    // ========== INICIALIZAR PLAYER ==========
    int screenWidth = GetScreenWidth();
    Player player = createPlayer((Vector2){ screenWidth / 2, 450.0f * 0.82f }, 150, 3);

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
    
    Phase *phase2 = createPhase(2, "Boa Viagem");
    insertPhase(&phaseList, phase2);

    // ATALHO: Aponta direto para o próximo da lista (Fase 2) para testar direto
    Phase *currentPhase = phaseList->next; 
    
    printf("Fase de teste atual: %s (numero %d)\n", currentPhase->phaseName, currentPhase->phaseNumber);
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

        // ===== LÓGICA DO MENU =====
        if (inMenu) {
            updateMenu(&menu);

            if (menu.screen == MENU_MAIN && IsKeyPressed(KEY_ENTER)) {
                if (menu.selectedOption == 0) {
                    inMenu = 0;
                    totalGameTime = 0.0f;
                    isGameOver = 0;

                    // Reinicializar fases
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    unloadStage2(&stage2);
                    initStage2(&stage2);
                    
                    int currentScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ currentScreenWidth / 2, 450.0f * 0.82f }, 150, 3);
                }
                else if (menu.selectedOption == 2) {
                    break;
                }
            }
        }
        // ===== LÓGICA DO JOGO =====
        else {
            // ATUALIZAÇÃO DINÂMICA BASEADA NA FASE
            switch (currentPhase->phaseNumber) {
                case 1:
                    updateStage1(&stage1, &player, deltaTime);
                    break;
                case 2:
                    updateStage2(&stage2, &player, deltaTime);
                    break;
            }
            
            updatePlayer(&player, deltaTime);

            if (player.lives <= 0 && !isGameOver) {
                isGameOver = 1;
                gameOverTimer = 3.0f;
            }

            if (isGameOver) {
                gameOverTimer -= deltaTime;

                if (IsKeyPressed(KEY_ENTER) || gameOverTimer <= 0) {
                    int resetScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ resetScreenWidth / 2, 450.0f * 0.82f }, 150, 3);
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    unloadStage2(&stage2);
                    initStage2(&stage2);
                    isGameOver = 0;
                    totalGameTime = 0.0f;

                    menu = createMenu();
                    inMenu = 1;
                }
            }

            // Checagem de vitória automática por fase
            int currentStageComplete = (currentPhase->phaseNumber == 1) ? stage1.stage1Complete : stage2.stage2Complete;

            if (currentStageComplete && !isGameOver) {
                if (IsKeyPressed(KEY_ENTER)) {
                    int victoryScreenWidth = GetScreenWidth();
                    player = createPlayer((Vector2){ victoryScreenWidth / 2, 450.0f * 0.82f }, 150, 3);
                    unloadStage1(&stage1);
                    initStage1(&stage1);
                    unloadStage2(&stage2);
                    initStage2(&stage2);
                    totalGameTime = 0.0f;

                    menu = createMenu();
                    inMenu = 1;
                }
            }

            if (!isGameOver && !currentStageComplete) {
                totalGameTime += deltaTime;
            }
        }

        // ========== DESENHO ==========
        BeginDrawing();
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        ClearBackground(SKYBLUE);

        if (inMenu) {
            drawMenu(menu);
        }
        else {
            // DESENHO DINÂMICO BASEADO NA FASE
            switch (currentPhase->phaseNumber) {
                case 1:
                    drawStage1(&stage1, &player);
                    drawGameHUD(&stage1, &player, totalGameTime, screenWidth, screenHeight);
                    break;
                case 2:
                    drawStage2(&stage2, &player);
                    // O HUD ou barras específicas da fase 2 são desenhadas pelo próprio drawStage2
                    break;
            }

            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG MODE (D para desativar)", 10, 30, 14, RED);
                DrawFPS(10, screenHeight - 30);
            }

            int currentStageComplete = (currentPhase->phaseNumber == 1) ? stage1.stage1Complete : stage2.stage2Complete;

            if (isGameOver) {
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 180});
                const char *gameOverText = "GAME OVER";
                int textWidth = MeasureText(gameOverText, 60);
                DrawText(gameOverText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.26f), 60, RED);

                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 20);
                DrawText(finalScoreText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.44f), 20, WHITE);

                const char *restartText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(restartText, 16);
                DrawText(restartText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.56f), 16, WHITE);

                if (gameOverTimer > 0) {
                    char timerText[64];
                    sprintf(timerText, "Reiniciando em %.1f segundos", gameOverTimer);
                    textWidth = MeasureText(timerText, 14);
                    DrawText(timerText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.67f), 14, YELLOW);
                }
            }

            if (currentStageComplete && !isGameOver) {
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 180});
                const char *victoryText = "VITÓRIA!";
                int textWidth = MeasureText(victoryText, 60);
                DrawText(victoryText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.26f), 60, GREEN);

                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 20);
                DrawText(finalScoreText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.44f), 20, WHITE);

                const char *continueText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(continueText, 16);
                DrawText(continueText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.56f), 16, WHITE);
            }
        }

        EndDrawing();
    }

    // ========== LIMPEZA ==========
    unloadStage1(&stage1);
    unloadStage2(&stage2);
    unloadPlayerResources(&player);
    if (phaseList != NULL) {
        freePhaseList(phaseList);
    }

    CloseWindow();
    return 0;
}