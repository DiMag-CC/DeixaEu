#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steps/stage3.h"
#include "entities/player.h"
#include "entities/pigeon.h"
#include "entities/raindrop.h"
#include "entities/umbrella.h"
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

void drawGameHUD(Stage1 *stage, Player *player, float totalGameTime, 
                 int slowedByRain, float slowDownTimer) {
    
    const int HUD_Y_START = 10;
    const int HUD_Y_STEP = 25;
    
    // ===== COLUNA ESQUERDA (Informações do Jogador) =====
    
    // Vidas
    char livesText[64];
    sprintf(livesText, "Vidas: %d / 3", player->lives);
    DrawText(livesText, 10, HUD_Y_START, 16, BLACK);
    
    // Pontos
    char scoreText[64];
    sprintf(scoreText, "Pontos: %.0f", player->score);
    DrawText(scoreText, 10, HUD_Y_START + HUD_Y_STEP, 16, BLACK);
    
    // Tempo
    char timeText[64];
    sprintf(timeText, "Tempo: %.1f s", totalGameTime);
    DrawText(timeText, 10, HUD_Y_START + HUD_Y_STEP * 2, 16, BLACK);
    
    // Dificuldade
    char diffText[64];
    sprintf(diffText, "Dificuldade: x%.1f", stage->scrollSpeed);
    DrawText(diffText, 10, HUD_Y_START + HUD_Y_STEP * 3, 16, DARKBLUE);
    
    // ===== COLUNA DIREITA (Status Especiais) =====
    
    int rightX = GetScreenWidth() - 300;
    
    // Status de Lentidão
    if (slowedByRain) {
        char slowText[64];
        sprintf(slowText, "LENTO! (%.1f s)", slowDownTimer);
        DrawText(slowText, rightX, HUD_Y_START, 16, RED);
        
        // Barra visual de lentidão
        int barWidth = 150;
        float barProgress = slowDownTimer / 2.0f;  // Max 2 segundos
        if (barProgress > 1.0f) barProgress = 1.0f;
        
        DrawRectangle(rightX, HUD_Y_START + 25, barWidth, 10, LIGHTGRAY);
        DrawRectangle(rightX, HUD_Y_START + 25, (int)(barWidth * barProgress), 10, RED);
        DrawRectangleLinesEx((Rectangle){rightX, HUD_Y_START + 25, barWidth, 10}, 1, BLACK);
    }
    
    // Status de Proteção do Guarda-Chuva
    if (player->hasUmbrella > 0) {
        char protectionText[64];
        sprintf(protectionText, "Protecao: %.1f s", player->umbrellaTimer);
        DrawText(protectionText, rightX, HUD_Y_START + HUD_Y_STEP * 2, 16, GREEN);
        
        // Barra visual de proteção
        int barWidth = 150;
        float barProgress = player->umbrellaTimer / 5.0f;  // Max 5 segundos
        if (barProgress > 1.0f) barProgress = 1.0f;
        
        DrawRectangle(rightX, HUD_Y_START + HUD_Y_STEP * 2 + 25, barWidth, 10, LIGHTGRAY);
        DrawRectangle(rightX, HUD_Y_START + HUD_Y_STEP * 2 + 25, (int)(barWidth * barProgress), 10, GREEN);
        DrawRectangleLinesEx((Rectangle){rightX, HUD_Y_START + HUD_Y_STEP * 2 + 25, barWidth, 10}, 1, BLACK);
    }
    
    // ===== PROGRESSO DA FASE =====
    
    float progressPercent = stage->distanceTraveled / 8000.0f;
    if (progressPercent > 1.0f) progressPercent = 1.0f;
    
    int progressBarY = GetScreenHeight() - 40;
    int progressBarWidth = GetScreenWidth() - 20;
    int progressBarHeight = 20;
    
    char progressText[64];
    sprintf(progressText, "Progresso: %.0f / 8000 m", stage->distanceTraveled);
    DrawText(progressText, 10, progressBarY - 25, 14, BLACK);
    
    DrawRectangle(10, progressBarY, progressBarWidth, progressBarHeight, LIGHTGRAY);
    DrawRectangle(10, progressBarY, (int)(progressBarWidth * progressPercent), progressBarHeight, GREEN);
    DrawRectangleLinesEx((Rectangle){10, progressBarY, progressBarWidth, progressBarHeight}, 2, BLACK);
}

// ========== DEBUG: RENDERIZAR PLAYER ==========

void drawPlayerDebug(Player player) {
    // Desenhar hitbox com linhas tracejadas
    DrawRectangleLinesEx(player.hitbox, 1, RED);
    
    // Desenhar ponto de origem
    DrawCircle(player.position.x, player.position.y, 3, GREEN);
    
    // Desenhar vetor velocidade
    if (player.velocity.x != 0 || player.velocity.y != 0) {
        Vector2 velocityEnd = {
            player.position.x + player.velocity.x * 10,
            player.position.y + player.velocity.y * 10
        };
        DrawLineEx(player.position, velocityEnd, 2, YELLOW);
    }
    
    // Desenhar status na tela
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

    // ========== INICIALIZAR STAGE 3 ==========
    Stage3 stage3;

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
    
    Phase *phase3 = createPhase(3, "Parque das Esculturas");
    
    insertPhase(&phaseList, phase3);
    
    Phase *currentPhase = phase3;
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
                    slowedByRain = 0;
                    slowDownTimer = 0.0f;
                    
                    initStage3(&stage3, &player);
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
            updateStage3(&stage3, &player, deltaTime);
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
                    // Reset do jogo e voltar para o menu
                    player = createPlayer((Vector2){ SCREEN_WIDTH / 2, GROUND_LEVEL }, 150, 3);
                    unloadStage3(&stage3);
                    isGameOver = 0;
                    totalGameTime = 0.0f;
                    slowedByRain = 0;
                    slowDownTimer = 0.0f;
                    
                    menu = createMenu();
                    inMenu = 1;
                }
            }
            
            // ===== VICTORY REDIRECTION =====
            if (stage3.state == STAGE3_FINISHED) {
                if (IsKeyPressed(KEY_ENTER)) {
                    // Reset do jogo e voltar para o menu
                    player = createPlayer((Vector2){ SCREEN_WIDTH / 2, GROUND_LEVEL }, 150, 3);
                    unloadStage3(&stage3);
                    isGameOver = 0;
                    totalGameTime = 0.0f;
                    slowedByRain = 0;
                    slowDownTimer = 0.0f;
                    
                    menu = createMenu();
                    inMenu = 1;
                }
            }
            
            // ===== CONTAR TEMPO (se não está game over/venceu) =====
            if (!isGameOver && stage3.state != STAGE3_FINISHED) {
                totalGameTime += deltaTime;
            }
        }

        // ========== DESENHO ==========
        BeginDrawing();
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        // Stage 3 usa fundo azul cel; evita flash branco chamando o clear correto
        ClearBackground(SKYBLUE);

        if (inMenu) {
            // ===== DESENHAR MENU =====
            drawMenu(menu);
        } 
<<<<<<< HEAD
        else {
            // ===== DESENHAR JOGO =====
            drawStage1(&stage, &player);
            drawPlayer(player);
=======
        else {
            // ===== DESENHAR JOGO COM CÂMERA RESPONSIVA =====
            float scaleX = screenWidth / WORLD_WIDTH;
            float scaleY = screenHeight / WORLD_HEIGHT;
            float worldScale = (scaleX < scaleY) ? scaleX : scaleY;

            Camera2D camera = { 0 };
            camera.zoom = worldScale * 0.90f;
            camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
            camera.target = (Vector2){
                player.position.x + PLAYER_WIDTH / 2.0f,
                player.position.y + player.height / 2.0f - CAMERA_VERTICAL_LOOKAHEAD
            };
            
            BeginMode2D(camera);
            
            drawStage3(&stage3, &player);
            drawPlayer(player);
            
            EndMode2D();

            // ===== HUD - VIDAS =====
            char livesText[32];
            sprintf(livesText, "Vidas: %d", player.lives);
            DrawText(livesText, 10, 10, 20, BLACK);
>>>>>>> fase3
            
            // ===== DESENHAR HUD COMPLETO (COMMIT 3) =====
            drawGameHUD(&stage, &player, totalGameTime, slowedByRain, slowDownTimer);
            
<<<<<<< HEAD
            // ===== DEBUG MODE =====
            if (IsKeyPressed(KEY_D)) {
                debugMode = !debugMode;
            }
            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG MODE (D para desativar)", 10, 30, 14, RED);
            }
=======
            // ===== HUD - TEMPO =====
            char timeText[64];
            sprintf(timeText, "Tempo: %.1f seg", totalGameTime);
            DrawText(timeText, 10, 60, 20, BLACK);

            // ===== HUD - STATUS DE LENTIDÃO =====
            if (slowedByRain) {
                char slowText[64];
                sprintf(slowText, "LENTO! %.1f seg", slowDownTimer);
                DrawText(slowText, screenWidth - 200, 10, 16, RED);
                
                // Desenhar barra visual de lentidão
                DrawRectangle(screenWidth - 200, 30, 150, 10, RED);
                DrawRectangle(screenWidth - 200, 30, (int)(150 * (slowDownTimer / 2.0f)), 10, YELLOW);
            }

            // ===== HUD - PROTEÇÃO DO GUARDA-CHUVA =====
            if (player.hasUmbrella > 0) {
                char protectionText[64];
                sprintf(protectionText, "Protecao: %.1f s", player.umbrellaTimer);
                DrawText(protectionText, screenWidth - 250, 45, 16, GREEN);
            }
>>>>>>> fase3

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
            
            // ===== DESENHAR VITORIA =====
            if (stage3.state == STAGE3_FINISHED) {
                // Overlay escuro com tom dourado sutil
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){10, 10, 30, 210});
                
                // Texto de Vitória ("Valeu a pena, mae.")
                const char *victoryText = "VALEU A PENA, MAE.";
                int textWidth = MeasureText(victoryText, 40);
                DrawText(victoryText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.27f), 40, YELLOW);
                
                // Pontos e tempo
                char finalScoreText[128];
                sprintf(finalScoreText, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                textWidth = MeasureText(finalScoreText, 22);
                DrawText(finalScoreText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.42f), 22, WHITE);
                
                // Instruções para retornar ao menu
                const char *returnText = "Pressione ENTER para voltar ao menu";
                textWidth = MeasureText(returnText, 18);
                DrawText(returnText, (screenWidth - textWidth) / 2, (int)(screenHeight * 0.56f), 18, GREEN);
            }

            // ===== FPS (Debug) =====
            DrawFPS(screenWidth - 80, 10);
        }

        EndDrawing();
    }

<<<<<<< HEAD
    unloadStage1(&stage);
    unloadPlayer(player);
=======
    // ========== LIMPEZA ==========
    unloadStage3(&stage3);
>>>>>>> fase3

    if (phaseList != NULL) {
        free(phase3);
    }

    CloseWindow();

    return 0;
}
