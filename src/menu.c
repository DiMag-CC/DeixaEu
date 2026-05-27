#include "menu.h"
#include <stdio.h>

static Texture2D menuBackgroundTexture = {0};

static float menuScale(void) {
    float scaleX = GetScreenWidth() / 800.0f;
    float scaleY = GetScreenHeight() / 450.0f;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    if (scale < 0.45f) return 0.45f;
    if (scale > 2.4f) return 2.4f;
    return scale;
}

static void drawTextCentered(const char *text, int y, int fontSize, Color color) {
    int screenWidth = GetScreenWidth();
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, (screenWidth - textWidth) / 2, y, fontSize, color);
}

static void drawMenuCloud(float x, float y, float scale, Color base, Color shadow, Color light) {
    DrawRectangle((int)(x + 10 * scale), (int)(y + 16 * scale), (int)(116 * scale), (int)(20 * scale), shadow);
    DrawRectangle((int)(x + 34 * scale), (int)(y + 4 * scale), (int)(86 * scale), (int)(26 * scale), base);
    DrawRectangle((int)(x + 0 * scale), (int)(y + 28 * scale), (int)(148 * scale), (int)(22 * scale), base);
    DrawRectangle((int)(x + 44 * scale), (int)(y + 12 * scale), (int)(48 * scale), (int)(10 * scale), light);
    DrawRectangle((int)(x + 105 * scale), (int)(y + 24 * scale), (int)(38 * scale), (int)(10 * scale), light);
}

static void drawTextureCover(Texture2D texture, Rectangle dest, Color tint) {
    float sourceWidth = (float)texture.width;
    float sourceHeight = (float)texture.height;
    float sourceRatio = sourceWidth / sourceHeight;
    float destRatio = dest.width / dest.height;
    Rectangle source = { 0.0f, 0.0f, sourceWidth, sourceHeight };

    if (sourceRatio > destRatio) {
        source.width = sourceHeight * destRatio;
        source.x = (sourceWidth - source.width) * 0.5f;
    } else {
        source.height = sourceWidth / destRatio;
        source.y = (sourceHeight - source.height) * 0.5f;
    }

    DrawTexturePro(texture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, tint);
}

static void drawProceduralMenuBackground(float scale) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float floorY = screenHeight * 0.78f;
    float tile = 34.0f * scale;

    DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                           (Color){ 10, 29, 60, 255 },
                           (Color){ 113, 84, 104, 255 });

    DrawCircleGradient((int)(screenWidth * 0.62f), (int)(screenHeight * 0.31f), 78.0f * scale,
                   (Color){ 229, 213, 139, 140 },
                   (Color){ 229, 213, 139, 0 });
    DrawCircle((int)(screenWidth * 0.62f), (int)(screenHeight * 0.31f), (int)(24 * scale),
               (Color){ 232, 218, 154, 235 });

    drawMenuCloud(screenWidth * 0.06f, screenHeight * 0.14f, 1.2f * scale,
                  (Color){ 45, 80, 109, 185 },
                  (Color){ 24, 48, 81, 210 },
                  (Color){ 98, 124, 146, 150 });
    drawMenuCloud(screenWidth * 0.67f, screenHeight * 0.18f, 0.95f * scale,
                  (Color){ 53, 88, 119, 170 },
                  (Color){ 30, 59, 91, 205 },
                  (Color){ 123, 120, 145, 130 });
    drawMenuCloud(screenWidth * 0.30f, screenHeight * 0.52f, 0.78f * scale,
                  (Color){ 79, 111, 132, 135 },
                  (Color){ 38, 72, 101, 160 },
                  (Color){ 143, 130, 145, 100 });

    DrawRectangle(0, (int)(floorY - 18 * scale), screenWidth, (int)(18 * scale),
                  (Color){ 56, 63, 66, 180 });
    DrawRectangle(0, (int)floorY, screenWidth, screenHeight - (int)floorY,
                  (Color){ 164, 88, 48, 255 });

    int startCol = -2;
    int cols = (int)(screenWidth / tile) + 5;
    for (int row = 0; row < 5; row++) {
        for (int col = startCol; col < cols; col++) {
            float x = col * tile + ((row % 2) ? tile * 0.5f : 0.0f);
            float y = floorY + row * tile * 0.58f;
            int pattern = (row + col) % 3;
            if (pattern < 0) pattern += 3;
            Color color = (pattern == 0) ? (Color){ 191, 101, 49, 255 } :
                          (pattern == 1) ? (Color){ 217, 130, 62, 255 } :
                                           (Color){ 117, 99, 61, 255 };
            DrawRectangle((int)x, (int)y, (int)(tile - 2), (int)(tile * 0.58f - 1), color);
            DrawRectangleLines((int)x, (int)y, (int)(tile - 2), (int)(tile * 0.58f - 1),
                               (Color){ 87, 55, 45, 170 });
        }
    }

    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 5, 8, 24, 58 });
}

static void drawMenuBackground(float scale) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    if (menuBackgroundTexture.id == 0) {
        menuBackgroundTexture = LoadTexture("assets/img/favelinha.png");
        if (menuBackgroundTexture.id > 0) {
            SetTextureFilter(menuBackgroundTexture, TEXTURE_FILTER_POINT);
        }
    }

    if (menuBackgroundTexture.id > 0) {
        drawTextureCover(menuBackgroundTexture,
                         (Rectangle){ 0.0f, 0.0f, (float)screenWidth, (float)screenHeight },
                         WHITE);
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 2, 7, 18, 116 });
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){ 0, 0, 0, 72 },
                               (Color){ 0, 0, 0, 160 });
    } else {
        drawProceduralMenuBackground(scale);
    }
}

void unloadMenuResources(void) {
    if (menuBackgroundTexture.id > 0) {
        UnloadTexture(menuBackgroundTexture);
        menuBackgroundTexture = (Texture2D){0};
    }
}

Menu createMenu() {
    Menu menu;
    menu.screen = MENU_MAIN;
    menu.selectedOption = 0;
    return menu;
}

void updateMenu(Menu *menu) {
    if (menu->screen == MENU_MAIN) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            menu->selectedOption--;
        }

        if (menu->selectedOption < 0) menu->selectedOption = 3;

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            menu->selectedOption++;
        }

        if (menu->selectedOption > 3) menu->selectedOption = 0;

        if (IsKeyPressed(KEY_ENTER)) {
            if (menu->selectedOption == 1) {
                menu->screen = MENU_RANKING;
            } else if (menu->selectedOption == 2) {
                menu->screen = MENU_CREDITS;
            }
        }
    }
    else if (menu->screen == MENU_RANKING) {
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
            menu->screen = MENU_MAIN;
            menu->selectedOption = 1;
        }
    }
    else if (menu->screen == MENU_CREDITS) {
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
            menu->screen = MENU_MAIN;
            menu->selectedOption = 2; // Cursor retorna posicionado na opção "Creditos"
        }
    }
}

void drawMenu(Menu menu) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float scale = menuScale();
    
    drawMenuBackground(scale);
    
    if (menu.screen == MENU_MAIN) {
        int titleSize = (int)(80 * scale);
        int subtitleSize = (int)(24 * scale);
        int optionSize = (int)(40 * scale);
        int hintSize = (int)(16 * scale);
        int titleY = (int)(40 * scale);
        int subtitleY = titleY + (int)(95 * scale);
        int optionsStartY = subtitleY + (int)(105 * scale);
        int optionGap = (int)(65 * scale);
        
        if (screenHeight < 420) {
            optionsStartY = subtitleY + (int)(75 * scale);
            optionGap = (int)(50 * scale);
        }

        DrawRectangleRounded((Rectangle){
                                 screenWidth * 0.5f - 255.0f * scale,
                                 titleY - 18.0f * scale,
                                 510.0f * scale,
                                 screenHeight - titleY - 56.0f * scale
                             },
                             0.08f, 8, (Color){ 3, 12, 27, 150 });
        
        drawTextCentered("DEIXA EU", titleY, titleSize, YELLOW);
        drawTextCentered("Fuja de casa pela cidade do Recife!", subtitleY, subtitleSize, LIGHTGRAY);
        
        const char *options[] = { "Iniciar Jogo", "Ranking", "Creditos", "Sair" };
        
        for (int i = 0; i < 4; i++) {
            Color textColor = (menu.selectedOption == i) ? YELLOW : WHITE;
            int optionY = optionsStartY + (i * optionGap);
            int optionWidth = MeasureText(options[i], optionSize);
            int optionX = (screenWidth - optionWidth) / 2;
            
            if (menu.selectedOption == i) {
                DrawText(">", optionX - (int)(42 * scale), optionY, optionSize, YELLOW);
            }
            
            DrawText(options[i], optionX, optionY, optionSize, textColor);
        }
        
        drawTextCentered("Use W/S ou Setas para navegar | ENTER para confirmar", 
                         screenHeight - (int)(36 * scale), hintSize, GRAY);
    }
    else if (menu.screen == MENU_RANKING) {
        int titleSize = (int)(60 * scale);
        int hintSize = (int)(16 * scale);
        int titleY = (int)(35 * scale);

        DrawRectangleRounded((Rectangle){
                                 screenWidth * 0.16f,
                                 titleY - 16.0f * scale,
                                 screenWidth * 0.68f,
                                 screenHeight * 0.70f
                             },
                             0.08f, 8, (Color){ 3, 12, 27, 140 });
        drawTextCentered("RANKING", titleY, titleSize, YELLOW);
        drawTextCentered("Pressione ESC para voltar", screenHeight - (int)(34 * scale), hintSize, GRAY);
    }
    else if (menu.screen == MENU_CREDITS) {
        int titleSize = (int)(60 * scale);
        int textSize = (int)(20 * scale);
        int nameSize = (int)(18 * scale);
        int hintSize = (int)(16 * scale);
        int titleY = (int)(35 * scale);
        int y = titleY + (int)(95 * scale);
        int lineGap = (int)(30 * scale);
        
        DrawRectangleRounded((Rectangle){
                                 screenWidth * 0.5f - 280.0f * scale,
                                 titleY - 16.0f * scale,
                                 560.0f * scale,
                                 screenHeight - titleY - 58.0f * scale
                             },
                             0.08f, 8, (Color){ 3, 12, 27, 145 });
        drawTextCentered("CREDITOS", titleY, titleSize, YELLOW);
        drawTextCentered("Desenvolvido por:", y, textSize, WHITE);
        y += lineGap + (int)(10 * scale);
        drawTextCentered("Arthur Moury", y, nameSize, LIGHTGRAY);
        y += lineGap;
        drawTextCentered("Diego Magnata", y, nameSize, LIGHTGRAY);
        y += lineGap;
        drawTextCentered("Luiza Barbosa", y, nameSize, LIGHTGRAY);
        y += lineGap;
        drawTextCentered("Helio de Moraes", y, nameSize, LIGHTGRAY);
        y += lineGap;
        drawTextCentered("Maria Augusta", y, nameSize, LIGHTGRAY);
        
        drawTextCentered("Inspiracao: Super Mario + Subway Surfers", screenHeight - (int)(92 * scale), nameSize, LIGHTGRAY);
        drawTextCentered("Deixa Eu", screenHeight - (int)(64 * scale), nameSize, LIGHTGRAY);
        
        drawTextCentered("Pressione ESC para fechar", screenHeight - (int)(34 * scale), hintSize, GRAY);
    }
}
