#include "stage3.h"
#include "../utils/gameConstants.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WORLD_WIDTH 800.0f
#define WORLD_HEIGHT 450.0f
#define TOWER_VISIBLE_X 395.0f
#define TOWER_VISIBLE_Y 26.0f
#define TOWER_VISIBLE_WIDTH 234.0f
#define TOWER_VISIBLE_HEIGHT 973.0f
#define TOWER_BASE_Y (GLOBAL_GROUND_LEVEL + PLAYER_HEIGHT)
#define FLOOR_VISUAL_TOP (GLOBAL_GROUND_LEVEL - 12.0f)
#define TOWER_START_X 2500.0f
#define PUDDLE_WIDTH 96.0f
#define PUDDLE_HEIGHT 34.0f
#define BOTTLE_WIDTH 52.0f
#define BOTTLE_HEIGHT 30.0f
#define BOTTLE_GAP 6.0f
#define PUDDLE_CLUSTER_WIDTH (PUDDLE_WIDTH + BOTTLE_GAP + BOTTLE_WIDTH)
#define PUDDLE_MIN_GAP 320.0f
#define TOWER_PUDDLE_CLEARANCE 180.0f
#define PLAYER_CENTER_X (WORLD_WIDTH * 0.5f - PLAYER_WIDTH * 0.5f)
#define PLAYER_LEFT_LIMIT 80.0f
#define PLAYER_RIGHT_LIMIT 620.0f
#define CAMERA_ZOOM_FACTOR 0.90f
#define CAMERA_VERTICAL_LOOKAHEAD 150.0f
#define POOP_GROUND_Y (TOWER_BASE_Y - 13.0f)
#define POOP_LANDED_DURATION 0.5f

static void movePuddle(Puddle *puddle, float deltaX);
static void syncTowerHitbox(Stage3 *stage);
static Vector2 ambientToWorld(Player *player, Vector2 localPosition);

static float clampFloat(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static float visibleWorldWidth(void) {
    float scaleX = GetScreenWidth() / WORLD_WIDTH;
    float scaleY = GetScreenHeight() / WORLD_HEIGHT;
    float zoom = ((scaleX < scaleY) ? scaleX : scaleY) * CAMERA_ZOOM_FACTOR;

    return GetScreenWidth() / zoom;
}

static float visibleWorldHeight(void) {
    float scaleX = GetScreenWidth() / WORLD_WIDTH;
    float scaleY = GetScreenHeight() / WORLD_HEIGHT;
    float zoom = ((scaleX < scaleY) ? scaleX : scaleY) * CAMERA_ZOOM_FACTOR;

    return GetScreenHeight() / zoom;
}

static Vector2 cameraTargetForPlayer(Player *player) {
    return (Vector2){
        player->position.x + PLAYER_WIDTH / 2.0f,
        player->position.y + player->height / 2.0f - CAMERA_VERTICAL_LOOKAHEAD
    };
}

static Rectangle cameraWorldRect(Player *player, float padding) {
    Vector2 target = cameraTargetForPlayer(player);
    float width = visibleWorldWidth();
    float height = visibleWorldHeight();

    return (Rectangle){
        target.x - width * 0.5f - padding,
        target.y - height * 0.5f - padding,
        width + padding * 2.0f,
        height + padding * 2.0f
    };
}

static Vector2 ambientToWorld(Player *player, Vector2 localPosition) {
    Rectangle view = cameraWorldRect(player, 0.0f);

    return (Vector2){
        view.x + localPosition.x,
        view.y + localPosition.y
    };
}

static float visibleWorldLeft(void) {
    return (WORLD_WIDTH - visibleWorldWidth()) * 0.5f;
}

static float towerDrawWidth(Stage3 *stage) {
    return stage->towerTexture.id > 0 ? TOWER_VISIBLE_WIDTH : 100.0f;
}

static float towerDrawHeight(Stage3 *stage) {
    return stage->towerTexture.id > 0 ? TOWER_VISIBLE_HEIGHT : 600.0f;
}

static Rectangle towerSourceRect(Stage3 *stage) {
    if (stage->towerTexture.id > 0) {
        return (Rectangle){ TOWER_VISIBLE_X, TOWER_VISIBLE_Y, TOWER_VISIBLE_WIDTH, TOWER_VISIBLE_HEIGHT };
    }

    return (Rectangle){ 0.0f, 0.0f, 100.0f, 600.0f };
}

static float approachMaxScroll(Stage3 *stage) {
    float playerCenter = PLAYER_CENTER_X + PLAYER_WIDTH * 0.5f;
    return TOWER_START_X + towerDrawWidth(stage) * 0.5f - playerCenter;
}

static void applyHorizontalScroll(Stage3 *stage, float scrollDelta) {
    if (scrollDelta == 0.0f) {
        return;
    }

    stage->scrollX += scrollDelta;
    stage->towerPosition.x -= scrollDelta;
    syncTowerHitbox(stage);

    for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
        movePuddle(&stage->puddles[i], -scrollDelta);
    }
}

static void syncTowerHitbox(Stage3 *stage) {
    float towerWidth = towerDrawWidth(stage);
    float towerHeight = towerDrawHeight(stage);

    stage->towerHitbox = (Rectangle){
        stage->towerPosition.x + (towerWidth * 0.2f),
        stage->towerPosition.y,
        towerWidth * 0.6f,
        towerHeight
    };
}

static float towerPuddleClearLeft(Stage3 *stage) {
    return stage->towerPosition.x - TOWER_PUDDLE_CLEARANCE;
}

static bool canPlacePuddle(Stage3 *stage, float x) {
    return x + PUDDLE_CLUSTER_WIDTH < towerPuddleClearLeft(stage);
}

static bool puddleTouchesTowerArea(Stage3 *stage, Puddle *puddle) {
    float clearLeft = towerPuddleClearLeft(stage);
    float clearRight = stage->towerPosition.x + towerDrawWidth(stage) + TOWER_PUDDLE_CLEARANCE * 0.7f;

    return puddle->position.x + PUDDLE_CLUSTER_WIDTH > clearLeft && puddle->position.x < clearRight;
}

static void deactivatePuddle(Puddle *puddle) {
    puddle->position = (Vector2){ -PUDDLE_WIDTH * 2.0f, TOWER_BASE_Y - PUDDLE_HEIGHT + 4.0f };
    puddle->hitbox = (Rectangle){ -PUDDLE_WIDTH * 2.0f, TOWER_BASE_Y - 15.0f, PUDDLE_WIDTH - 38.0f, 14.0f };
    puddle->bottlePosition = (Vector2){ -PUDDLE_WIDTH * 2.0f, TOWER_BASE_Y - BOTTLE_HEIGHT + 4.0f };
    puddle->bottleHitbox = (Rectangle){ -PUDDLE_WIDTH * 2.0f, TOWER_BASE_Y - BOTTLE_HEIGHT + 12.0f, BOTTLE_WIDTH - 22.0f, BOTTLE_HEIGHT - 15.0f };
    puddle->active = false;
    puddle->canLockPlayer = false;
}

static void clearPuddlesFromTowerArea(Stage3 *stage) {
    for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
        if (stage->puddles[i].active && puddleTouchesTowerArea(stage, &stage->puddles[i])) {
            deactivatePuddle(&stage->puddles[i]);
        }
    }
}

static Color lerpColor(Color from, Color to, float amount) {
    amount = clampFloat(amount, 0.0f, 1.0f);

    return (Color){
        (unsigned char)(from.r + (to.r - from.r) * amount),
        (unsigned char)(from.g + (to.g - from.g) * amount),
        (unsigned char)(from.b + (to.b - from.b) * amount),
        (unsigned char)(from.a + (to.a - from.a) * amount)
    };
}

static void stopAmbientSpawning(Stage3 *stage) {
    stage->ambientSpawningEnabled = false;

    // A torre deve ficar sem novos perigos aereos; remove apenas dejetos ativos.
    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
        stage->poops[i].active = false;
        stage->poops[i].landed = false;
        stage->poops[i].groundTimer = 0.0f;
    }
}

static void initPuddle(Puddle *puddle, float x) {
    float bottleX = x + PUDDLE_WIDTH + BOTTLE_GAP;

    puddle->position = (Vector2){ x, TOWER_BASE_Y - PUDDLE_HEIGHT + 4.0f };
    puddle->hitbox = (Rectangle){
        x + 18.0f,
        TOWER_BASE_Y - 15.0f,
        PUDDLE_WIDTH - 38.0f,
        14.0f
    };
    puddle->bottlePosition = (Vector2){ bottleX, TOWER_BASE_Y - BOTTLE_HEIGHT + 5.0f };
    puddle->bottleHitbox = (Rectangle){
        bottleX + 9.0f,
        TOWER_BASE_Y - BOTTLE_HEIGHT + 12.0f,
        BOTTLE_WIDTH - 22.0f,
        BOTTLE_HEIGHT - 15.0f
    };
    puddle->active = true;
    puddle->canLockPlayer = true;
}

static float randomPuddleSpacing(void) {
    return PUDDLE_CLUSTER_WIDTH + PUDDLE_MIN_GAP + (float)(rand() % 220);
}

static bool resetPuddleAhead(Stage3 *stage, Puddle *puddle, float minX) {
    float baseX = minX;
    for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
        if (!stage->puddles[i].active || &stage->puddles[i] == puddle) {
            continue;
        }

        float nextFreeX = stage->puddles[i].position.x + PUDDLE_CLUSTER_WIDTH + PUDDLE_MIN_GAP;
        if (nextFreeX > baseX) {
            baseX = nextFreeX;
        }
    }

    float x = baseX + (float)(rand() % 220);

    if (!canPlacePuddle(stage, x)) {
        deactivatePuddle(puddle);
        return false;
    }

    initPuddle(puddle, x);
    return true;
}

static void movePuddle(Puddle *puddle, float deltaX) {
    puddle->position.x += deltaX;
    puddle->hitbox.x += deltaX;
    puddle->bottlePosition.x += deltaX;
    puddle->bottleHitbox.x += deltaX;
}

static void drawPuddle(Puddle puddle) {
    if (!puddle.active) {
        return;
    }

    float x = puddle.position.x;
    float y = puddle.position.y;

    DrawRectangle((int)(x + 10), (int)(y + 14), 78, 14, (Color){ 41, 70, 89, 205 });
    DrawRectangle((int)(x + 18), (int)(y + 8), 60, 10, (Color){ 58, 92, 119, 220 });
    DrawRectangle((int)(x + 28), (int)(y + 2), 42, 8, (Color){ 73, 108, 135, 210 });
    DrawRectangle((int)(x + 4), (int)(y + 18), 16, 8, (Color){ 54, 86, 109, 205 });
    DrawRectangle((int)(x + 76), (int)(y + 17), 16, 8, (Color){ 48, 78, 101, 205 });
    DrawRectangle((int)(x + 22), (int)(y + 20), 52, 8, (Color){ 45, 77, 102, 210 });
    DrawRectangle((int)(x + 24), (int)(y + 10), 18, 4, (Color){ 122, 158, 181, 170 });
    DrawRectangle((int)(x + 44), (int)(y + 15), 10, 3, (Color){ 104, 139, 163, 150 });
    DrawRectangle((int)(x + 61), (int)(y + 8), 12, 3, (Color){ 100, 138, 164, 140 });
    DrawRectangle((int)(x + 1), (int)(y + 28), 15, 5, (Color){ 42, 71, 91, 190 });
    DrawRectangle((int)(x + 84), (int)(y + 27), 10, 4, (Color){ 50, 81, 103, 175 });
}

static void drawBottle(Puddle puddle) {
    if (!puddle.active) {
        return;
    }

    float x = puddle.bottlePosition.x;
    float y = puddle.bottlePosition.y;
    Color glassDark = (Color){ 31, 46, 40, 245 };
    Color glassMid = (Color){ 74, 101, 85, 235 };
    Color glassEdge = (Color){ 132, 158, 137, 225 };
    Color glassLight = (Color){ 209, 229, 203, 225 };
    Color dangerRed = (Color){ 164, 24, 18, 255 };
    Color dangerOrange = (Color){ 244, 141, 27, 255 };
    Color warning = (Color){ 255, 214, 54, 255 };
    Color shard = (Color){ 185, 210, 190, 230 };

    DrawTriangle((Vector2){ x - 8.0f, y + 5.0f },
                 (Vector2){ x + 5.0f, y + 11.0f },
                 (Vector2){ x + 3.0f, y + 25.0f },
                 glassMid);
    DrawTriangle((Vector2){ x - 6.0f, y + 4.0f },
                 (Vector2){ x + 2.0f, y + 9.0f },
                 (Vector2){ x - 2.0f, y + 17.0f },
                 glassLight);
    DrawRectangle((int)(x + 3), (int)(y + 10), 12, 16, glassDark);
    DrawRectangle((int)(x + 14), (int)(y + 11), 28, 13, glassDark);
    DrawRectangle((int)(x + 18), (int)(y + 8), 22, 18, glassMid);
    DrawRectangle((int)(x + 37), (int)(y + 13), 11, 8, glassDark);
    DrawRectangle((int)(x + 47), (int)(y + 14), 8, 6, glassMid);
    DrawRectangle((int)(x + 52), (int)(y + 13), 5, 8, glassLight);
    DrawRectangle((int)(x + 20), (int)(y + 10), 18, 12, dangerRed);
    DrawRectangleLines((int)(x + 20), (int)(y + 10), 18, 12, dangerOrange);
    DrawTriangle((Vector2){ x + 29.0f, y + 11.0f },
                 (Vector2){ x + 22.0f, y + 21.0f },
                 (Vector2){ x + 36.0f, y + 21.0f },
                 warning);
    DrawRectangle((int)(x + 28), (int)(y + 14), 2, 4, dangerRed);
    DrawRectangle((int)(x + 28), (int)(y + 19), 2, 2, dangerRed);
    DrawRectangle((int)(x + 17), (int)(y + 7), 19, 3, glassLight);
    DrawRectangle((int)(x + 39), (int)(y + 12), 8, 3, glassLight);
    DrawTriangle((Vector2){ x + 8.0f, y + 29.0f },
                 (Vector2){ x + 20.0f, y + 24.0f },
                 (Vector2){ x + 17.0f, y + 32.0f },
                 shard);
    DrawTriangle((Vector2){ x + 25.0f, y + 28.0f },
                 (Vector2){ x + 35.0f, y + 24.0f },
                 (Vector2){ x + 38.0f, y + 31.0f },
                 glassEdge);
    DrawRectangle((int)(x + 44), (int)(y + 25), 8, 4, glassLight);
    DrawRectangle((int)(x + 1), (int)(y + 27), 5, 4, glassEdge);
}

static void drawFallingPoop(Vector2 position) {
    float x = position.x;
    float y = position.y;
    Color paperShadow = (Color){ 152, 151, 143, 235 };
    Color paperMid = (Color){ 224, 225, 218, 245 };
    Color paperLight = (Color){ 246, 247, 239, 250 };
    Color dirtDark = (Color){ 83, 72, 50, 245 };
    Color dirtMid = (Color){ 112, 96, 63, 245 };
    Color trail = (Color){ 220, 220, 216, 210 };

    DrawRectangle((int)(x - 4.0f), (int)(y - 54.0f), 4, 18, trail);
    DrawRectangle((int)(x + 10.0f), (int)(y - 38.0f), 4, 26, trail);
    DrawRectangle((int)(x - 21.0f), (int)(y - 28.0f), 4, 16, (Color){ 190, 190, 186, 185 });
    DrawRectangle((int)(x - 11.0f), (int)(y - 15.0f), 5, 20, paperLight);
    DrawRectangle((int)(x - 21.0f), (int)(y + 6.0f), 18, 9, paperMid);
    DrawRectangle((int)(x - 14.0f), (int)(y - 2.0f), 24, 20, paperLight);
    DrawRectangle((int)(x + 7.0f), (int)(y + 6.0f), 14, 11, paperShadow);
    DrawRectangle((int)(x - 7.0f), (int)(y + 2.0f), 13, 13, dirtDark);
    DrawRectangle((int)(x + 4.0f), (int)(y + 5.0f), 13, 15, dirtMid);
    DrawRectangle((int)(x - 2.0f), (int)(y - 5.0f), 6, 25, paperLight);
    DrawRectangle((int)(x - 18.0f), (int)(y + 18.0f), 13, 5, paperShadow);
    DrawRectangle((int)(x + 8.0f), (int)(y + 18.0f), 12, 5, paperShadow);
    DrawRectangle((int)(x - 25.0f), (int)(y + 27.0f), 4, 8, paperMid);
    DrawRectangle((int)(x + 1.0f), (int)(y + 32.0f), 4, 8, paperShadow);
}

static void drawLandedPoop(Vector2 position) {
    float x = position.x;
    float y = position.y;
    Color paperShadow = (Color){ 147, 146, 137, 235 };
    Color paperMid = (Color){ 220, 221, 213, 245 };
    Color paperLight = (Color){ 246, 247, 238, 250 };
    Color dirtDark = (Color){ 80, 70, 48, 245 };
    Color dirtMid = (Color){ 112, 95, 60, 245 };

    DrawRectangle((int)(x - 27.0f), (int)(y - 5.0f), 15, 8, paperMid);
    DrawRectangle((int)(x - 17.0f), (int)(y - 10.0f), 32, 13, paperLight);
    DrawRectangle((int)(x + 9.0f), (int)(y - 7.0f), 24, 11, paperMid);
    DrawRectangle((int)(x - 22.0f), (int)(y + 2.0f), 46, 10, paperShadow);
    DrawRectangle((int)(x - 13.0f), (int)(y - 5.0f), 18, 7, dirtDark);
    DrawRectangle((int)(x + 4.0f), (int)(y - 4.0f), 18, 9, dirtMid);
    DrawRectangle((int)(x - 6.0f), (int)(y + 3.0f), 16, 7, dirtDark);
    DrawRectangle((int)(x - 34.0f), (int)(y + 4.0f), 7, 4, paperMid);
    DrawRectangle((int)(x + 35.0f), (int)(y + 2.0f), 6, 4, paperShadow);
    DrawRectangle((int)(x - 29.0f), (int)(y + 12.0f), 4, 3, paperShadow);
}

static void drawPixelCloud(float x, float y, float scale, Color base, Color shadow, Color light) {
    float s = scale;

    DrawRectangle((int)(x + 28 * s), (int)(y + 18 * s), (int)(116 * s), (int)(24 * s), shadow);
    DrawRectangle((int)(x + 56 * s), (int)(y + 2 * s), (int)(86 * s), (int)(32 * s), base);
    DrawRectangle((int)(x + 4 * s), (int)(y + 30 * s), (int)(132 * s), (int)(28 * s), base);
    DrawRectangle((int)(x + 88 * s), (int)(y + 42 * s), (int)(108 * s), (int)(22 * s), shadow);
    DrawRectangle((int)(x + 38 * s), (int)(y + 12 * s), (int)(54 * s), (int)(16 * s), light);
    DrawRectangle((int)(x + 132 * s), (int)(y + 24 * s), (int)(44 * s), (int)(14 * s), light);
}

static void drawRain(float left, float top, float width, float height, float intensity) {
    if (intensity <= 0.02f) {
        return;
    }

    float time = (float)GetTime();
    unsigned char alpha = (unsigned char)(88.0f * intensity);
    Color rainColor = (Color){ 126, 164, 181, alpha };

    for (int i = 0; i < 54; i++) {
        float x = left + fmodf((i * 79.0f) + time * 18.0f, width + 160.0f) - 80.0f;
        float y = top + fmodf((i * 43.0f) + time * 280.0f, height + 120.0f) - 60.0f;
        DrawLineEx((Vector2){ x, y }, (Vector2){ x - 8.0f, y + 20.0f }, 1.0f, rainColor);
    }
}

static void drawStage3Background(Stage3 *stage, Player *player) {
    Rectangle view = cameraWorldRect(player, 180.0f);
    float climbProgress = clampFloat((TOWER_BASE_Y - player->position.y) / (towerDrawHeight(stage) * 0.82f), 0.0f, 1.0f);
    float sunrise = clampFloat(fmaxf(-stage->scrollY / 650.0f, climbProgress), 0.0f, 1.0f);
    float rainIntensity = 1.0f - sunrise;
    float left = view.x;
    float top = view.y;
    float width = view.width;
    float height = view.height;
    float time = (float)GetTime();
    Color skyTop = lerpColor((Color){ 13, 38, 72, 255 }, (Color){ 55, 38, 93, 255 }, sunrise);
    Color skyBottom = lerpColor((Color){ 75, 98, 120, 255 }, (Color){ 221, 116, 70, 255 }, sunrise);

    DrawRectangleGradientV((int)left - 2, (int)top - 2, (int)width + 4, (int)height + 4, skyTop, skyBottom);

    float hazeRoute = fmodf(time * 7.0f, WORLD_WIDTH);
    for (int i = -2; i < 3; i++) {
        float offset = left + i * WORLD_WIDTH - hazeRoute;
        DrawRectangle((int)(offset + 0), (int)(top + height * 0.24f), 160, 28, (Color){ 27, 59, 87, 190 });
        DrawRectangle((int)(offset + 112), (int)(top + height * 0.29f), 190, 34, (Color){ 35, 73, 99, 185 });
        DrawRectangle((int)(offset + 334), (int)(top + height * 0.32f), 148, 22, (Color){ 42, 82, 107, 170 });
        DrawRectangle((int)(offset + 522), (int)(top + height * 0.26f), 214, 30, (Color){ 28, 62, 91, 180 });
    }

    float sunX = left + width * 0.52f;
    float sunY = top + height * (0.68f - sunrise * 0.36f);
    DrawCircleGradient((int)sunX, (int)sunY, 58,
                       (Color){ 226, 214, 128, 150 },
                       (Color){ 226, 214, 128, 0 });
    DrawCircle((int)sunX, (int)sunY, 22, (Color){ 225, 214, 146, 235 });

    float cloudRoute = fmodf(time * 12.0f, WORLD_WIDTH);
    drawPixelCloud(left + 20.0f - cloudRoute * 0.4f, top + height * 0.12f, 1.8f,
                   (Color){ 34, 73, 101, 190 },
                   (Color){ 20, 47, 78, 205 },
                   (Color){ 61, 105, 130, 160 });
    drawPixelCloud(left + 470.0f - cloudRoute * 0.55f, top + height * 0.2f, 1.55f,
                   (Color){ 56, 91, 121, 190 },
                   (Color){ 29, 57, 91, 210 },
                   (Color){ 105, 118, 145, 150 });
    drawPixelCloud(left + 130.0f - cloudRoute * 0.75f, top + height * 0.56f, 1.2f,
                   (Color){ 82, 122, 140, 160 },
                   (Color){ 42, 76, 101, 180 },
                   (Color){ 135, 130, 148, 120 });

    drawRain(left, top, width, height, rainIntensity);
}

static void drawStage3Floor(Stage3 *stage, float floorY) {
    float left = visibleWorldLeft() - WORLD_WIDTH;
    float width = visibleWorldWidth() + WORLD_WIDTH * 2.0f;
    float height = visibleWorldHeight();
    float drawY = floorY - 72.0f;
    float tileSize = 32.0f;
    float floorScroll = stage->scrollX * 0.95f;
    int baseCol = (int)floorf(floorScroll / tileSize);
    float offset = floorScroll - (baseCol * tileSize);
    int visibleCols = (int)(width / tileSize) + 18;

    DrawRectangle((int)left - 120, (int)drawY, (int)width + 240, (int)(height + WORLD_HEIGHT - drawY + 160),
                  (Color){ 176, 96, 48, 255 });

    for (int row = 0; row < 6; row++) {
        for (int col = -8; col < visibleCols; col++) {
            int worldCol = baseCol + col;
            float x = left + col * tileSize - offset + ((row % 2) ? tileSize * 0.5f : 0.0f);
            float y = drawY + 8.0f + row * 24.0f;
            int pattern = (row + worldCol) % 3;
            if (pattern < 0) pattern += 3;
            Color tile = (pattern == 0) ? (Color){ 177, 92, 49, 255 } :
                         (pattern == 1) ? (Color){ 214, 126, 62, 255 } :
                                          (Color){ 151, 116, 54, 255 };
            DrawRectangle((int)x, (int)y, 31, 23, tile);
            DrawRectangleLines((int)x, (int)y, 31, 23, (Color){ 92, 55, 42, 190 });

            if ((row + worldCol) % 4 == 0) {
                DrawRectangle((int)x + 9, (int)y + 7, 13, 7, (Color){ 202, 143, 72, 190 });
            }
        }
    }
}

static void drawTowerAura(Stage3 *stage) {
    float towerWidth = towerDrawWidth(stage);
    float towerHeight = towerDrawHeight(stage);
    float left = visibleWorldLeft();
    float right = left + visibleWorldWidth();
    float towerLeft = stage->towerPosition.x;
    float towerRight = towerLeft + towerWidth;

    if (towerLeft > right + 180.0f || towerRight < left - 180.0f) {
        return;
    }

    float time = (float)GetTime();
    float pulse = 0.5f + 0.5f * sinf(time * 2.4f);
    float centerX = towerLeft + towerWidth * 0.5f;
    float baseY = stage->towerPosition.y + towerHeight;
    float midY = stage->towerPosition.y + towerHeight * 0.56f;
    Color auraCore = (Color){ 108, 228, 255, (unsigned char)(34 + pulse * 24.0f) };
    Color auraOuter = (Color){ 108, 228, 255, 0 };
    Color beamTop = (Color){ 177, 247, 255, (unsigned char)(18 + pulse * 12.0f) };
    Color beamBottom = (Color){ 177, 247, 255, 0 };

    DrawCircleGradient((int)centerX, (int)midY, 128.0f + pulse * 18.0f, auraCore, auraOuter);
    DrawRectangleGradientV((int)(centerX - 34.0f), (int)(stage->towerPosition.y + 72.0f), 68,
                           (int)(towerHeight - 80.0f), beamTop, beamBottom);
    DrawEllipse((int)centerX, (int)(baseY - 6.0f), 128.0f, 18.0f,
                (Color){ 115, 233, 255, (unsigned char)(30 + pulse * 20.0f) });

    for (int i = 0; i < 7; i++) {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        float sparkleX = centerX + side * (42.0f + (float)((i * 17) % 42));
        float sparkleY = stage->towerPosition.y + 130.0f + fmodf(time * (22.0f + i * 2.0f) + i * 109.0f,
                                                                 towerHeight - 220.0f);
        Color sparkle = (Color){ 205, 252, 255, (unsigned char)(115 + pulse * 75.0f) };

        DrawRectangle((int)sparkleX, (int)sparkleY, 4, 12, sparkle);
        DrawRectangle((int)(sparkleX - 4.0f), (int)(sparkleY + 4.0f), 12, 4, sparkle);
    }
}

void initStage3(Stage3 *stage, Player *player) {
    stage->state = STAGE3_APPROACH;
    stage->scrollX = 0.0f;
    stage->scrollY = 0.0f;
    stage->puddleLockTimer = 0.0f;
    stage->puddlePushVelocity = 0.0f;
    stage->ambientSpawningEnabled = true;
    
    stage->towerTexture = LoadTexture("assets/img/torre_cristal.png");
    stage->cloudTexture = LoadTexture("assets/img/nuvem.png");
    stage->birdTexture = LoadTexture("assets/img/passaro.png");
    
    // Posiciona a torre mais para a direita para expandir o mapa (1200px)
    // Desenha apenas a area visivel do PNG para a base da torre ficar exatamente no chao.
    stage->towerPosition = (Vector2){ TOWER_START_X, TOWER_BASE_Y - towerDrawHeight(stage) };
    syncTowerHitbox(stage);
    
    // Staggered cloud positions, extremely high up, with randomized speed and scale
    stage->clouds[0].position = (Vector2){ rand() % 350, 10 + (rand() % 30) };
    stage->clouds[0].speed = 10.0f + (rand() % 25);
    stage->clouds[0].scale = 0.15f + (rand() % 10) / 100.0f; // Scale between 0.15 and 0.25
    
    stage->clouds[1].position = (Vector2){ 450 + (rand() % 300), 10 + (rand() % 30) };
    stage->clouds[1].speed = 10.0f + (rand() % 25);
    stage->clouds[1].scale = 0.15f + (rand() % 10) / 100.0f;
    
    // Inicializa 3 pássaros com distâncias e timers de cocô individuais
    stage->birds[0].position = (Vector2){ rand() % 250, 30 + (rand() % 30) };
    stage->birds[0].speed = 50.0f + (rand() % 30);
    stage->birds[0].poopTimer = 0.0f;
    stage->birds[0].poopInterval = 1.0f + (rand() % 200) / 100.0f;
    
    stage->birds[1].position = (Vector2){ 350 + (rand() % 300), 20 + (rand() % 30) };
    stage->birds[1].speed = 45.0f + (rand() % 40);
    stage->birds[1].poopTimer = 0.0f;
    stage->birds[1].poopInterval = 1.0f + (rand() % 200) / 100.0f;
    
    stage->birds[2].position = (Vector2){ 800 + (rand() % 400), 35 + (rand() % 25) };
    stage->birds[2].speed = 60.0f + (rand() % 30);
    stage->birds[2].poopTimer = 0.0f;
    stage->birds[2].poopInterval = 1.0f + (rand() % 200) / 100.0f;
    
    player->position = (Vector2){ PLAYER_CENTER_X, GLOBAL_GROUND_LEVEL };
    player->isClimbing = false;
    player->velocity = (Vector2){0, 0};
    player->speed = 0.0f; // Para o auto-run
    player->movementControlledExternally = true; // Stage 3 controla o movimento
    
    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
        stage->poops[i].active = false;
        stage->poops[i].landed = false;
        stage->poops[i].groundTimer = 0.0f;
        stage->poops[i].speedY = 0.0f;
    }

    float puddleX = 420.0f + (float)(rand() % 180);
    for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
        if (canPlacePuddle(stage, puddleX)) {
            initPuddle(&stage->puddles[i], puddleX);
        } else {
            deactivatePuddle(&stage->puddles[i]);
        }
        puddleX += randomPuddleSpacing();
    }
}

void updateStage3(Stage3 *stage, Player *player, float deltaTime) {
    if (player->lives <= 0) {
        return;
    }

    for (int i = 0; i < STAGE3_MAX_CLOUDS; i++) {
        stage->clouds[i].position.x -= stage->clouds[i].speed * deltaTime;
        if (stage->ambientSpawningEnabled && stage->clouds[i].position.x < -200) {
            stage->clouds[i].position.x = WORLD_WIDTH + (i * 400) + (rand() % 200); // Staggered reset
            stage->clouds[i].position.y = 10 + (rand() % 30);
            stage->clouds[i].speed = 10.0f + (rand() % 25);
            stage->clouds[i].scale = 0.15f + (rand() % 10) / 100.0f;
        }
    }
    
    for (int i = 0; i < STAGE3_MAX_BIRDS; i++) {
        stage->birds[i].position.x -= stage->birds[i].speed * deltaTime;
        if (stage->ambientSpawningEnabled && stage->birds[i].position.x < -150) {
            // Reinicia mantendo-os distantes uns dos outros
            stage->birds[i].position.x = WORLD_WIDTH + (i * 300) + (rand() % 200);
            stage->birds[i].position.y = 20 + (rand() % 35); 
            stage->birds[i].poopTimer = 0.0f;
            stage->birds[i].poopInterval = 1.0f + (rand() % 250) / 100.0f;
        }
    }

    if (stage->state == STAGE3_APPROACH) {
        float moveSpeed = player->grounded ? 230.0f : 315.0f;
        float moveDelta = moveSpeed * deltaTime;
        float scrollDelta = 0.0f;
        float maxScrollX = approachMaxScroll(stage);
        bool movementLockedByPuddle = stage->puddleLockTimer > 0.0f;

        if (stage->puddleLockTimer > 0.0f) {
            stage->puddleLockTimer -= deltaTime;
            if (stage->puddleLockTimer <= 0.0f) {
                stage->puddleLockTimer = 0.0f;
                stage->puddlePushVelocity = 0.0f;
            }
        }

        if (stage->puddleLockTimer > 0.0f) {
            player->grounded = false;
            player->velocity.y = 0.0f;
        }
        
        if (!movementLockedByPuddle && (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))) {
            if (player->position.x < PLAYER_CENTER_X) {
                player->position.x += moveDelta;
                if (player->position.x > PLAYER_CENTER_X) {
                    player->position.x = PLAYER_CENTER_X;
                }
            } else if (stage->scrollX < maxScrollX) {
                scrollDelta = moveDelta;
                if (stage->scrollX + scrollDelta > maxScrollX) {
                    scrollDelta = maxScrollX - stage->scrollX;
                }
            } else if (player->position.x < PLAYER_RIGHT_LIMIT) {
                player->position.x += moveDelta;
            }
        }
        if (!movementLockedByPuddle && (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))) {
            if (player->position.x > PLAYER_CENTER_X) {
                player->position.x -= moveDelta;
                if (player->position.x < PLAYER_CENTER_X) {
                    player->position.x = PLAYER_CENTER_X;
                }
            } else if (player->position.x > PLAYER_LEFT_LIMIT) {
                player->position.x -= moveDelta;
            } else if (stage->scrollX > 0.0f) {
                scrollDelta = -moveDelta;
                if (stage->scrollX + scrollDelta < 0.0f) {
                    scrollDelta = -stage->scrollX;
                }
            }
        }

        if (scrollDelta != 0.0f) {
            applyHorizontalScroll(stage, scrollDelta);
        }

        float nextPuddleX = visibleWorldLeft() + visibleWorldWidth() + 120.0f;
        for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
            bool shouldResetPuddle = !stage->puddles[i].active ||
                                     stage->puddles[i].position.x < visibleWorldLeft() - PUDDLE_WIDTH;

            if (stage->puddles[i].active && puddleTouchesTowerArea(stage, &stage->puddles[i])) {
                deactivatePuddle(&stage->puddles[i]);
                shouldResetPuddle = true;
            }

            if (shouldResetPuddle && stage->scrollX < maxScrollX - 120.0f) {
                resetPuddleAhead(stage, &stage->puddles[i], nextPuddleX);
            }

            if (stage->puddles[i].active) {
                nextPuddleX = stage->puddles[i].position.x + randomPuddleSpacing();
            }
        }
        
        // Barreira física após a torre: permite passar pela frente, andar no espaço extra, mas impede ir além do limite do mapa expandido
        float towerWidth = towerDrawWidth(stage);
        if (stage->puddleLockTimer > 0.0f && stage->puddlePushVelocity > 0.0f) {
            float pushScrollDelta = stage->puddlePushVelocity * deltaTime;
            if (stage->scrollX < maxScrollX) {
                if (stage->scrollX + pushScrollDelta > maxScrollX) {
                    pushScrollDelta = maxScrollX - stage->scrollX;
                }

                applyHorizontalScroll(stage, pushScrollDelta);
            } else if (player->position.x < PLAYER_RIGHT_LIMIT) {
                player->position.x += pushScrollDelta;
            }

            stage->puddlePushVelocity *= 0.96f;
        }

        float barrierX = stage->towerPosition.x + towerWidth + 150.0f;
        if (player->position.x + PLAYER_WIDTH > barrierX) {
            player->position.x = barrierX - PLAYER_WIDTH;
        }
        if (player->position.x < PLAYER_LEFT_LIMIT) {
            player->position.x = PLAYER_LEFT_LIMIT;
        }
        if (player->position.x > PLAYER_RIGHT_LIMIT) {
            player->position.x = PLAYER_RIGHT_LIMIT;
        }

        clearPuddlesFromTowerArea(stage);
        
        // Verifica se o player está à frente da torre (sobreposto no eixo X)
        bool isOverlappingTowerX = (player->position.x + PLAYER_WIDTH >= stage->towerHitbox.x) && 
                                   (player->position.x <= stage->towerHitbox.x + stage->towerHitbox.width);
        bool isNearTower = (player->position.x + PLAYER_WIDTH >= stage->towerHitbox.x - 80.0f) &&
                           (player->position.x <= stage->towerHitbox.x + stage->towerHitbox.width + 80.0f);

        if (isNearTower && stage->ambientSpawningEnabled) {
            stopAmbientSpawning(stage);
        } else if (!isNearTower && !stage->ambientSpawningEnabled) {
            stage->ambientSpawningEnabled = true;
        }
        
        // Se estiver sobreposto e pressionar W ou SETA PARA CIMA, inicia a escalada
        if (isOverlappingTowerX) {
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
                stage->state = STAGE3_CLIMBING;
                player->isClimbing = true;
                player->velocity.y = 0;
                player->position.x = stage->towerHitbox.x + stage->towerHitbox.width / 2.0f - PLAYER_WIDTH / 2.0f;
            }
        }

        Rectangle playerHitbox = {
            player->position.x,
            player->position.y,
            PLAYER_WIDTH,
            player->height
        };
        Rectangle footHitbox = {
            player->position.x + 4.0f,
            player->position.y + player->height - 8.0f,
            PLAYER_WIDTH - 8.0f,
            8.0f
        };

        for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
            if (!stage->puddles[i].active) {
                continue;
            }

            bool playerOnPuddle = CheckCollisionRecs(footHitbox, stage->puddles[i].hitbox);
            if (CheckCollisionRecs(playerHitbox, stage->puddles[i].bottleHitbox)) {
                player->lives = 0;
                break;
            }

            if (playerOnPuddle && stage->puddles[i].canLockPlayer) {
                stage->puddleLockTimer = 0.78f;
                stage->puddlePushVelocity = 520.0f;
                stage->puddles[i].canLockPlayer = false;
                break;
            } else if (!playerOnPuddle) {
                stage->puddles[i].canLockPlayer = true;
            }
        }
        
    } else if (stage->state == STAGE3_CLIMBING) {
        stage->ambientSpawningEnabled = true;
        stage->scrollY = -clampFloat(TOWER_BASE_Y - player->position.y, 0.0f, towerDrawHeight(stage));

        clearPuddlesFromTowerArea(stage);
        
        // Limita o jogador de ir abaixo da tela ao escalar
        if (player->position.y + player->height > WORLD_HEIGHT) {
            player->position.y = WORLD_HEIGHT - player->height;
        }
        
        if (player->position.x < stage->towerHitbox.x) player->position.x = stage->towerHitbox.x;
        if (player->position.x + PLAYER_WIDTH > stage->towerHitbox.x + stage->towerHitbox.width) player->position.x = stage->towerHitbox.x + stage->towerHitbox.width - PLAYER_WIDTH;
        
        if (player->position.y <= stage->towerPosition.y + 80.0f) { 
            stage->state = STAGE3_FINISHED;
            player->isClimbing = false; // finalizou subida
        }
    }
    
    // ===== GERENCIAMENTO DE COCO DOS PASSAROS =====
    if (stage->ambientSpawningEnabled) {
        for (int b = 0; b < STAGE3_MAX_BIRDS; b++) {
            stage->birds[b].poopTimer += deltaTime;
            if (stage->birds[b].poopTimer >= stage->birds[b].poopInterval) {
                stage->birds[b].poopTimer = 0.0f;
                // Randomiza o próximo intervalo individual do pássaro
                stage->birds[b].poopInterval = 1.0f + (rand() % 350) / 100.0f;
                
                // Spawna apenas se o pássaro estiver na tela
                if (stage->birds[b].position.x > 0.0f && stage->birds[b].position.x < WORLD_WIDTH) {
                    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
                        if (!stage->poops[i].active) {
                            stage->poops[i].position = (Vector2){ stage->birds[b].position.x + 8.0f, stage->birds[b].position.y + 10.0f };
                            stage->poops[i].active = true;
                            stage->poops[i].landed = false;
                            stage->poops[i].groundTimer = 0.0f;
                            stage->poops[i].speedY = 150.0f + (rand() % 150); // Velocidades de queda aleatórias (150 a 300)
                            break;
                        }
                    }
                }
            }
        }
    }
    
    // Atualiza poops
    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
        if (stage->poops[i].active) {
            if (!stage->poops[i].landed) {
                stage->poops[i].position.y += stage->poops[i].speedY * deltaTime;

                if (stage->poops[i].position.y >= POOP_GROUND_Y) {
                    stage->poops[i].position.y = POOP_GROUND_Y;
                    stage->poops[i].landed = true;
                    stage->poops[i].speedY = 0.0f;
                    stage->poops[i].groundTimer = POOP_LANDED_DURATION;
                }
            } else {
                stage->poops[i].groundTimer -= deltaTime;
                if (stage->poops[i].groundTimer <= 0.0f) {
                    stage->poops[i].active = false;
                    continue;
                }
            }
            
            Rectangle poopHitbox = stage->poops[i].landed
                ? (Rectangle){ stage->poops[i].position.x - 28.0f, stage->poops[i].position.y - 9.0f, 56.0f, 18.0f }
                : (Rectangle){ stage->poops[i].position.x - 16.0f, stage->poops[i].position.y - 8.0f, 32.0f, 36.0f };
            if (CheckCollisionRecs(player->hitbox, poopHitbox)) {
                player->lives = 0; // Se atingido pelas fezes, perde!
                stage->poops[i].active = false;
            }
        }
    }
}

void drawStage3(Stage3 *stage, Player *player) {
    drawStage3Background(stage, player);
    
    // Chão: Recua para baixo conforme a torre desce (simulando a subida)
    float towerHeight = towerDrawHeight(stage);
    float floorY = stage->towerPosition.y + towerHeight;
    
    drawStage3Floor(stage, floorY);

    for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
        drawPuddle(stage->puddles[i]);
        drawBottle(stage->puddles[i]);
    }
    
    for (int i = 0; i < STAGE3_MAX_CLOUDS; i++) {
        Vector2 cloudPosition = ambientToWorld(player, stage->clouds[i].position);
        if (stage->cloudTexture.id > 0) {
            DrawTextureEx(stage->cloudTexture,
                          cloudPosition,
                          0.0f,
                          stage->clouds[i].scale,
                          WHITE);
        } else {
            drawPixelCloud(cloudPosition.x,
                           cloudPosition.y,
                           0.42f + stage->clouds[i].scale,
                           (Color){ 68, 104, 129, 205 },
                           (Color){ 33, 62, 95, 220 },
                           (Color){ 130, 128, 149, 160 });
        }
    }
    
    drawTowerAura(stage);

    if (stage->towerTexture.id > 0) {
        Rectangle source = towerSourceRect(stage);
        Rectangle dest = {
            stage->towerPosition.x,
            stage->towerPosition.y,
            towerDrawWidth(stage),
            towerDrawHeight(stage)
        };
        DrawTexturePro(stage->towerTexture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(stage->towerHitbox, GRAY);
    }
    for (int i = 0; i < STAGE3_MAX_BIRDS; i++) {
        if (stage->birds[i].position.x < -170.0f || stage->birds[i].position.x > WORLD_WIDTH + 500.0f) {
            continue;
        }

        if (stage->birdTexture.id > 0) {
            DrawTextureEx(stage->birdTexture, stage->birds[i].position, 0.0f, 0.1f, WHITE); // scale super menor
        } else {
            DrawTriangle(
                (Vector2){stage->birds[i].position.x, stage->birds[i].position.y},
                (Vector2){stage->birds[i].position.x + 10, stage->birds[i].position.y - 5},
                (Vector2){stage->birds[i].position.x + 10, stage->birds[i].position.y + 5},
                BLACK
            );
        }
    }
    
    // Desenha os dejetos dos passaros com sprite diferente ao tocar o chao.
    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
        if (stage->poops[i].active) {
            if (stage->poops[i].landed) {
                drawLandedPoop(stage->poops[i].position);
            } else {
                drawFallingPoop(stage->poops[i].position);
            }
        }
    }
}

void unloadStage3(Stage3 *stage) {
    if (stage->towerTexture.id > 0) UnloadTexture(stage->towerTexture);
    if (stage->cloudTexture.id > 0) UnloadTexture(stage->cloudTexture);
    if (stage->birdTexture.id > 0) UnloadTexture(stage->birdTexture);
}
