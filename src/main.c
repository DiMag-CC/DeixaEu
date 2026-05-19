#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steps/stage1.h"
#include "entities/player.h"
#include "entities/pigeon.h"
#include "entities/raindrop.h"
#include "entities/umbrella.h"
#include "menu.h"
#include "structure/stepList.h"

// ========== CONSTANTES ==========
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define FPS 60

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Deixa Eu");
    SetTargetFPS(FPS);

    // ========== INICIALIZAR STAGE 1 ==========
    Stage1 stage;
    initStage1(&stage);

    // ========== INICIALIZAR PLAYER ==========
    Player player = createPlayer((Vector2){ SCREEN_WIDTH / 2, GROUND_LEVEL }, 150, 3);

    // ========== ESTADOS DO JOGO ==========
    int isGameOver = 0;
    float gameOverTimer = 0.0f;
    float totalGameTime = 0.0f;
    int slowedByRain = 0;      // Flag: player está lento por chuva/fezes?
    float slowDownTimer = 0.0f; // Timer de duração da lentidão
    int debugMode = 0;  // Pressionar 'D' para togglear

    // ========== MENU ==========
    Menu menu = createMenu();
    int inMenu = 1;

    // ========== FASES (Lista Circular) ==========
    Phase *phaseList = NULL;
    
    Phase *phase1 = createPhase(1, "Pedalando no Caos");
    Phase *phase2 = createPhase(2, "Boa Viagem");
    Phase *phase3 = createPhase(3, "Parque das Esculturas");
    
    insertPhase(&phaseList, phase1);
    insertPhase(&phaseList, phase2);
    insertPhase(&phaseList, phase3);
    
    Phase *currentPhase = phaseList;
    printf("Fase atual: %s (numero %d)\n", currentPhase->phaseName, currentPhase->phaseNumber);
    fflush(stdout);

    // ========== LOOP PRINCIPAL ==========
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // ===== MENU =====
        if (inMenu) {
            updateMenu(&menu);
            
            if (IsKeyPressed(KEY_ESCAPE) && menu.screen == MENU_CREDITS) {
                menu.screen = MENU_MAIN;
                menu.selectedOption = 0;
            }
            
            if (menu.screen == MENU_MAIN && IsKeyPressed(KEY_ENTER)) {
                if (menu.selectedOption == 0) {
                    // Iniciar jogo
                    inMenu = 0;
                    totalGameTime = 0.0f;
                    isGameOver = 0;
                    slowedByRain = 0;
                    slowDownTimer = 0.0f;
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
            updateStage1(&stage, &player, deltaTime);
            updatePlayer(&player, deltaTime);

            // ===== SISTEMA DE LENTIDÃO =====
            // Se o player estava lento, decrementar timer
            if (slowedByRain) {
                slowDownTimer -= deltaTime;
                
                if (slowDownTimer <= 0.0f) {
                    slowedByRain = 0;
                    slowDownTimer = 0.0f;
                    
                    // Restaurar velocidade normal
                    player.speed = 150.0f;  // Velocidade original
                }
            }

            // ===== VERIFICAR GAME OVER =====
            if (player.lives <= 0 && !isGameOver) {
                isGameOver = 1;
                gameOverTimer = 3.0f;
            }

            // ===== GAME OVER SCREEN =====
            if (isGameOver) {
                gameOverTimer -= deltaTime;
                
                if (IsKeyPressed(KEY_ENTER) || gameOverTimer <= 0) {
                    // Reiniciar jogo
                    player = createPlayer((Vector2){ SCREEN_WIDTH / 2, GROUND_LEVEL }, 150, 3);
                    unloadStage1(&stage);
                    initStage1(&stage);
                    isGameOver = 0;
                    totalGameTime = 0.0f;
                    slowedByRain = 0;
                    slowDownTimer = 0.0f;
                    
                    menu = createMenu();
                    inMenu = 1;
                }
            }
            
            // ===== CONTAR TEMPO (se não está game over) =====
            if (!isGameOver) {
                totalGameTime += deltaTime;
            }
        }

        // ========== DESENHO ==========
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (inMenu) {
            // ===== DESENHAR MENU =====
            drawMenu(menu);
        } 
        else {
            // ===== DESENHAR JOGO =====
            drawStage1(&stage);
            drawPlayer(player);
            
            // ===== DEBUG MODE =====
            if (IsKeyPressed(KEY_D)) {
                debugMode = !debugMode;
            }
            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG MODE (D para desativar)", 10, 30, 14, RED);
            }

            // ===== HUD - VIDAS =====
            char livesText[32];
            sprintf(livesText, "Vidas: %d", player.lives);
            DrawText(livesText, 10, 10, 20, BLACK);
            
            // ===== HUD - PONTOS =====
            char scoreText[64];
            sprintf(scoreText, "Pontos: %.0f", player.score);
            DrawText(scoreText, 10, 35, 20, BLACK);
            
            // ===== HUD - TEMPO =====
            char timeText[64];
            sprintf(timeText, "Tempo: %.1f seg", totalGameTime);
            DrawText(timeText, 10, 60, 20, BLACK);

            // ===== HUD - STATUS DE LENTIDÃO =====
            if (slowedByRain) {
                char slowText[64];
                sprintf(slowText, "LENTO! %.1f seg", slowDownTimer);
                DrawText(slowText, SCREEN_WIDTH - 200, 10, 16, RED);
                
                // Desenhar barra visual de lentidão
                DrawRectangle(SCREEN_WIDTH - 200, 30, 150, 10, RED);
                DrawRectangle(SCREEN_WIDTH - 200, 30, (int)(150 * (slowDownTimer / 2.0f)), 10, YELLOW);
            }

            // ===== HUD - PROTEÇÃO DO GUARDA-CHUVA =====
            if (player.hasUmbrella > 0) {
                char protectionText[64];
                sprintf(protectionText, "Protecao: %.1f s", player.umbrellaTimer);
                DrawText(protectionText, SCREEN_WIDTH - 250, 45, 16, GREEN);
            }

            // ===== DESENHAR GAME OVER =====
            if (isGameOver) {
                // Overlay escuro
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
                
                // Texto "GAME OVER"
                const char *gameOverText = "GAME OVER";
                int textWidth = MeasureText(gameOverText, 60);
                DrawText(gameOverText, (SCREEN_WIDTH - textWidth) / 2, 120, 60, RED);
                
                // Pontuação final
                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 20);
                DrawText(finalScoreText, (SCREEN_WIDTH - textWidth) / 2, 200, 20, WHITE);
                
                // Instruções
                const char *restartText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(restartText, 16);
                DrawText(restartText, (SCREEN_WIDTH - textWidth) / 2, 250, 16, WHITE);
                
                // Timer de auto-reinício
                if (gameOverTimer > 0) {
                    char timerText[64];
                    sprintf(timerText, "Reiniciando em %.1f segundos", gameOverTimer);
                    textWidth = MeasureText(timerText, 14);
                    DrawText(timerText, (SCREEN_WIDTH - textWidth) / 2, 300, 14, YELLOW);
                }
            }

            // ===== FPS (Debug) =====
            DrawFPS(SCREEN_WIDTH - 80, 10);
        }

        EndDrawing();
    }

    // ========== LIMPEZA ==========
    unloadStage1(&stage);

    if (phaseList != NULL) {
        free(phase1);
        free(phase2);
        free(phase3);
    }

    CloseWindow();

    return 0;
}