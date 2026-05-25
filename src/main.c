#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steps/stage1.h"
#include "steps/stage3.h"
#include "entities/player.h"
#include "menu.h"
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

typedef enum {
    GAME_STAGE_1 = 1,
    GAME_STAGE_3 = 3
} GameStage;

typedef enum {
    PAUSE_RESUME = 0,
    PAUSE_MENU = 1,
    PAUSE_RESTART = 2
} PauseOption;

static Texture2D hudHeartTexture = {0};

static int playerReachedStage1Exit(Player *player) {
    float visibleWidth = GetScreenWidth() / STAGE1_CAMERA_ZOOM;
    float rightLimit = GetScreenWidth() * 0.5f + visibleWidth * 0.5f - player->width * 0.5f;

    return player->position.x >= rightLimit - 2.0f;
}

static void skipToNextStage(Stage1 *stage1,
                            Stage3 *stage3,
                            Player *player,
                            int *stage1Initialized,
                            int *stage3Initialized,
                            GameStage *activeStage,
                            Phase **currentPhase,
                            Phase *phase3,
                            float *bikeDropOverlayTimer,
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

    printf("Fase atual: %s (numero %d)\n", (*currentPhase)->phaseName, (*currentPhase)->phaseNumber);
    fflush(stdout);
}

void drawHeartIcon(float x, float y, float size, Color color) {
    if (hudHeartTexture.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)hudHeartTexture.width, (float)hudHeartTexture.height };
        Rectangle dest = { x, y, size, size };
        DrawTexturePro(hudHeartTexture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, color);
        return;
    }

    DrawCircle((int)(x + size * 0.30f), (int)(y + size * 0.30f), size * 0.22f, color);
    DrawCircle((int)(x + size * 0.70f), (int)(y + size * 0.30f), size * 0.22f, color);
    DrawTriangle(
        (Vector2){ x + size * 0.08f, y + size * 0.38f },
        (Vector2){ x + size * 0.92f, y + size * 0.38f },
        (Vector2){ x + size * 0.50f, y + size * 0.98f },
        color
    );
}

void drawHudBar(float x, float y, float width, float height, float percent, Color fillColor) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;

    Rectangle bg = { x, y, width, height };
    Rectangle fill = { x, y, width * percent, height };

    DrawRectangleRounded(bg, 0.45f, 8, (Color){ 28, 35, 46, 210 });
    DrawRectangleRounded(fill, 0.45f, 8, fillColor);
    DrawRectangleRoundedLines(bg, 0.45f, 8, (Color){ 255, 255, 255, 120 });
}

void drawGameHUD(Stage1 *stage, Player *player, float totalGameTime, int screenWidth, int screenHeight) {
    (void)screenHeight;

    float progressPercent = stage->distanceTraveled / STAGE1_TARGET_DISTANCE;
    if (progressPercent > 1.0f) progressPercent = 1.0f;
    if (progressPercent < 0.0f) progressPercent = 0.0f;

    float panelWidth = screenWidth < 760 ? screenWidth - 24.0f : 620.0f;
    float panelHeight = 116.0f;
    float panelX = 12.0f;
    float panelY = 12.0f;
    Rectangle panel = { panelX, panelY, panelWidth, panelHeight };

    DrawRectangleRounded(panel, 0.08f, 10, (Color){ 7, 18, 32, 185 });
    DrawRectangleRoundedLines(panel, 0.08f, 10, (Color){ 255, 255, 255, 95 });

    DrawText("Fase 1", (int)(panelX + 18), (int)(panelY + 14), 18, RAYWHITE);
    for (int i = 0; i < 3; i++) {
        Color heartColor = (i < player->lives) ? (Color){ 226, 48, 70, 255 } : (Color){ 81, 88, 101, 230 };
        drawHeartIcon(panelX + 90.0f + i * 30.0f, panelY + 12.0f, 24.0f, heartColor);
    }

    char scoreText[64];
    sprintf(scoreText, "%.0f pts", player->score);
    DrawText(scoreText, (int)(panelX + panelWidth - MeasureText(scoreText, 18) - 18), (int)(panelY + 14), 18, RAYWHITE);

    DrawText("Progresso", (int)(panelX + 18), (int)(panelY + 52), 14, (Color){ 215, 225, 235, 255 });
    drawHudBar(panelX + 100.0f, panelY + 53.0f, panelWidth - 118.0f, 16.0f, progressPercent, (Color){ 64, 197, 112, 255 });

    char timeText[64];
    sprintf(timeText, "Tempo %.1fs", totalGameTime);
    DrawText(timeText, (int)(panelX + 18), (int)(panelY + 84), 14, (Color){ 215, 225, 235, 255 });

    char diffText[64];
    sprintf(diffText, "Dificuldade x%.1f", stage->difficultyMultiplier);
    DrawText(diffText, (int)(panelX + 150), (int)(panelY + 84), 14, (Color){ 215, 225, 235, 255 });

    if (player->hasUmbrella > 0) {
        float umbrellaPercent = player->umbrellaTimer / 8.0f;
        DrawText("Protecao", (int)(panelX + panelWidth - 210), (int)(panelY + 84), 14, (Color){ 215, 225, 235, 255 });
        drawHudBar(panelX + panelWidth - 132.0f, panelY + 86.0f, 112.0f, 12.0f, umbrellaPercent, (Color){ 89, 167, 255, 255 });
    }
}

void drawStage3HUD(Player *player, float totalGameTime, int screenWidth) {
    float panelWidth = screenWidth < 620 ? screenWidth - 24.0f : 430.0f;
    float panelX = 12.0f;
    float panelY = 12.0f;
    Rectangle panel = { panelX, panelY, panelWidth, 86.0f };

    DrawRectangleRounded(panel, 0.08f, 10, (Color){ 7, 18, 32, 185 });
    DrawRectangleRoundedLines(panel, 0.08f, 10, (Color){ 255, 255, 255, 95 });

    DrawText("Fase 3", (int)(panelX + 18), (int)(panelY + 14), 18, RAYWHITE);

    for (int i = 0; i < 3; i++) {
        Color heartColor = (i < player->lives) ? (Color){ 226, 48, 70, 255 } : (Color){ 81, 88, 101, 230 };
        drawHeartIcon(panelX + 90.0f + i * 30.0f, panelY + 12.0f, 24.0f, heartColor);
    }

    char scoreText[64];
    sprintf(scoreText, "%.0f pts", player->score);
    DrawText(scoreText, (int)(panelX + panelWidth - MeasureText(scoreText, 18) - 18), (int)(panelY + 14), 18, RAYWHITE);

    char timeText[64];
    sprintf(timeText, "Tempo %.1fs", totalGameTime);
    DrawText(timeText, (int)(panelX + 18), (int)(panelY + 54), 15, (Color){ 215, 225, 235, 255 });

    DrawText("Chegue ate a torre", (int)(panelX + panelWidth - 152), (int)(panelY + 54), 15, (Color){ 215, 225, 235, 255 });
}

void drawPauseMenu(int selectedOption, int screenWidth, int screenHeight) {
    const char *options[] = { "Voltar", "Menu", "Reiniciar" };
    int panelWidth = 360;
    int panelHeight = 300;
    int panelX = (screenWidth - panelWidth) / 2;
    int panelY = (screenHeight - panelHeight) / 2;

    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 0, 0, 0, 145 });
    DrawRectangleRounded((Rectangle){ panelX, panelY, panelWidth, panelHeight }, 0.08f, 10, (Color){ 8, 18, 32, 235 });
    DrawRectangleRoundedLines((Rectangle){ panelX, panelY, panelWidth, panelHeight }, 0.08f, 10, (Color){ 255, 255, 255, 120 });

    const char *title = "PAUSE";
    int titleSize = 42;
    DrawText(title, panelX + (panelWidth - MeasureText(title, titleSize)) / 2, panelY + 32, titleSize, YELLOW);

    for (int i = 0; i < 3; i++) {
        int fontSize = 28;
        int textWidth = MeasureText(options[i], fontSize);
        int y = panelY + 112 + i * 58;
        Rectangle hitbox = { panelX + 52.0f, y - 8.0f, panelWidth - 104.0f, 44.0f };

        if (selectedOption == i) {
            DrawRectangleRounded(hitbox, 0.25f, 8, (Color){ 255, 255, 255, 36 });
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
    if (selectedOption < 0) selectedOption = 2;
    if (selectedOption > 2) selectedOption = 0;

    Vector2 mouse = GetMousePosition();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int panelX = (screenWidth - 360) / 2;
    int panelY = (screenHeight - 300) / 2;

    for (int i = 0; i < 3; i++) {
        Rectangle hitbox = { panelX + 52.0f, panelY + 112.0f + i * 58.0f - 8.0f, 256.0f, 44.0f };
        if (CheckCollisionPointRec(mouse, hitbox)) {
            selectedOption = i;
        }
    }

    return selectedOption;
}

void drawBikeDropOverlay(float timer, int screenWidth, int screenHeight) {
    float alphaFactor = timer > 1.0f ? 1.0f : timer;
    unsigned char alpha = (unsigned char)(170.0f * alphaFactor);
    Color overlay = { 0, 0, 0, alpha };

    DrawRectangle(0, 0, screenWidth, screenHeight, overlay);

    const char *title = "Bicicleta deixada para tras";
    const char *subtitle = "Agora e a pe ate a torre";
    int titleSize = 38;
    int subtitleSize = 22;
    int titleX = (screenWidth - MeasureText(title, titleSize)) / 2;
    int subtitleX = (screenWidth - MeasureText(subtitle, subtitleSize)) / 2;
    int centerY = screenHeight / 2;

    DrawText(title, titleX, centerY - 42, titleSize, (Color){ 255, 238, 117, (unsigned char)(255.0f * alphaFactor) });
    DrawText(subtitle, subtitleX, centerY + 12, subtitleSize, (Color){ 235, 242, 250, (unsigned char)(240.0f * alphaFactor) });
}

void refreshStage1Layout(Stage1 *stage) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    GLOBAL_WORLD_SCALE = (float)screenHeight / BASE_SCREEN_HEIGHT;
    GLOBAL_GROUND_LEVEL = screenHeight * GROUND_Y_RATIO;
    stage->groundLevel = GLOBAL_GROUND_LEVEL;
    stage->camera.target = (Vector2){ screenWidth * 0.5f, screenHeight * 0.5f };
    stage->camera.offset = (Vector2){ screenWidth * 0.5f, screenHeight * 0.5f };
    stage->camera.zoom = STAGE1_CAMERA_ZOOM;
}

void centerWindowOnCurrentMonitor(int width, int height) {
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);
    int monitorX = GetMonitorPosition(monitor).x;
    int monitorY = GetMonitorPosition(monitor).y;

    SetWindowPosition(
        monitorX + (monitorWidth - width) / 2,
        monitorY + (monitorHeight - height) / 2
    );
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
    InitWindow(WINDOWED_WIDTH, WINDOWED_HEIGHT, "Deixa Eu");
    SetExitKey(KEY_NULL);
    toggleGameFullscreen();
    SetTargetFPS(FPS);

    // ========== INICIALIZAR STAGE 1 ==========
    Stage1 stage1;
    initStage1(&stage1);
    int stage1Initialized = 1;

    Stage3 stage3 = {0};
    int stage3Initialized = 0;

    // ========== INICIALIZAR PLAYER ==========
    int screenWidth = GetScreenWidth();
    Player player = createPlayer((Vector2){ screenWidth * 0.18f, GROUND_LEVEL }, 150, 3);
    hudHeartTexture = LoadTexture("assets/img/HealthHeart.png");

    // ========== ESTADOS DO JOGO ==========
    int isGameOver = 0;
    float gameOverTimer = 0.0f;
    float totalGameTime = 0.0f;
    int debugMode = 0;
    GameStage activeStage = GAME_STAGE_1;
    int isPaused = 0;
    int pauseSelectedOption = PAUSE_RESUME;
    float bikeDropOverlayTimer = 0.0f;
    int pendingStage3Transition = 0;

    // ========== MENU ==========
    Menu menu = createMenu();
    int inMenu = 1;

    // ========== FASES (Lista Circular) ==========
    Phase *phaseList = NULL;

    Phase *phase1 = createPhase(1, "Recife Chuvoso");
    Phase *phase3 = createPhase(3, "Torre Final");
    insertPhase(&phaseList, phase1);
    insertPhase(&phaseList, phase3);

    Phase *currentPhase = phase1;
    printf("Fase atual: %s (numero %d)\n", currentPhase->phaseName, currentPhase->phaseNumber);
    fflush(stdout);

    // ========== LOOP PRINCIPAL ==========
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // ===== ALTERAR TELA CHEIA =====
        if (IsKeyPressed(KEY_F11) || (IsKeyPressed(KEY_F) && IsKeyDown(KEY_LEFT_ALT))) {
            toggleGameFullscreen();
        }

        if (IsWindowResized() || IsKeyPressed(KEY_F11) || (IsKeyPressed(KEY_F) && IsKeyDown(KEY_LEFT_ALT))) {
            if (stage1Initialized) {
                refreshStage1Layout(&stage1);
            }
            if (activeStage == GAME_STAGE_1) {
                player.position.x = GetScreenWidth() * 0.18f;
            }
        }

        // Debug visual desativado por padrão para não conflitar com movimento em A/D.

        // ===== MENU =====
        if (inMenu) {
            updateMenu(&menu);

            if (menu.screen == MENU_MAIN && menu.confirmPressed) {
                if (menu.selectedOption == 0) {
                    // Iniciar jogo
                    inMenu = 0;
                    totalGameTime = 0.0f;
                    isGameOver = 0;
                    isPaused = 0;
                    bikeDropOverlayTimer = 0.0f;
                    pendingStage3Transition = 0;
                    activeStage = GAME_STAGE_1;
                    currentPhase = phase1;

                    // Reinicializar stage
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
                    player = createPlayer((Vector2){ currentScreenWidth * 0.18f, GROUND_LEVEL }, 150, 3);
                }
                else if (menu.selectedOption == 2) {
                    // Sair do jogo
                    break;
                }
            }
        }
        // ===== JOGO =====
        else {
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
                    Rectangle hitbox = { panelX + 52.0f, panelY + 112.0f + i * 58.0f - 8.0f, 256.0f, 44.0f };
                    if (CheckCollisionPointRec(mouse, hitbox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        clickedPauseOption = true;
                    }
                }

                if (IsKeyPressed(KEY_ENTER) || clickedPauseOption) {
                    if (pauseSelectedOption == PAUSE_RESUME) {
                        isPaused = 0;
                    } else {
                        unloadPlayerResources(&player);
                        player = createPlayer((Vector2){ GetScreenWidth() * 0.18f, GROUND_LEVEL }, 150, 3);

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
            // ===== ATUALIZAR STAGE E PLAYER =====
            if (!isGameOver && IsKeyPressed(KEY_P)) {
                skipToNextStage(&stage1,
                                &stage3,
                                &player,
                                &stage1Initialized,
                                &stage3Initialized,
                                &activeStage,
                                &currentPhase,
                                phase3,
                                &bikeDropOverlayTimer,
                                &pendingStage3Transition);
            }

            if (activeStage == GAME_STAGE_1) {
                updateStage1(&stage1, &player, deltaTime);
                updatePlayer(&player, deltaTime);

                if (stage1.stage1Complete &&
                    stage1.distanceTraveled >= STAGE1_TARGET_DISTANCE &&
                    playerReachedStage1Exit(&player) &&
                    !isGameOver &&
                    !pendingStage3Transition) {
                    pendingStage3Transition = 1;
                    bikeDropOverlayTimer = 2.2f;
                    player.velocity = (Vector2){ 0.0f, 0.0f };
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
                        printf("Fase atual: %s (numero %d)\n", currentPhase->phaseName, currentPhase->phaseNumber);
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
                    unloadPlayerResources(&player);
                    player = createPlayer((Vector2){ resetScreenWidth * 0.18f, GROUND_LEVEL }, 150, 3);
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

            // ===== VICTORY CHECK =====
            if (activeStage == GAME_STAGE_3 && stage3.state == STAGE3_FINISHED && !isGameOver) {
                if (IsKeyPressed(KEY_ENTER)) {
                    // Reset do jogo e voltar para o menu
                    int victoryScreenWidth = GetScreenWidth();
                    unloadPlayerResources(&player);
                    player = createPlayer((Vector2){ victoryScreenWidth * 0.18f, GROUND_LEVEL }, 150, 3);
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

            // ===== CONTAR TEMPO =====
            if (!isGameOver && !(activeStage == GAME_STAGE_3 && stage3.state == STAGE3_FINISHED)) {
                totalGameTime += deltaTime;
            }
            }
        }

        // ========== DESENHO ==========
        BeginDrawing();
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        if (inMenu) {
            ClearBackground((Color){ 164, 88, 48, 255 });
        } else if (activeStage == GAME_STAGE_3) {
            ClearBackground((Color){ 176, 96, 48, 255 });
        } else {
            ClearBackground(SKYBLUE);
        }

        if (inMenu) {
            // ===== DESENHAR MENU =====
            drawMenu(menu);
        }
        else {
            // ===== DESENHAR JOGO =====
            if (activeStage == GAME_STAGE_1) {
                drawStage1(&stage1, &player);

                // ===== DESENHAR HUD =====
                drawGameHUD(&stage1, &player, totalGameTime, screenWidth, screenHeight);
            } else if (activeStage == GAME_STAGE_3) {
                drawStage3(&stage3, &player);
                drawStage3HUD(&player, totalGameTime, screenWidth);
            }

            if (bikeDropOverlayTimer > 0.0f) {
                drawBikeDropOverlay(bikeDropOverlayTimer, screenWidth, screenHeight);
            }

            if (isPaused) {
                drawPauseMenu(pauseSelectedOption, screenWidth, screenHeight);
            }

            // ===== DEBUG MODE =====
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
            if (activeStage == GAME_STAGE_3 && stage3.state == STAGE3_FINISHED && !isGameOver) {
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

    CloseWindow();
    return 0;
}
