#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "steps/stage1.h"
#include "steps/stage2.h"

#include "steps/stage3.h"
#include "entities/player.h"
#include "menu.h"
#include "structure/stepList.h"

#define SCREEN_WIDTH  1920.0f
#define SCREEN_HEIGHT 1080.0f
#define WORLD_WIDTH   800.0f
#define WORLD_HEIGHT  450.0f
#define CAMERA_VERTICAL_LOOKAHEAD 150.0f
#define FPS 60
#define MAX_RANKING_ENTRIES 10
#define RANK_STAGE2_SAND_DISTANCE 1500.0f
#define RANK_STAGE2_SEA_DISTANCE 600.0f

// ========== ESTADO GLOBAL DO JOGO ==========
typedef enum {
    STATE_INTRO = 0,
    STATE_MENU,
    STATE_STAGE1,
    STATE_TRANSITION_12,   // imagem12.png
    STATE_STAGE2,
    STATE_TRANSITION_23,   // tela raylib: fase final
    STATE_STAGE3,
    STATE_GAMEOVER
} GameState;

typedef enum {
    MUSIC_NONE = -1,
    MUSIC_STAGE1 = 0,
    MUSIC_STAGE2_SAND,
    MUSIC_STAGE2_SEA,
    MUSIC_STAGE3,
    MUSIC_WIN,
    MUSIC_COUNT
} MusicTrack;

typedef struct {
    char playerName[32];
    float duration;
    float distance;
} RankingEntry;

// ========== TEXTURAS DA INTRODUÇÃO ==========
static Texture2D txIntroBg;
static Texture2D txIntroPlayer;
static Texture2D txFinalScreen;
static Texture2D txHudHeart;
static int introTexturesLoaded = 0;
static int finalScreenLoaded = 0;
static int hudHeartLoaded = 0;

static Music musicTracks[MUSIC_COUNT];
static int musicTracksLoaded = 0;
static MusicTrack currentMusicTrack = MUSIC_NONE;
static RankingEntry rankingEntries[MAX_RANKING_ENTRIES];
static int rankingCount = 0;
static int nextPlayerNumber = 1;

static int rankingComesBefore(RankingEntry a, RankingEntry b) {
    if (a.distance > b.distance) return 1;
    if (a.distance < b.distance) return 0;
    return a.duration < b.duration;
}

static void insertionSortRanking(void) {
    for (int i = 1; i < rankingCount; i++) {
        RankingEntry key = rankingEntries[i];
        int j = i - 1;

        while (j >= 0 && rankingComesBefore(key, rankingEntries[j])) {
            rankingEntries[j + 1] = rankingEntries[j];
            j--;
        }

        rankingEntries[j + 1] = key;
    }
}

static void addRankingEntry(const char *playerName, float duration, float distance) {
    RankingEntry entry = {0};
    snprintf(entry.playerName, sizeof(entry.playerName), "%s", playerName);
    entry.duration = duration;
    entry.distance = distance;

    if (rankingCount < MAX_RANKING_ENTRIES) {
        rankingEntries[rankingCount++] = entry;
    } else if (rankingComesBefore(entry, rankingEntries[MAX_RANKING_ENTRIES - 1])) {
        rankingEntries[MAX_RANKING_ENTRIES - 1] = entry;
    } else {
        return;
    }

    insertionSortRanking();
}

static float calculateReachedDistance(GameState gameState, Stage1 *stage1, Stage2 *stage2, Stage3 *stage3, Player *player) {
    float stage1Distance = stage1->distanceTraveled;
    if (stage1Distance > STAGE1_TARGET_DISTANCE) stage1Distance = STAGE1_TARGET_DISTANCE;

    switch (gameState) {
    case STATE_STAGE1:
        return stage1Distance;
    case STATE_TRANSITION_12:
        return STAGE1_TARGET_DISTANCE;
    case STATE_STAGE2:
        if (stage2->mode == STAGE2_MODE_SAND) {
            return STAGE1_TARGET_DISTANCE + stage2->distanceTraveled;
        }
        if (stage2->mode == STAGE2_MODE_TRANSITION) {
            return STAGE1_TARGET_DISTANCE + RANK_STAGE2_SAND_DISTANCE;
        }
        return STAGE1_TARGET_DISTANCE + RANK_STAGE2_SAND_DISTANCE + stage2->distanceTraveled;
    case STATE_TRANSITION_23:
        return STAGE1_TARGET_DISTANCE + RANK_STAGE2_SAND_DISTANCE + RANK_STAGE2_SEA_DISTANCE;
    case STATE_STAGE3: {
        float stage3Distance = stage3->scrollX;
        if (stage3->state == STAGE3_CLIMBING || stage3->state == STAGE3_FINISHED) {
            stage3Distance += fmaxf(0.0f, GetScreenHeight() * 0.82f - player->position.y);
        }
        return STAGE1_TARGET_DISTANCE + RANK_STAGE2_SAND_DISTANCE + RANK_STAGE2_SEA_DISTANCE + stage3Distance;
    }
    default:
        return 0.0f;
    }
}

static void drawRankingEntries(int screenWidth, int screenHeight) {
    int fontSize = screenHeight < 720 ? 18 : 24;
    int headerSize = screenHeight < 720 ? 20 : 26;
    int tableWidth = (int)(screenWidth * 0.68f);
    int startX = (screenWidth - tableWidth) / 2;
    int startY = (int)(screenHeight * 0.24f);
    int rowHeight = fontSize + 18;

    DrawRectangleRounded((Rectangle){ (float)startX - 18.0f, (float)startY - 18.0f,
                                      (float)tableWidth + 36.0f, (float)(rowHeight * 8 + 42) },
                         0.08f, 6, (Color){ 8, 20, 34, 170 });
    DrawText("#", startX, startY, headerSize, YELLOW);
    DrawText("Jogador", startX + 70, startY, headerSize, YELLOW);
    DrawText("Tempo", startX + tableWidth - 310, startY, headerSize, YELLOW);
    DrawText("Distancia", startX + tableWidth - 155, startY, headerSize, YELLOW);

    if (rankingCount == 0) {
        const char *emptyText = "Nenhuma partida registrada";
        int textWidth = MeasureText(emptyText, fontSize);
        DrawText(emptyText, (screenWidth - textWidth) / 2, startY + rowHeight * 2, fontSize, LIGHTGRAY);
        return;
    }

    int rows = rankingCount < 8 ? rankingCount : 8;
    for (int i = 0; i < rows; i++) {
        char rank[16];
        char timeText[32];
        char distanceText[32];
        int y = startY + rowHeight * (i + 1);

        snprintf(rank, sizeof(rank), "%d", i + 1);
        snprintf(timeText, sizeof(timeText), "%.1fs", rankingEntries[i].duration);
        snprintf(distanceText, sizeof(distanceText), "%.0fm", rankingEntries[i].distance);

        Color rowColor = i == 0 ? (Color){ 255, 238, 122, 255 } : RAYWHITE;
        DrawText(rank, startX, y, fontSize, rowColor);
        DrawText(rankingEntries[i].playerName, startX + 70, y, fontSize, rowColor);
        DrawText(timeText, startX + tableWidth - 310, y, fontSize, rowColor);
        DrawText(distanceText, startX + tableWidth - 155, y, fontSize, rowColor);
    }
}

static void loadGameMusic(void) {
    if (musicTracksLoaded) return;

    musicTracks[MUSIC_STAGE1] = LoadMusicStream("assets/music/sambaSongLevel1.wav");
    musicTracks[MUSIC_STAGE2_SAND] = LoadMusicStream("assets/music/BossaNOva1.wav");
    musicTracks[MUSIC_STAGE2_SEA] = LoadMusicStream("assets/music/BolhasFundodoMar.wav");
    musicTracks[MUSIC_STAGE3] = LoadMusicStream("assets/music/DramaFrevo1.wav");
    musicTracks[MUSIC_WIN] = LoadMusicStream("assets/music/Win1.mp3");

    for (int i = 0; i < MUSIC_COUNT; i++) {
        musicTracks[i].looping = true;
        SetMusicVolume(musicTracks[i], 0.55f);
    }

    musicTracksLoaded = 1;
}

static void unloadGameMusic(void) {
    if (!musicTracksLoaded) return;

    if (currentMusicTrack != MUSIC_NONE) {
        StopMusicStream(musicTracks[currentMusicTrack]);
        currentMusicTrack = MUSIC_NONE;
    }

    for (int i = 0; i < MUSIC_COUNT; i++) {
        UnloadMusicStream(musicTracks[i]);
    }

    musicTracksLoaded = 0;
}

static void playGameMusic(MusicTrack track) {
    if (!musicTracksLoaded || track == currentMusicTrack) return;

    if (currentMusicTrack != MUSIC_NONE) {
        StopMusicStream(musicTracks[currentMusicTrack]);
    }

    currentMusicTrack = track;
    if (currentMusicTrack != MUSIC_NONE) {
        PlayMusicStream(musicTracks[currentMusicTrack]);
    }
}

static void updateGameMusic(void) {
    if (musicTracksLoaded && currentMusicTrack != MUSIC_NONE) {
        UpdateMusicStream(musicTracks[currentMusicTrack]);
    }
}

static void drawTextureCover(Texture2D texture, Rectangle dest, Color tint) {
    if (texture.id <= 0 || texture.width <= 0 || texture.height <= 0) return;

    float sourceAspect = (float)texture.width / (float)texture.height;
    float destAspect = dest.width / dest.height;
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };

    if (sourceAspect > destAspect) {
        source.width = texture.height * destAspect;
        source.x = ((float)texture.width - source.width) * 0.5f;
    } else {
        source.height = texture.width / destAspect;
        source.y = ((float)texture.height - source.height) * 0.5f;
    }

    DrawTexturePro(texture, source, dest, (Vector2){0,0}, 0.0f, tint);
}

static void unloadFinalScreenTexture(void) {
    if (finalScreenLoaded) {
        UnloadTexture(txFinalScreen);
        txFinalScreen = (Texture2D){0};
        finalScreenLoaded = 0;
    }
}

static void drawFinalVictoryScreen(int screenWidth, int screenHeight) {
    if (!finalScreenLoaded) {
        txFinalScreen = LoadTexture("assets/img/telafinal.png");
        finalScreenLoaded = 1;
    }

    if (txFinalScreen.id > 0) {
        drawTextureCover(txFinalScreen, (Rectangle){0, 0, (float)screenWidth, (float)screenHeight}, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){ 247, 171, 73, 255 },
                               (Color){ 29, 102, 137, 255 });
    }

    const char *message = "Valeu a pena, mãe";
    int fontSize = screenHeight < 720 ? 42 : 64;
    int textWidth = MeasureText(message, fontSize);
    int x = (screenWidth - textWidth) / 2;
    int y = (int)(screenHeight * 0.14f);

    DrawText(message, x + 3, y + 3, fontSize, (Color){ 0, 37, 58, 165 });
    DrawText(message, x, y, fontSize, (Color){ 255, 245, 174, 255 });
}

static void unloadHudHeartTexture(void) {
    if (hudHeartLoaded) {
        UnloadTexture(txHudHeart);
        txHudHeart = (Texture2D){0};
        hudHeartLoaded = 0;
    }
}

static void drawSharedLifeScoreTimeHUD(Player *player, float totalGameTime, int screenWidth, int screenHeight) {
    (void)screenWidth;

    if (!hudHeartLoaded) {
        txHudHeart = LoadTexture("assets/img/HealthHeart.png");
        hudHeartLoaded = 1;
    }

    float scale = (float)screenHeight / 1080.0f;
    if (scale < 0.72f) scale = 0.72f;
    if (scale > 1.25f) scale = 1.25f;
    float margin = 16.0f * scale;
    float heartSize = 36.0f * scale;
    float gap = 7.0f * scale;
    int fontSize = (int)(22.0f * scale);
    int lives = player->lives;
    if (lives < 0) lives = 0;
    if (lives > 3) lives = 3;

    DrawRectangleRounded((Rectangle){ margin - 8.0f, margin - 8.0f, 360.0f * scale, 92.0f * scale },
                         0.10f, 6, (Color){ 6, 18, 31, 145 });

    for (int i = 0; i < 3; i++) {
        Rectangle dest = {
            margin + i * (heartSize + gap),
            margin,
            heartSize,
            heartSize
        };
        Color tint = i < lives ? WHITE : (Color){ 255, 255, 255, 65 };

        if (txHudHeart.id > 0) {
            Rectangle source = { 0.0f, 0.0f, (float)txHudHeart.width, (float)txHudHeart.height };
            DrawTexturePro(txHudHeart, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, tint);
        } else {
            DrawCircle((int)(dest.x + dest.width * 0.30f), (int)(dest.y + dest.height * 0.34f), dest.width * 0.22f, RED);
            DrawCircle((int)(dest.x + dest.width * 0.70f), (int)(dest.y + dest.height * 0.34f), dest.width * 0.22f, RED);
            DrawTriangle(
                (Vector2){ dest.x + dest.width * 0.10f, dest.y + dest.height * 0.42f },
                (Vector2){ dest.x + dest.width * 0.90f, dest.y + dest.height * 0.42f },
                (Vector2){ dest.x + dest.width * 0.50f, dest.y + dest.height * 0.96f },
                RED
            );
        }
    }

    char pointsText[64];
    char timeText[64];
    snprintf(pointsText, sizeof(pointsText), "Pontos: %.0f", player->score);
    snprintf(timeText, sizeof(timeText), "Tempo: %.1fs", totalGameTime);
    DrawText(pointsText, (int)margin, (int)(margin + heartSize + 9.0f * scale), fontSize, RAYWHITE);
    DrawText(timeText, (int)(margin + 170.0f * scale), (int)(margin + heartSize + 9.0f * scale), fontSize, RAYWHITE);
}

// ========== HUD FASE 1 ==========
void drawGameHUD(Stage1 *stage, Player *player, float totalGameTime, int screenWidth, int screenHeight) {
    const int HUD_Y_START  = 12;
    const int HUD_Y_STEP   = 25;
    const int HUD_MARGIN   = 12;
    int fontSize = (screenWidth < 1024) ? 14 : 16;

    drawSharedLifeScoreTimeHUD(player, totalGameTime, screenWidth, screenHeight);

    char buf[128];

    sprintf(buf, "Dificuldade: x%.1f", stage->difficultyMultiplier);
    DrawText(buf, HUD_MARGIN, HUD_Y_START + HUD_Y_STEP * 4, fontSize, DARKBLUE);

    if (player->hasUmbrella > 0) {
        sprintf(buf, "Protecao: %.1f s", player->umbrellaTimer);
        int px = screenWidth - 250;
        DrawText(buf, px, HUD_Y_START, 16, GREEN);
        int bw = 150;
        float bp = player->umbrellaTimer / 5.0f;
        if (bp > 1.0f) bp = 1.0f;
        DrawRectangle(px, HUD_Y_START + 25, bw, 10, LIGHTGRAY);
        DrawRectangle(px, HUD_Y_START + 25, (int)(bw * bp), 10, GREEN);
        DrawRectangleLinesEx((Rectangle){(float)px, (float)HUD_Y_START + 25, (float)bw, 10.0f}, 1, BLACK);
    }

    float pp = stage->distanceTraveled / STAGE1_TARGET_DISTANCE;
    if (pp > 1.0f) pp = 1.0f;
    int pby = screenHeight - 40;
    int pbw = screenWidth - 20;
    sprintf(buf, "Progresso: %.0f / %.0f m", stage->distanceTraveled, STAGE1_TARGET_DISTANCE);
    DrawText(buf, HUD_MARGIN, pby - 25, 14, BLACK);
    DrawRectangle(HUD_MARGIN, pby, pbw, 20, LIGHTGRAY);
    DrawRectangle(HUD_MARGIN, pby, (int)(pbw * pp), 20, GREEN);
    DrawRectangleLinesEx((Rectangle){(float)HUD_MARGIN, (float)pby, (float)pbw, 20.0f}, 2, BLACK);
}

// ========== DEBUG PLAYER ==========
void drawPlayerDebug(Player player) {
    DrawRectangleLinesEx(player.hitbox, 1, RED);
    DrawCircle((int)player.position.x, (int)player.position.y, 3, GREEN);
    char dbg[256];
    sprintf(dbg, "Player: (%.0f, %.0f) | Vel: (%.1f, %.1f) | Speed: %.0f | Lives: %d",
            player.position.x, player.position.y,
            player.velocity.x, player.velocity.y,
            player.speed, player.lives);
    DrawText(dbg, 10, 80, 14, BLACK);
}

// ========== INTRO CINEMATOGRÁFICA ==========
void drawStoryIntroScreen(float storyTime, int screenWidth, int screenHeight) {
    if (!introTexturesLoaded) {
        txIntroBg     = LoadTexture("assets/img/landscapeFase1New2.png");
        txIntroPlayer = LoadTexture("assets/img/CharacterStandingR.png");
        introTexturesLoaded = 1;
    }

    if (txIntroBg.id > 0) {
        drawTextureCover(txIntroBg,
                         (Rectangle){0,0,(float)screenWidth,(float)screenHeight},
                         (Color){110,110,125,255});
    } else {
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){15,20,35,255});
    }

    if (txIntroPlayer.id > 0) {
        float stretch = sinf(storyTime * 3.5f) * 4.0f;
        float pw = 140.0f, ph = 175.0f + stretch;
        DrawTexturePro(txIntroPlayer,
                       (Rectangle){0,0,(float)txIntroPlayer.width,(float)txIntroPlayer.height},
                       (Rectangle){60.0f, screenHeight * 0.86f - ph * 0.5f, pw, ph},
                       (Vector2){0,0}, 0.0f, WHITE);
    }

    int bx = 260, bw = screenWidth - 320, bh = 130, px = 30, py = 25;
    const char *tA[] = {
        "Minha mãe sempre diz o que eu devo fazer, para onde devo ir...",
        "Mas eu cansei de apenas assistir à vida passar pela janela.",
        "Lá fora, a tempestade urbana está mais forte e poluída do que nunca.",
        "Minha jornada me levará além do asfalto, cruzando as praias"
    };
    const char *tB[] = {
        "\"Não saia na chuva\", \"A cidade é perigosa\".",
        "Hoje eu vou descobrir o recife por conta própria. DEIXA EU!",
        "Para conquistar minha liberdade, precisarei ser mais rápido que o trânsito.",
        "e mergulhando nas profundezas de um oceano poluído. A aventura começa agora..."
    };
    Color borderColors[] = {
        (Color){102,191,255,255}, (Color){253,249,0,255},
        (Color){255,161,0,255},   (Color){0,228,48,255}
    };
    Color bgColors[] = {
        (Color){40,55,75,220}, (Color){50,50,50,220},
        (Color){70,45,30,220}, (Color){35,60,45,220}
    };
    float startTimes[] = { 0.0f, 4.0f, 9.0f, 14.0f };
    int boxYs[] = { 60, 220, 380, 540 };

    char bufA[256], bufB[256];
    for (int i = 0; i < 4; i++) {
        if (storyTime < startTimes[i]) break;
        float lt = storyTime - startTimes[i];
        float a  = lt * 2.0f; if (a > 1.0f) a = 1.0f;
        int by = boxYs[i] + (int)((1.0f - a) * 15.0f);
        DrawRectangleRounded((Rectangle){(float)bx,(float)by,(float)bw,(float)bh}, 0.15f, 4,
                             (Color){bgColors[i].r,bgColors[i].g,bgColors[i].b,(unsigned char)(a*220)});
        DrawRectangleRoundedLinesEx((Rectangle){(float)bx,(float)by,(float)bw,(float)bh}, 0.15f, 4, 3.0f,
                                    (Color){borderColors[i].r,borderColors[i].g,borderColors[i].b,(unsigned char)(a*255)});
        int cA = (int)(lt * 45.0f); int lA = strlen(tA[i]); if (cA > lA) cA = lA;
        strncpy(bufA, tA[i], cA); bufA[cA] = '\0';
        float ltB = lt - (i < 2 ? 1.0f : 1.2f);
        int cB = (ltB > 0) ? (int)(ltB * 45.0f) : 0; int lB = strlen(tB[i]); if (cB > lB) cB = lB;
        strncpy(bufB, tB[i], cB); bufB[cB] = '\0';
        DrawText(bufA, bx+px, by+py, 26, (Color){255,255,255,(unsigned char)(a*255)});
        if (cB > 0) DrawText(bufB, bx+px, by+py+40, 26, (Color){borderColors[i].r,borderColors[i].g,borderColors[i].b,(unsigned char)(a*255)});
    }

    float pulse = (sinf(storyTime * 4.0f) + 1.0f) / 2.0f;
    DrawRectangleRounded((Rectangle){(float)(screenWidth-280),30,250,45}, 0.2f, 4, (Color){20,20,20,180});
    DrawText("Pular Cena (ENTER)", screenWidth-240, 42, 18,
             (Color){255,255,255,(unsigned char)(200 + pulse * 55)});
    float tp = storyTime / 20.0f; if (tp > 1.0f) tp = 1.0f;
    DrawText("PROGRESSO DA INTRODUÇÃO:", screenWidth-360, screenHeight-95, 16, LIGHTGRAY);
    DrawRectangle(screenWidth-360, screenHeight-70, 320, 20, DARKGRAY);
    DrawRectangle(screenWidth-360, screenHeight-70, (int)(320*tp), 20, GREEN);
    DrawRectangleLines(screenWidth-360, screenHeight-70, 320, 20, WHITE);
}

// ========== TRANSIÇÃO SIMPLES (fase 2→3) ==========
typedef struct {
    Texture2D image;
    float     timer;
    float     fadeDuration;
    float     holdDuration;
    int       loaded;
} TransitionScreen;

static TransitionScreen transition = {0};

static void initTransition(const char *imagePath, float fade, float hold) {
    if (transition.loaded && transition.image.id > 0) { UnloadTexture(transition.image); }
    transition.loaded = 0;
    transition.image        = (Texture2D){0};
    if (imagePath != NULL && imagePath[0] != '\0') {
        transition.image = LoadTexture(imagePath);
    }
    transition.timer        = 0.0f;
    transition.fadeDuration = fade;
    transition.holdDuration = hold;
    transition.loaded       = 1;
}

static int updateTransition(float deltaTime) {
    transition.timer += deltaTime;
    float total = transition.fadeDuration * 2.0f + transition.holdDuration;
    return (transition.timer >= total) ? 1 : 0;
}

static void drawTransitionScreen(int screenWidth, int screenHeight) {
    float fade = transition.fadeDuration, hold = transition.holdDuration, t = transition.timer;
    float alpha = (t < fade) ? t/fade : (t < fade+hold) ? 1.0f : 1.0f-(t-fade-hold)/fade;
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    unsigned char a = (unsigned char)(alpha * 255.0f);
    if (transition.image.id > 0) {
        ClearBackground(BLACK);
        drawTextureCover(transition.image,
                         (Rectangle){0,0,(float)screenWidth,(float)screenHeight},
                         (Color){255,255,255,a});
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 0, 0, 0, (unsigned char)(95.0f * alpha) });
    } else {
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){ 13, 47, 75, 255 },
                               (Color){ 10, 21, 43, 255 });
    }

    const char *title = "Fase Final";
    const char *subtitle = "O Parque das Esculturas";
    int titleSize = screenHeight < 720 ? 54 : 78;
    int subtitleSize = screenHeight < 720 ? 30 : 42;
    int titleWidth = MeasureText(title, titleSize);
    int subtitleWidth = MeasureText(subtitle, subtitleSize);
    Color titleColor = (Color){ 255, 245, 166, a };
    Color subtitleColor = (Color){ 236, 248, 255, a };
    float panelWidth = fmaxf((float)titleWidth, (float)subtitleWidth) + screenWidth * 0.08f;
    float panelHeight = (float)(titleSize + subtitleSize) + screenHeight * 0.10f;
    float panelX = screenWidth * 0.5f - panelWidth * 0.5f;
    float panelY = screenHeight * 0.34f - screenHeight * 0.035f;

    DrawRectangleRounded((Rectangle){ panelX, panelY, panelWidth, panelHeight },
                         0.10f, 8, (Color){ 2, 9, 20, (unsigned char)(178.0f * alpha) });
    DrawRectangleRoundedLines((Rectangle){ panelX, panelY, panelWidth, panelHeight },
                              0.10f, 8, (Color){ 255, 245, 166, (unsigned char)(95.0f * alpha) });

    DrawText(title, (screenWidth - titleWidth) / 2 + 3, (int)(screenHeight * 0.34f) + 3,
             titleSize, (Color){ 0, 0, 0, (unsigned char)(130.0f * alpha) });
    DrawText(title, (screenWidth - titleWidth) / 2, (int)(screenHeight * 0.34f), titleSize, titleColor);
    DrawText(subtitle, (screenWidth - subtitleWidth) / 2 + 2,
             (int)(screenHeight * 0.34f) + titleSize + 20,
             subtitleSize, (Color){ 0, 0, 0, (unsigned char)(135.0f * alpha) });
    DrawText(subtitle, (screenWidth - subtitleWidth) / 2, (int)(screenHeight * 0.34f) + titleSize + 18,
             subtitleSize, subtitleColor);
}

// ========== CUTSCENE ANIMADA: TRANSIÇÃO FASE 1 → FASE 2 ==========
//
// Tudo autocontido, sem depender do estado da fase 1.
//
// 0.0s – 0.4s : fade-in do fundo da praia (landscapeLevel2.png)
// 0.4s – 1.6s : personagem faz arco parabólico de pulo (CharacterJumpingR.png)
//               entra pela esquerda, pico no centro, aterra em 30% da tela
// 1.6s – 4.2s : personagem corre de 30% até 18% (posição inicial fase 2)
//               (characterMovingR1.png), desacelerando ao chegar
// 4.2s         : fase 2 inicia

#define CS12_FADE_END    0.4f
#define CS12_JUMP_END    1.6f
#define CS12_RUN_END     4.2f

#define CS12_CHAR_W      140.0f
#define CS12_CHAR_H      158.0f
#define CS12_START_X_RATIO  0.30f   // onde o personagem aterra após o pulo
#define CS12_TARGET_X_RATIO 1.05f   // sai pela direita da tela

typedef struct {
    Texture2D txBg;
    Texture2D txJump;
    Texture2D txRun;
    float     timer;
    int       loaded;
} Cutscene12;

static Cutscene12 cs12 = {0};

static void initCutscene12(void) {
    if (cs12.loaded) {
        UnloadTexture(cs12.txBg);
        UnloadTexture(cs12.txJump);
        UnloadTexture(cs12.txRun);
    }
    cs12.txBg   = LoadTexture("assets/img/landscapeLevel2.png");
    cs12.txJump = LoadTexture("assets/img/CharacterJumpingR.png");
    cs12.txRun  = LoadTexture("assets/img/characterMovingR1.png");
    cs12.timer  = 0.0f;
    cs12.loaded = 1;
}

static void unloadCutscene12(void) {
    if (!cs12.loaded) return;
    UnloadTexture(cs12.txBg);
    UnloadTexture(cs12.txJump);
    UnloadTexture(cs12.txRun);
    cs12.loaded = 0;
}

static int updateCutscene12(float deltaTime) {
    cs12.timer += deltaTime;
    return (cs12.timer >= CS12_RUN_END) ? 1 : 0;
}

static void drawCutscene12(int screenWidth, int screenHeight) {
    float t       = cs12.timer;
    float groundY = (float)screenHeight * 0.82f;

    // --- alpha do fundo: fade-in rápido ---
    float bgAlpha = (t < CS12_FADE_END) ? (t / CS12_FADE_END) : 1.0f;
    unsigned char ba = (unsigned char)(bgAlpha * 255.0f);

    ClearBackground(BLACK);

    // fundo da praia
    if (cs12.txBg.id > 0) {
        drawTextureCover(cs12.txBg,
                         (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                         (Color){255, 255, 255, ba});
    } else {
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                               (Color){80, 160, 220, ba}, (Color){220, 200, 140, ba});
    }

    float charX = 0.0f, charY = 0.0f;
    Texture2D *tx = NULL;

    if (t < CS12_JUMP_END) {
        // ---- PULO: arco parabólico entrando pela esquerda ----
        float jumpStart = CS12_FADE_END;
        float p = (t - jumpStart) / (CS12_JUMP_END - jumpStart);
        if (p < 0.0f) p = 0.0f;
        if (p > 1.0f) p = 1.0f;

        float startX    = -(CS12_CHAR_W);                          // fora da tela à esquerda
        float landX     = (float)screenWidth * CS12_START_X_RATIO; // onde aterra
        charX = startX + (landX - startX) * p;

        float jumpHeight = (float)screenHeight * 0.14f;
        charY = groundY - CS12_CHAR_H - jumpHeight * 4.0f * p * (1.0f - p);
        tx = &cs12.txJump;

    } else {
        // ---- CORRIDA: de 30% até 18%, desacelerando ----
        float p = (t - CS12_JUMP_END) / (CS12_RUN_END - CS12_JUMP_END);
        if (p > 1.0f) p = 1.0f;

        // velocidade constante até sumir pela direita
        float startX = (float)screenWidth * CS12_START_X_RATIO;
        float endX   = (float)screenWidth * CS12_TARGET_X_RATIO;
        charX = startX + (endX - startX) * p;

        float bob = sinf(t * 18.0f) * 3.5f;
        charY = groundY - CS12_CHAR_H + bob;
        tx = &cs12.txRun;
    }

    if (tx && tx->id > 0)
        DrawTexturePro(*tx,
                       (Rectangle){0, 0, (float)tx->width, (float)tx->height},
                       (Rectangle){charX, charY, CS12_CHAR_W, CS12_CHAR_H},
                       (Vector2){0, 0}, 0.0f, (Color){255, 255, 255, ba});
}

// ========== MAIN ==========
int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Deixa Eu");
    InitAudioDevice();
    loadGameMusic();
    int monitor = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
    ToggleFullscreen();
    SetTargetFPS(FPS);

    // ===== FASES =====
    Stage1 stage1; initStage1(&stage1);
    Stage2 stage2;
    Stage3 stage3;

    // ===== PLAYER =====
    int sw = GetScreenWidth();
    Player player = createPlayer((Vector2){ sw * 0.18f, GROUND_LEVEL }, 150, 3);

    // ===== ESTADO =====
    GameState gameState    = STATE_INTRO;
    int       debugMode    = 0;
    float     totalGameTime = 0.0f;
    float     storyTimer   = 0.0f;
    float     gameOverTimer = 0.0f;
    float     finalReturnTimer = 0.0f;
    int       matchRecorded = 0;
    char      currentPlayerName[32] = "Jogador 1";
    Menu      menu          = createMenu();

    // ===== LISTA DE FASES =====
    Phase *phaseList = NULL;
    Phase *phase1    = createPhase(1, "Recife Chuvoso");
    Phase *phase2    = createPhase(2, "Boa Viagem");
    Phase *phase3    = createPhase(3, "Parque das Esculturas");
    insertPhase(&phaseList, phase1);
    insertPhase(&phaseList, phase2);
    insertPhase(&phaseList, phase3);

    // ========== LOOP PRINCIPAL ==========
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        int skipPhaseRequested = IsKeyPressed(KEY_P);

        if (IsKeyPressed(KEY_F11) || (IsKeyPressed(KEY_F) && IsKeyDown(KEY_LEFT_ALT))) {
            if (!IsWindowFullscreen()) {
                int currentMonitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(currentMonitor), GetMonitorHeight(currentMonitor));
            }
            ToggleFullscreen();
        }
        if (IsKeyPressed(KEY_F3))
            debugMode = !debugMode;

        // ===== UPDATE POR ESTADO =====
        switch (gameState) {

        case STATE_INTRO:
            storyTimer += dt;
            if (storyTimer >= 20.0f || IsKeyPressed(KEY_ENTER)) {
                gameState = STATE_MENU;
                if (introTexturesLoaded) {
                    UnloadTexture(txIntroBg);
                    UnloadTexture(txIntroPlayer);
                    introTexturesLoaded = 0;
                }
            }
            break;

        case STATE_MENU:
            updateMenu(&menu);
            if (menu.screen == MENU_MAIN && IsKeyPressed(KEY_ENTER)) {
                if (menu.selectedOption == 0) {
                    // Iniciar jogo — começa na fase 1
                    unloadStage1(&stage1); initStage1(&stage1);
                    int csw = GetScreenWidth();
                    player = createPlayer((Vector2){ csw * 0.18f, GROUND_LEVEL }, 150, 3);
                    snprintf(currentPlayerName, sizeof(currentPlayerName), "Jogador %d", nextPlayerNumber++);
                    matchRecorded = 0;
                    finalReturnTimer = 0.0f;
                    totalGameTime = 0.0f;
                    gameState = STATE_STAGE1;
                } else if (menu.selectedOption == 3) {
                    goto cleanup;
                }
            }
            break;

        case STATE_STAGE1:
            if (skipPhaseRequested) {
                initStage2(&stage2);
                int csw = GetScreenWidth();
                player = createPlayer((Vector2){ csw * 0.18f, GROUND_LEVEL }, 150, 3);
                gameState = STATE_STAGE2;
                break;
            }

            updateStage1(&stage1, &player, dt);
            updatePlayer(&player, dt);
            if (!stage1.stage1Complete) totalGameTime += dt;

            if (player.lives <= 0) {
                if (!matchRecorded) {
                    addRankingEntry(currentPlayerName, totalGameTime,
                                    calculateReachedDistance(gameState, &stage1, &stage2, &stage3, &player));
                    matchRecorded = 1;
                }
                gameOverTimer = 3.0f;
                gameState = STATE_GAMEOVER;
            } else if (stage1.stage1Complete) {
                // Inicia transição 1→2
                initCutscene12();
                gameState = STATE_TRANSITION_12;
            }
            break;

        case STATE_TRANSITION_12:
            if (skipPhaseRequested || updateCutscene12(dt)) {
                unloadCutscene12();
                initStage2(&stage2);
                int csw = GetScreenWidth();
                player = createPlayer((Vector2){ csw * 0.18f, GROUND_LEVEL }, 150, 3);
                gameState = STATE_STAGE2;
            }
            break;

        case STATE_STAGE2:
            if (skipPhaseRequested) {
                initTransition("assets/img/pika_de_brennand.png", 0.8f, 2.4f);
                gameState = STATE_TRANSITION_23;
                break;
            }

            updateStage2(&stage2, &player, dt);
            if (!stage2.stage2Complete) totalGameTime += dt;

            if (player.lives <= 0) {
                if (!matchRecorded) {
                    addRankingEntry(currentPlayerName, totalGameTime,
                                    calculateReachedDistance(gameState, &stage1, &stage2, &stage3, &player));
                    matchRecorded = 1;
                }
                gameOverTimer = 3.0f;
                gameState = STATE_GAMEOVER;
            } else if (stage2.stage2Complete) {
                // Inicia transição 2→3
                initTransition("assets/img/pika_de_brennand.png", 0.8f, 2.4f);
                gameState = STATE_TRANSITION_23;
            }
            break;

        case STATE_TRANSITION_23:
            if (skipPhaseRequested || updateTransition(dt)) {
                // Prepara fase 3
                int csw = GetScreenWidth();
                player = createPlayer((Vector2){ csw * 0.18f, GROUND_LEVEL }, 150, 3);
                initStage3(&stage3, &player);
                if (transition.loaded && transition.image.id > 0) { UnloadTexture(transition.image); }
                transition.loaded = 0;
                gameState = STATE_STAGE3;
            }
            break;

        case STATE_STAGE3:
            if (skipPhaseRequested) {
                stage3.state = STAGE3_FINISHED;
                player.isClimbing = false;
                finalReturnTimer = 0.0f;
                if (!matchRecorded) {
                    addRankingEntry(currentPlayerName, totalGameTime,
                                    calculateReachedDistance(gameState, &stage1, &stage2, &stage3, &player));
                    matchRecorded = 1;
                }
                break;
            }

            if (stage3.state == STAGE3_FINISHED) {
                finalReturnTimer += dt;
                if (finalReturnTimer >= 10.0f) {
                    unloadStage1(&stage1); initStage1(&stage1);
                    int csw = GetScreenWidth();
                    player = createPlayer((Vector2){ csw * 0.18f, GROUND_LEVEL }, 150, 3);
                    totalGameTime = 0.0f;
                    finalReturnTimer = 0.0f;
                    menu = createMenu();
                    gameState = STATE_MENU;
                }
                break;
            }

            updateStage3(&stage3, &player, dt);
            if (stage3.state != STAGE3_FINISHED) totalGameTime += dt;

            if (player.lives <= 0) {
                if (!matchRecorded) {
                    addRankingEntry(currentPlayerName, totalGameTime,
                                    calculateReachedDistance(gameState, &stage1, &stage2, &stage3, &player));
                    matchRecorded = 1;
                }
                gameOverTimer = 3.0f;
                gameState = STATE_GAMEOVER;
            }
            if (stage3.state == STAGE3_FINISHED && !matchRecorded) {
                addRankingEntry(currentPlayerName, totalGameTime,
                                calculateReachedDistance(gameState, &stage1, &stage2, &stage3, &player));
                matchRecorded = 1;
                finalReturnTimer = 0.0f;
            }
            // Vitória final: STAGE3_FINISHED (tratado no draw)
            break;

        case STATE_GAMEOVER:
            gameOverTimer -= dt;
            if (IsKeyPressed(KEY_ENTER) || gameOverTimer <= 0) {
                unloadStage1(&stage1); initStage1(&stage1);
                int csw = GetScreenWidth();
                player = createPlayer((Vector2){ csw * 0.18f, GROUND_LEVEL }, 150, 3);
                totalGameTime = 0.0f;
                finalReturnTimer = 0.0f;
                menu = createMenu();
                gameState = STATE_MENU;
            }
            break;
        }

        switch (gameState) {
        case STATE_STAGE1:
        case STATE_TRANSITION_12:
            playGameMusic(MUSIC_STAGE1);
            break;

        case STATE_STAGE2:
            if (stage2.mode == STAGE2_MODE_SAND) {
                playGameMusic(MUSIC_STAGE2_SAND);
            } else {
                playGameMusic(MUSIC_STAGE2_SEA);
            }
            break;

        case STATE_TRANSITION_23:
            playGameMusic(MUSIC_STAGE2_SEA);
            break;

        case STATE_STAGE3:
            playGameMusic(stage3.state == STAGE3_FINISHED ? MUSIC_WIN : MUSIC_STAGE3);
            break;

        default:
            playGameMusic(MUSIC_NONE);
            break;
        }
        updateGameMusic();

        // ========== DESENHO ==========
        BeginDrawing();
        int sW = GetScreenWidth();
        int sH = GetScreenHeight();

        switch (gameState) {

        case STATE_INTRO:
            ClearBackground((Color){11,16,27,255});
            drawStoryIntroScreen(storyTimer, sW, sH);
            break;

        case STATE_MENU:
            ClearBackground(BLACK);
            drawMenu(menu);
            if (menu.screen == MENU_RANKING) {
                drawRankingEntries(sW, sH);
            }
            break;

        case STATE_STAGE1:
            ClearBackground(SKYBLUE);
            drawStage1(&stage1, &player);
            drawGameHUD(&stage1, &player, totalGameTime, sW, sH);
            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG (F3)", 10, 30, 14, RED);
                DrawFPS(10, sH - 30);
            }
            // Tela de vitória fase 1 (aguardando transição)
            if (stage1.stage1Complete) {
                DrawRectangle(0,0,sW,sH,(Color){0,0,0,160});
                const char *vt = "FASE 1 COMPLETA!";
                int vtw = MeasureText(vt, 60);
                DrawText(vt, (sW-vtw)/2, sH/2-40, 60, GREEN);
            }
            break;

        case STATE_TRANSITION_12:
            drawCutscene12(sW, sH);
            break;

        case STATE_TRANSITION_23:
            drawTransitionScreen(sW, sH);
            break;

        case STATE_STAGE2:
            ClearBackground(SKYBLUE);
            drawStage2(&stage2, &player);
            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG (F3)", 10, 30, 14, RED);
                DrawFPS(10, sH - 30);
            }
            if (stage2.stage2Complete) {
                DrawRectangle(0,0,sW,sH,(Color){0,0,0,160});
                const char *vt = "FASE 2 COMPLETA!";
                int vtw = MeasureText(vt, 60);
                DrawText(vt, (sW-vtw)/2, sH/2-40, 60, GREEN);
            }
            break;

        case STATE_STAGE3:
            ClearBackground(BLACK);
            if (stage3.state == STAGE3_FINISHED) {
                drawFinalVictoryScreen(sW, sH);
            } else {
                drawStage3(&stage3, &player);
                drawSharedLifeScoreTimeHUD(&player, totalGameTime, sW, sH);
            }
            if (debugMode) {
                drawPlayerDebug(player);
                DrawText("DEBUG (F3)", 10, 30, 14, RED);
                DrawFPS(10, sH - 30);
            }
            break;

        case STATE_GAMEOVER:
            ClearBackground((Color){20,10,10,255});
            {
                const char *gt = "GAME OVER";
                int gtw = MeasureText(gt, 60);
                DrawText(gt, (sW-gtw)/2, (int)(sH*0.26f), 60, RED);
                char sc[128];
                sprintf(sc, "Pontos: %.0f | Tempo: %.1f seg", player.score, totalGameTime);
                int scw = MeasureText(sc, 20);
                DrawText(sc, (sW-scw)/2, (int)(sH*0.44f), 20, WHITE);
                const char *rt = "Pressione ENTER para voltar ao menu";
                int rtw = MeasureText(rt, 16);
                DrawText(rt, (sW-rtw)/2, (int)(sH*0.56f), 16, WHITE);
                if (gameOverTimer > 0) {
                    char tt[64];
                    sprintf(tt, "Reiniciando em %.1f s", gameOverTimer);
                    int ttw = MeasureText(tt, 14);
                    DrawText(tt, (sW-ttw)/2, (int)(sH*0.67f), 14, YELLOW);
                }
            }
            break;
        }

        EndDrawing();
    }

cleanup:
    if (introTexturesLoaded) { UnloadTexture(txIntroBg); UnloadTexture(txIntroPlayer); }
    unloadFinalScreenTexture();
    if (transition.loaded && transition.image.id > 0)   UnloadTexture(transition.image);
    unloadCutscene12();
    unloadStage1(&stage1);
    unloadStage2(&stage2);
    unloadStage3(&stage3);
    unloadPlayerResources(&player);
    unloadGameMusic();
    unloadHudHeartTexture();
    unloadMenuResources();
    if (phaseList) freePhaseList(phaseList);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
