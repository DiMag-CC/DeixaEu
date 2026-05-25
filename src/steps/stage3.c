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
#define TOWER_START_X 5000.0f
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
#define CAMERA_ZOOM_FACTOR 0.65f
#define CAMERA_VERTICAL_LOOKAHEAD 150.0f
#define POOP_GROUND_Y GLOBAL_GROUND_LEVEL
#define POOP_LANDED_DURATION 0.5f
#define STAGE3_GRAVITY 760.0f
#define STAGE3_JUMP_FORCE 470.0f
#define FINAL_CLIMB_BACKGROUND_COUNT 5
#define TOWER_STAGE3_VERTICAL_OFFSET 55.0f
#define CLIMB_STEP_UP 38.0f
#define CLIMB_STEP_SIDE 72.0f
#define CLIMB_AUTO_SPEED 210.0f
#define CLIMB_CHALLENGE_LIMIT 2.0f
#define CLIMB_MAX_MISSES 3
#define FINAL_CLIMB_PLAYER_WIDTH 168.0f
#define FINAL_CLIMB_PLAYER_HEIGHT 218.0f
#define STAGE3_BIRD_SCALE 0.13f
#define CLIMB_BIRD_SCALE 0.24f
#define STAGE3_POOP_SIZE 45.0f
#define CLIMB_POOP_SIZE 72.0f

typedef enum {
    CLIMB_MOVE_NONE = 0,
    CLIMB_MOVE_UP,
    CLIMB_MOVE_LEFT,
    CLIMB_MOVE_RIGHT
} ClimbMove;

static void movePuddle(Puddle *puddle, float deltaX);
static void syncTowerHitbox(Stage3 *stage);
static Vector2 ambientToWorld(Stage3 *stage, Player *player, Vector2 localPosition);
static void resetStage3ClimbBirds(Stage3 *stage);

static Texture2D finalClimbBackgrounds[FINAL_CLIMB_BACKGROUND_COUNT] = {0};
static Texture2D finalClimbTowerFrames[2] = {0};
static Texture2D finalClimbCharacterFrames[2] = {0};
static Texture2D stage3FloorTexture = {0};
static Texture2D stage3PuddleTexture = {0};
static Texture2D stage3BottleTexture = {0};
static Texture2D stage3SkyTexture = {0};
static int climbChallengeActive = 0;
static ClimbMove climbChallengeMove = CLIMB_MOVE_NONE;
static float climbPointerAngle = 0.0f;
static float climbPointerSpeed = 0.0f;
static float climbGreenStart = 0.0f;
static float climbGreenSize = 0.0f;
static float climbChallengeTimer = 0.0f;
static int climbMissCount = 0;
static ClimbMove climbAutoMove = CLIMB_MOVE_NONE;
static float climbAutoDistance = 0.0f;

static float clampFloat(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static float normalizeDegrees(float angle) {
    angle = fmodf(angle, 360.0f);
    if (angle < 0.0f) {
        angle += 360.0f;
    }
    return angle;
}

static bool angleInArc(float angle, float start, float size) {
    angle = normalizeDegrees(angle);
    start = normalizeDegrees(start);
    float end = normalizeDegrees(start + size);

    if (size >= 360.0f) {
        return true;
    }

    if (start <= end) {
        return angle >= start && angle <= end;
    }

    return angle >= start || angle <= end;
}

static void resetClimbChallenge(void) {
    climbChallengeActive = 0;
    climbChallengeMove = CLIMB_MOVE_NONE;
    climbChallengeTimer = 0.0f;
    climbMissCount = 0;
    climbAutoMove = CLIMB_MOVE_NONE;
    climbAutoDistance = 0.0f;
}

static void startClimbChallenge(ClimbMove move) {
    if (climbChallengeActive || climbAutoMove != CLIMB_MOVE_NONE) {
        return;
    }

    climbChallengeActive = 1;
    climbChallengeMove = move;
    climbPointerAngle = (float)(rand() % 360);
    climbPointerSpeed = 170.0f + (float)(rand() % 260);
    climbGreenSize = 34.0f + (float)(rand() % 35);
    climbGreenStart = (float)(rand() % 360);
    climbChallengeTimer = 0.0f;
}

static void failClimbChallenge(Player *player) {
    climbChallengeActive = 0;
    climbChallengeMove = CLIMB_MOVE_NONE;
    climbChallengeTimer = 0.0f;
    climbMissCount++;

    if (climbMissCount >= CLIMB_MAX_MISSES) {
        player->lives = 0;
        player->state = PLAYER_STATE_DEAD;
        resetClimbChallenge();
    }
}

static void resolveClimbChallenge(Player *player) {
    if (!climbChallengeActive) {
        return;
    }

    if (angleInArc(climbPointerAngle, climbGreenStart, climbGreenSize)) {
        climbAutoMove = climbChallengeMove;
        climbAutoDistance = (climbAutoMove == CLIMB_MOVE_UP) ? CLIMB_STEP_UP : CLIMB_STEP_SIDE;
        climbChallengeActive = 0;
        climbChallengeMove = CLIMB_MOVE_NONE;
        climbChallengeTimer = 0.0f;
    } else {
        failClimbChallenge(player);
    }
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

static float stage3CameraZoom(void) {
    float scaleX = GetScreenWidth() / WORLD_WIDTH;
    float scaleY = GetScreenHeight() / WORLD_HEIGHT;
    return ((scaleX < scaleY) ? scaleX : scaleY) * CAMERA_ZOOM_FACTOR;
}

static Vector2 cameraTargetForStage(Stage3 *stage, Player *player) {
    float zoom = stage3CameraZoom();
    float targetY = player->position.y + player->height / 2.0f - CAMERA_VERTICAL_LOOKAHEAD;

    if (stage->state == STAGE3_APPROACH) {
        float floorScreenY = GetScreenHeight() * 0.88f;
        targetY = TOWER_BASE_Y - (floorScreenY - GetScreenHeight() * 0.5f) / zoom;
    }

    return (Vector2){
        player->position.x + PLAYER_WIDTH / 2.0f,
        targetY
    };
}

static Rectangle cameraWorldRect(Stage3 *stage, Player *player, float padding) {
    Vector2 target = cameraTargetForStage(stage, player);
    float width = visibleWorldWidth();
    float height = visibleWorldHeight();

    return (Rectangle){
        target.x - width * 0.5f - padding,
        target.y - height * 0.5f - padding,
        width + padding * 2.0f,
        height + padding * 2.0f
    };
}

static Vector2 ambientToWorld(Stage3 *stage, Player *player, Vector2 localPosition) {
    Rectangle view = cameraWorldRect(stage, player, 0.0f);

    return (Vector2){
        view.x + localPosition.x,
        view.y + localPosition.y
    };
}

static float stage3SkyBirdY(void) {
    return 34.0f + (float)(rand() % 76);
}

static float stage3BirdInterval(void) {
    return 1.0f + (rand() % 250) / 100.0f;
}

static void randomizeStage3BirdFlow(Bird *bird, float baseY, float amplitudeMin, float amplitudeRange) {
    bird->baseY = baseY;
    bird->position.y = baseY;
    bird->wavePhase = (float)(rand() % 628) / 100.0f;
    bird->waveSpeed = 0.9f + (float)(rand() % 170) / 100.0f;
    bird->waveAmplitude = amplitudeMin + (float)(rand() % 100) / 100.0f * amplitudeRange;
}

static void resetStage3ApproachBird(Bird *bird, int index) {
    bird->position.x = GetScreenWidth() + (index * 300) + (rand() % 200);
    randomizeStage3BirdFlow(bird, stage3SkyBirdY(), 8.0f, 18.0f);
    bird->speed = 55.0f + (rand() % 65);
    bird->poopTimer = 0.0f;
    bird->poopInterval = stage3BirdInterval();
}

static void resetStage3ClimbBird(Bird *bird, int index) {
    bool fromRight = (index % 2) == 0;
    float offset = 80.0f + (float)((index * 95) + (rand() % 160));
    int verticalRange = GetScreenHeight() - 180;
    float baseY = 48.0f + (float)(rand() % (verticalRange > 80 ? verticalRange : 80));

    bird->position.x = fromRight ? GetScreenWidth() + offset : -180.0f - offset;
    randomizeStage3BirdFlow(bird, baseY, 18.0f, 34.0f);
    bird->speed = (fromRight ? 1.0f : -1.0f) * (230.0f + (rand() % 170));
    bird->poopTimer = 0.0f;
    bird->poopInterval = stage3BirdInterval();
}

static void resetStage3ClimbBirds(Stage3 *stage) {
    for (int i = 0; i < STAGE3_MAX_BIRDS; i++) {
        resetStage3ClimbBird(&stage->birds[i], i);
    }
}

static float visibleWorldLeft(void) {
    return (WORLD_WIDTH - visibleWorldWidth()) * 0.5f;
}

static float towerDrawWidth(Stage3 *stage) {
    return stage->towerTexture.id > 0 ? 680.0f : 160.0f;
}

static float towerDrawHeight(Stage3 *stage) {
    return stage->towerTexture.id > 0 ? 680.0f : 680.0f;
}

static Rectangle towerSourceRect(Stage3 *stage) {
    if (stage->towerTexture.id > 0) {
        return (Rectangle){ 0.0f, 0.0f, (float)stage->towerTexture.width, (float)stage->towerTexture.height };
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
    return x < stage->towerPosition.x - PUDDLE_WIDTH * 0.45f;
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

static void drawTextureInRect(Texture2D texture, Rectangle dest) {
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    DrawTexturePro(texture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
}

static void drawPuddle(Puddle puddle) {
    if (!puddle.active) {
        return;
    }

    float x = puddle.position.x;
    float y = puddle.position.y;

    if (stage3PuddleTexture.id > 0) {
        drawTextureInRect(stage3PuddleTexture, (Rectangle){ x, y, PUDDLE_WIDTH, PUDDLE_HEIGHT });
        return;
    }

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

    if (stage3BottleTexture.id > 0) {
        drawTextureInRect(stage3BottleTexture, (Rectangle){ x, y, BOTTLE_WIDTH, BOTTLE_HEIGHT });
        return;
    }

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

static int drawPoopTexture(Texture2D texture, Vector2 position, float size, float rotation) {
    if (texture.id <= 0) {
        return 0;
    }

    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle dest = {
        position.x,
        position.y,
        size,
        size
    };

    DrawTexturePro(texture, source, dest, (Vector2){size * 0.5f, size * 0.5f}, rotation, WHITE);
    return 1;
}

static void drawFallingPoop(Texture2D texture, Vector2 position, float rotation) {
    if (drawPoopTexture(texture, position, 45.0f, rotation)) {
        return;
    }

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

static void drawLandedPoop(Texture2D texture, Vector2 position) {
    if (drawPoopTexture(texture, position, 54.0f, 0.0f)) {
        return;
    }

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
    if (stage3SkyTexture.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)stage3SkyTexture.width, (float)stage3SkyTexture.height };
        Rectangle dest = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawTexturePro(stage3SkyTexture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        return;
    }

    float climbProgress = 0.0f;
    if (stage->state == STAGE3_CLIMBING) {
        climbProgress = clampFloat((TOWER_BASE_Y - player->position.y) / (towerDrawHeight(stage) * 0.82f), 0.0f, 1.0f);
    } else if (stage->state == STAGE3_FINISHED) {
        climbProgress = 1.0f;
    }
    float sunrise = clampFloat(fmaxf(-stage->scrollY / 650.0f, climbProgress), 0.0f, 1.0f);
    float rainIntensity = 1.0f - sunrise;
    float left = 0.0f;
    float top = 0.0f;
    float width = (float)GetScreenWidth();
    float height = (float)GetScreenHeight();
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
    float sunY = top + height * (0.56f - sunrise * 0.32f);
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

static void drawStage3Floor(Stage3 *stage, Player *player, float floorY) {
    Rectangle view = cameraWorldRect(stage, player, 0.0f);
    float left = view.x - WORLD_WIDTH;
    float width = view.width + WORLD_WIDTH * 2.0f;
    float height = visibleWorldHeight();
    float drawY = floorY + 96.0f;
    float tileSize = 32.0f;
    float floorScroll = stage->scrollX * 0.95f;
    int baseCol = (int)floorf(floorScroll / tileSize);
    float offset = floorScroll - (baseCol * tileSize);
    int visibleCols = (int)(width / tileSize) + 18;

    if (stage3FloorTexture.id > 0) {
        Rectangle source = {
            0.0f,
            (float)stage3FloorTexture.height * 0.43f,
            (float)stage3FloorTexture.width,
            (float)stage3FloorTexture.height * 0.27f
        };
        float platformWidth = view.width * 1.18f;
        float platformHeight = platformWidth * (source.height / source.width);
        float roadY = floorY - 42.0f;
        Rectangle dest = {
            view.x + view.width * 0.5f - platformWidth * 0.5f,
            roadY,
            platformWidth,
            platformHeight
        };

        DrawTexturePro(stage3FloorTexture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        return;
    }

    DrawRectangle((int)left - 120, (int)drawY, (int)width + 240, (int)(height + WORLD_HEIGHT),
                  (Color){ 176, 96, 48, 255 });

    for (int row = 0; row < 4; row++) {
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

static void updateStage3PlayerVisualState(Player *player) {
    bool movingLeft = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    bool movingRight = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);

    if (movingRight) {
        player->direction = 'R';
    } else if (movingLeft) {
        player->direction = 'L';
    }

    if (player->velocity.x > 1.0f) {
        player->direction = 'R';
    }

    if (!player->grounded || player->velocity.y != 0.0f) {
        player->state = player->velocity.y < 0.0f ? PLAYER_STATE_JUMPING : PLAYER_STATE_FALLING;
    } else if (movingLeft || movingRight || fabsf(player->velocity.x) > 1.0f) {
        player->state = PLAYER_STATE_RUNNING;
    } else {
        player->state = PLAYER_STATE_IDLE;
    }
}

static void syncStage3PlayerHitbox(Player *player) {
    player->hitbox = (Rectangle){
        player->position.x + 12.0f,
        player->position.y + 8.0f,
        player->width - 24.0f,
        player->height - 10.0f
    };
}

static void drawStage3Player(Player *player) {
    Texture2D currentSprite = {0};

    if (player->state == PLAYER_STATE_JUMPING || player->state == PLAYER_STATE_FALLING || !player->grounded) {
        currentSprite = player->direction == 'R' ? player->spriteJumpingR : player->spriteJumpingL;
    } else if (player->state == PLAYER_STATE_RUNNING) {
        currentSprite = player->direction == 'R' ? player->spriteMovingR : player->spriteMovingL;
    } else {
        currentSprite = player->direction == 'R' ? player->spriteStandingR : player->spriteStandingL;
    }

    if (currentSprite.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)currentSprite.width, (float)currentSprite.height };
        Rectangle dest = { player->position.x, player->position.y, player->width, player->height };
        DrawTexturePro(currentSprite, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(player->hitbox, RED);
    }
}

static void drawClimbTimingCircle(Vector2 playerScreenPosition, float playerWidth, float playerHeight) {
    if (!climbChallengeActive) {
        return;
    }

    float radius = 34.0f;
    Vector2 center = {
        playerScreenPosition.x + playerWidth + radius + 14.0f,
        playerScreenPosition.y + playerHeight * 0.35f
    };

    if (center.x + radius + 10.0f > GetScreenWidth()) {
        center.x = playerScreenPosition.x - radius - 14.0f;
    }
    if (center.y - radius < 10.0f) {
        center.y = radius + 10.0f;
    }
    if (center.y + radius > GetScreenHeight() - 10.0f) {
        center.y = GetScreenHeight() - radius - 10.0f;
    }

    DrawCircleV(center, radius + 7.0f, (Color){ 5, 12, 24, 185 });
    DrawRing(center, radius - 4.0f, radius + 4.0f, 0.0f, 360.0f, 48, (Color){ 238, 242, 247, 130 });
    DrawRing(center, radius - 5.0f, radius + 5.0f, climbGreenStart, climbGreenStart + climbGreenSize, 20,
             (Color){ 75, 232, 118, 255 });

    float pointerRad = normalizeDegrees(climbPointerAngle) * DEG2RAD;
    Vector2 pointerEnd = {
        center.x + cosf(pointerRad) * (radius + 1.0f),
        center.y + sinf(pointerRad) * (radius + 1.0f)
    };
    DrawLineEx(center, pointerEnd, 3.0f, (Color){ 255, 246, 115, 255 });
    DrawCircleV(center, 4.0f, RAYWHITE);
}

static float finalClimbLocalProgress(Stage3 *stage, Player *player, int *backgroundIndex) {
    float climbBottom = TOWER_BASE_Y - player->height;
    float climbTop = stage->towerPosition.y + 60.0f;
    float climbProgress = clampFloat((climbBottom - player->position.y) / (climbBottom - climbTop), 0.0f, 1.0f);
    float stageProgress = climbProgress * FINAL_CLIMB_BACKGROUND_COUNT;
    int index = (int)stageProgress;
    float localClimbProgress = stageProgress - index;

    if (index >= FINAL_CLIMB_BACKGROUND_COUNT) {
        index = FINAL_CLIMB_BACKGROUND_COUNT - 1;
        localClimbProgress = 1.0f;
    }

    if (backgroundIndex != NULL) {
        *backgroundIndex = index;
    }

    return localClimbProgress;
}

static Rectangle finalClimbTowerDest(float screenWidth, float screenHeight) {
    float towerWidth = clampFloat(screenWidth * 0.58f, 560.0f, 1100.0f);
    return (Rectangle){
        (screenWidth - towerWidth) * 0.5f,
        0.0f,
        towerWidth,
        screenHeight
    };
}

static Rectangle finalClimbPlayerDest(Stage3 *stage, Player *player, float screenWidth, float screenHeight) {
    int unusedIndex = 0;
    float localClimbProgress = finalClimbLocalProgress(stage, player, &unusedIndex);
    Rectangle towerDest = finalClimbTowerDest(screenWidth, screenHeight);
    float playerY = screenHeight * (0.80f - localClimbProgress * 0.62f);
    float climbRange = stage->towerHitbox.width - PLAYER_WIDTH;
    float horizontalProgress = 0.5f;

    if (climbRange > 1.0f) {
        horizontalProgress = clampFloat((player->position.x - stage->towerHitbox.x) / climbRange, 0.0f, 1.0f);
    }

    float climbLeft = towerDest.x + towerDest.width * 0.22f;
    float climbRight = towerDest.x + towerDest.width * 0.78f - FINAL_CLIMB_PLAYER_WIDTH;

    return (Rectangle){
        climbLeft + (climbRight - climbLeft) * horizontalProgress,
        playerY,
        FINAL_CLIMB_PLAYER_WIDTH,
        FINAL_CLIMB_PLAYER_HEIGHT
    };
}

static Rectangle stage3BirdScreenRect(Stage3 *stage, int index) {
    float birdScale = stage->state == STAGE3_CLIMBING ? CLIMB_BIRD_SCALE : STAGE3_BIRD_SCALE;
    Texture2D birdSprite = stage->birdTexture.id > 0 ? stage->birdTexture : stage->birdTextureAlt;
    float width = birdSprite.id > 0 ? (float)birdSprite.width * birdScale : 48.0f;
    float height = birdSprite.id > 0 ? (float)birdSprite.height * birdScale : 32.0f;

    return (Rectangle){
        stage->birds[index].position.x,
        stage->birds[index].position.y,
        width,
        height
    };
}

static Rectangle stage3BirdHitbox(Stage3 *stage, int index) {
    Rectangle rect = stage3BirdScreenRect(stage, index);

    return (Rectangle){
        rect.x + rect.width * 0.18f,
        rect.y + rect.height * 0.20f,
        rect.width * 0.64f,
        rect.height * 0.60f
    };
}

static void drawStage3ScreenHazards(Stage3 *stage) {
    for (int i = 0; i < STAGE3_MAX_BIRDS; i++) {
        if (stage->birds[i].position.x < -220.0f || stage->birds[i].position.x > GetScreenWidth() + 520.0f) {
            continue;
        }

        Texture2D birdSprite = ((int)(GetTime() * 8.0f + i) % 2 == 0)
            ? stage->birdTexture
            : stage->birdTextureAlt;

        if (birdSprite.id > 0) {
            Rectangle source = { 0.0f, 0.0f, (float)birdSprite.width, (float)birdSprite.height };
            Rectangle dest = stage3BirdScreenRect(stage, i);

            if (stage->birds[i].speed < 0.0f) {
                source.width = -source.width;
            }

            DrawTexturePro(birdSprite, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        } else {
            float direction = stage->birds[i].speed < 0.0f ? 1.0f : -1.0f;
            DrawTriangle(
                (Vector2){stage->birds[i].position.x + direction * 16.0f, stage->birds[i].position.y},
                (Vector2){stage->birds[i].position.x - direction * 8.0f, stage->birds[i].position.y - 7.0f},
                (Vector2){stage->birds[i].position.x - direction * 8.0f, stage->birds[i].position.y + 7.0f},
                BLACK
            );
        }
    }

    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
        if (stage->poops[i].active) {
            if (stage->poops[i].landed) {
                drawLandedPoop(stage->poopTexture, stage->poops[i].position);
            } else {
                float poopSize = stage->state == STAGE3_CLIMBING ? CLIMB_POOP_SIZE : STAGE3_POOP_SIZE;
                if (!drawPoopTexture(stage->poopTexture, stage->poops[i].position, poopSize, stage->poops[i].rotationZ)) {
                    drawFallingPoop(stage->poopTexture, stage->poops[i].position, stage->poops[i].rotationZ);
                }
            }
        }
    }
}

static void loadFinalClimbTextures(void) {
    const char *backgroundPaths[FINAL_CLIMB_BACKGROUND_COUNT] = {
        "assets/img/paisagemFinal1.png",
        "assets/img/paisagemFinal2.png",
        "assets/img/paisagemFinal3.png",
        "assets/img/paisagemFinal4.png",
        "assets/img/PaisagemFinal5.png"
    };
    const char *towerPaths[2] = {
        "assets/img/Brenand1.png",
        "assets/img/Brenand2.png"
    };
    const char *characterPaths[2] = {
        "assets/img/CharactherClibing1.png",
        "assets/img/CharactherClibing2.png"
    };

    for (int i = 0; i < FINAL_CLIMB_BACKGROUND_COUNT; i++) {
        if (finalClimbBackgrounds[i].id == 0) {
            finalClimbBackgrounds[i] = LoadTexture(backgroundPaths[i]);
        }
    }

    for (int i = 0; i < 2; i++) {
        if (finalClimbTowerFrames[i].id == 0) {
            finalClimbTowerFrames[i] = LoadTexture(towerPaths[i]);
        }
    }

    for (int i = 0; i < 2; i++) {
        if (finalClimbCharacterFrames[i].id == 0) {
            finalClimbCharacterFrames[i] = LoadTexture(characterPaths[i]);
        }
    }
}

static void unloadFinalClimbTextures(void) {
    for (int i = 0; i < FINAL_CLIMB_BACKGROUND_COUNT; i++) {
        if (finalClimbBackgrounds[i].id > 0) {
            UnloadTexture(finalClimbBackgrounds[i]);
            finalClimbBackgrounds[i] = (Texture2D){0};
        }
    }

    for (int i = 0; i < 2; i++) {
        if (finalClimbTowerFrames[i].id > 0) {
            UnloadTexture(finalClimbTowerFrames[i]);
            finalClimbTowerFrames[i] = (Texture2D){0};
        }
    }

    for (int i = 0; i < 2; i++) {
        if (finalClimbCharacterFrames[i].id > 0) {
            UnloadTexture(finalClimbCharacterFrames[i]);
            finalClimbCharacterFrames[i] = (Texture2D){0};
        }
    }
}

static void loadStage3MapTextures(void) {
    if (stage3FloorTexture.id == 0) {
        stage3FloorTexture = LoadTexture("assets/img/piso.png");
    }
    if (stage3PuddleTexture.id == 0) {
        stage3PuddleTexture = LoadTexture("assets/img/poca.png");
    }
    if (stage3BottleTexture.id == 0) {
        stage3BottleTexture = LoadTexture("assets/img/garrafa.png");
    }
    if (stage3SkyTexture.id == 0) {
        stage3SkyTexture = LoadTexture("assets/img/ceu.png");
    }
}

static void unloadStage3MapTextures(void) {
    if (stage3FloorTexture.id > 0) {
        UnloadTexture(stage3FloorTexture);
        stage3FloorTexture = (Texture2D){0};
    }
    if (stage3PuddleTexture.id > 0) {
        UnloadTexture(stage3PuddleTexture);
        stage3PuddleTexture = (Texture2D){0};
    }
    if (stage3BottleTexture.id > 0) {
        UnloadTexture(stage3BottleTexture);
        stage3BottleTexture = (Texture2D){0};
    }
    if (stage3SkyTexture.id > 0) {
        UnloadTexture(stage3SkyTexture);
        stage3SkyTexture = (Texture2D){0};
    }
}

static void drawFinalClimbScene(Stage3 *stage, Player *player) {
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();
    int backgroundIndex = 0;
    finalClimbLocalProgress(stage, player, &backgroundIndex);

    Texture2D background = finalClimbBackgrounds[backgroundIndex];
    if (background.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)background.width, (float)background.height };
        Rectangle dest = { 0.0f, 0.0f, screenWidth, screenHeight };
        DrawTexturePro(background, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(),
                               (Color){ 45, 77, 128, 255 },
                               (Color){ 229, 158, 92, 255 });
    }

    Texture2D towerFrame = finalClimbTowerFrames[backgroundIndex % 2];
    Rectangle towerDest = finalClimbTowerDest(screenWidth, screenHeight);

    if (towerFrame.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)towerFrame.width, (float)towerFrame.height };
        DrawTexturePro(towerFrame, source, towerDest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(towerDest, (Color){ 174, 151, 116, 255 });
        DrawRectangleLinesEx(towerDest, 2.0f, (Color){ 104, 78, 55, 255 });
    }

    if (backgroundIndex == 0) {
        drawRain(0.0f, 0.0f, screenWidth, screenHeight, 1.0f);
    }

    int climbFrame = (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) ||
                      IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) ||
                      IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) ||
                      climbChallengeActive ||
                      climbAutoMove != CLIMB_MOVE_NONE) ? 1 : 0;
    Texture2D currentSprite = finalClimbCharacterFrames[climbFrame];
    if (currentSprite.id == 0) {
        currentSprite = player->direction == 'R' ? player->spriteJumpingR : player->spriteJumpingL;
    }
    if (currentSprite.id == 0) {
        currentSprite = player->direction == 'R' ? player->spriteStandingR : player->spriteStandingL;
    }

    Rectangle playerDest = finalClimbPlayerDest(stage, player, screenWidth, screenHeight);

    if (currentSprite.id > 0) {
        Rectangle source = { 0.0f, 0.0f, (float)currentSprite.width, (float)currentSprite.height };
        DrawTexturePro(currentSprite, source, playerDest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(playerDest, RED);
    }

    drawStage3ScreenHazards(stage);
    drawClimbTimingCircle((Vector2){ playerDest.x, playerDest.y }, playerDest.width, playerDest.height);
}

void initStage3(Stage3 *stage, Player *player) {
    stage->state = STAGE3_APPROACH;
    stage->scrollX = 0.0f;
    stage->scrollY = 0.0f;
    stage->puddleLockTimer = 0.0f;
    stage->puddlePushVelocity = 0.0f;
    stage->ambientSpawningEnabled = true;
    resetClimbChallenge();
    
    stage->towerTexture = LoadTexture("assets/img/brenadFinal.png");
    stage->cloudTexture = LoadTexture("assets/img/nuvem.png");
    stage->birdTexture = LoadTexture("assets/img/pigeon1L.png");
    stage->birdTextureAlt = LoadTexture("assets/img/pigeon2L.png");
    stage->poopTexture = LoadTexture("assets/img/pigeonPoop.png");
    loadFinalClimbTextures();
    loadStage3MapTextures();
    
    // Posiciona a torre mais para a direita para expandir o mapa (1200px)
    // Desenha apenas a area visivel do PNG para a base da torre ficar exatamente no chao.
    stage->towerPosition = (Vector2){ TOWER_START_X, TOWER_BASE_Y - towerDrawHeight(stage) + TOWER_STAGE3_VERTICAL_OFFSET };
    syncTowerHitbox(stage);
    
    // Staggered cloud positions, extremely high up, with randomized speed and scale
    stage->clouds[0].position = (Vector2){ rand() % 350, 10 + (rand() % 30) };
    stage->clouds[0].speed = 10.0f + (rand() % 25);
    stage->clouds[0].scale = 0.15f + (rand() % 10) / 100.0f; // Scale between 0.15 and 0.25
    
    stage->clouds[1].position = (Vector2){ 450 + (rand() % 300), 10 + (rand() % 30) };
    stage->clouds[1].speed = 10.0f + (rand() % 25);
    stage->clouds[1].scale = 0.15f + (rand() % 10) / 100.0f;
    
    // Inicializa 3 pássaros com distâncias e timers de cocô individuais
    stage->birds[0].position = (Vector2){ GetScreenWidth() * 0.25f, stage3SkyBirdY() };
    stage->birds[0].speed = 50.0f + (rand() % 30);
    randomizeStage3BirdFlow(&stage->birds[0], stage->birds[0].position.y, 8.0f, 18.0f);
    stage->birds[0].poopTimer = 0.0f;
    stage->birds[0].poopInterval = 1.0f + (rand() % 200) / 100.0f;
    
    stage->birds[1].position = (Vector2){ GetScreenWidth() * 0.65f, stage3SkyBirdY() };
    stage->birds[1].speed = 45.0f + (rand() % 40);
    randomizeStage3BirdFlow(&stage->birds[1], stage->birds[1].position.y, 8.0f, 18.0f);
    stage->birds[1].poopTimer = 0.0f;
    stage->birds[1].poopInterval = 1.0f + (rand() % 200) / 100.0f;
    
    stage->birds[2].position = (Vector2){ GetScreenWidth() + 180.0f + (rand() % 220), stage3SkyBirdY() };
    stage->birds[2].speed = 60.0f + (rand() % 30);
    randomizeStage3BirdFlow(&stage->birds[2], stage->birds[2].position.y, 8.0f, 18.0f);
    stage->birds[2].poopTimer = 0.0f;
    stage->birds[2].poopInterval = 1.0f + (rand() % 200) / 100.0f;
    
    player->width = 96.0f;
    player->height = 120.0f;
    player->position = (Vector2){ PLAYER_CENTER_X, TOWER_BASE_Y - player->height };
    player->isClimbing = false;
    player->velocity = (Vector2){0, 0};
    player->speed = 0.0f; // Para o auto-run
    player->on_bike = false;
    player->isGrounded = true;
    player->grounded = true;
    player->direction = 'R';
    player->state = PLAYER_STATE_IDLE;
    player->movementControlledExternally = true; // Stage 3 controla o movimento
    syncStage3PlayerHitbox(player);
    
    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
        stage->poops[i].active = false;
        stage->poops[i].landed = false;
        stage->poops[i].groundTimer = 0.0f;
        stage->poops[i].speedY = 0.0f;
        stage->poops[i].rotationZ = 0.0f;
    }

    float puddleX = 760.0f + (float)(rand() % 220);
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

    if (player->slowEffectTimer > 0.0f) {
        player->slowEffectTimer -= deltaTime;
        if (player->slowEffectTimer <= 0.0f) {
            player->slowEffectTimer = 0.0f;
            player->speedMultiplier = 1.0f;
        }
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
        stage->birds[i].position.y = stage->birds[i].baseY +
            sinf((float)GetTime() * stage->birds[i].waveSpeed + stage->birds[i].wavePhase) *
            stage->birds[i].waveAmplitude;

        if (stage->state == STAGE3_APPROACH && stage->birds[i].position.x < -150) {
            resetStage3ApproachBird(&stage->birds[i], i);
        } else if (stage->state == STAGE3_CLIMBING) {
            bool leftMovingBirdExited = stage->birds[i].speed >= 0.0f && stage->birds[i].position.x < -180.0f;
            bool rightMovingBirdExited = stage->birds[i].speed < 0.0f && stage->birds[i].position.x > GetScreenWidth() + 180.0f;

            if (leftMovingBirdExited || rightMovingBirdExited) {
                resetStage3ClimbBird(&stage->birds[i], i);
            }
        }
    }

    if (stage->state == STAGE3_APPROACH) {
        float moveSpeed = (player->grounded ? 230.0f : 315.0f) * player->speedMultiplier;
        float moveDelta = moveSpeed * deltaTime;
        float scrollDelta = 0.0f;
        float maxScrollX = approachMaxScroll(stage);
        float towerAppearScroll = TOWER_START_X -
                                  (PLAYER_CENTER_X + PLAYER_WIDTH * 0.5f +
                                   visibleWorldWidth() * 0.5f - 120.0f);
        float autoStopScroll = clampFloat(towerAppearScroll, 0.0f, maxScrollX);
        bool movementLockedByPuddle = stage->puddleLockTimer > 0.0f;
        float groundY = TOWER_BASE_Y - player->height;

        if (stage->puddleLockTimer > 0.0f) {
            stage->puddleLockTimer -= deltaTime;
            if (stage->puddleLockTimer <= 0.0f) {
                stage->puddleLockTimer = 0.0f;
                stage->puddlePushVelocity = 0.0f;
            }
        }

        if (!movementLockedByPuddle && player->grounded &&
            (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))) {
            player->velocity.y = -STAGE3_JUMP_FORCE;
            player->grounded = false;
            player->isGrounded = false;
        }

        if (!player->grounded) {
            player->velocity.y += STAGE3_GRAVITY * deltaTime;
            player->position.y += player->velocity.y * deltaTime;

            if (player->position.y >= groundY) {
                player->position.y = groundY;
                player->velocity.y = 0.0f;
                player->grounded = true;
                player->isGrounded = true;
            }
        }
        
        player->velocity.x = 0.0f;
        bool towerVisible = stage->scrollX >= autoStopScroll - 0.5f;
        if (!movementLockedByPuddle) {
            if (!towerVisible) {
                scrollDelta = moveDelta;
                if (stage->scrollX + scrollDelta > autoStopScroll) {
                    scrollDelta = autoStopScroll - stage->scrollX;
                }
                player->velocity.x = moveSpeed;
            }

            float lateralSpeed = moveSpeed * 1.15f;
            float lateralDelta = lateralSpeed * deltaTime;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
                player->position.x -= lateralDelta;
                player->velocity.x = -lateralSpeed;
                if (!towerVisible) {
                    scrollDelta *= 0.82f;
                }
            }
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
                player->position.x += lateralDelta;
                player->velocity.x = lateralSpeed;
                if (!towerVisible) {
                    scrollDelta *= 1.08f;
                }
            }
        }

        if (stage->scrollX + scrollDelta > autoStopScroll) {
            scrollDelta = autoStopScroll - stage->scrollX;
        }

        if (scrollDelta != 0.0f) {
            applyHorizontalScroll(stage, scrollDelta);
        }

        float nextPuddleX = visibleWorldLeft() + visibleWorldWidth() + 120.0f;
        for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
            bool shouldResetPuddle = !stage->puddles[i].active ||
                                     stage->puddles[i].position.x < visibleWorldLeft() - PUDDLE_WIDTH;

            if (shouldResetPuddle && stage->scrollX < autoStopScroll - 120.0f) {
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
            player->position.x += pushScrollDelta;
            player->velocity.x = stage->puddlePushVelocity;

            stage->puddlePushVelocity *= 0.96f;
        }

        float barrierX = stage->towerPosition.x + towerWidth + 150.0f;
        if (player->position.x + PLAYER_WIDTH > barrierX) {
            player->position.x = barrierX - PLAYER_WIDTH;
        }
        if (player->position.x < PLAYER_LEFT_LIMIT) {
            player->position.x = PLAYER_LEFT_LIMIT;
        }
        float rightLimit = towerVisible ? stage->towerPosition.x + towerWidth - player->width : PLAYER_RIGHT_LIMIT;
        if (player->position.x > rightLimit) {
            player->position.x = rightLimit;
        }

        // Verifica se o player está à frente da torre (sobreposto no eixo X)
        bool isOverlappingTowerX = (player->position.x + PLAYER_WIDTH >= stage->towerHitbox.x) && 
                                   (player->position.x <= stage->towerHitbox.x + stage->towerHitbox.width);
        bool isNearTower = (player->position.x + PLAYER_WIDTH >= stage->towerHitbox.x - 80.0f) &&
                           (player->position.x <= stage->towerHitbox.x + stage->towerHitbox.width + 80.0f);

        if (isNearTower || !stage->ambientSpawningEnabled) {
            stage->ambientSpawningEnabled = true;
        }
        
        // Se estiver sobreposto e pressionar W ou SETA PARA CIMA, inicia a escalada
        if (isOverlappingTowerX) {
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
                stage->state = STAGE3_CLIMBING;
                resetClimbChallenge();
                player->isClimbing = true;
                player->velocity.y = 0;
                if (player->lives < 3) {
                    healPlayer(player);
                }
                player->position.x = stage->towerHitbox.x + stage->towerHitbox.width / 2.0f - PLAYER_WIDTH / 2.0f;
                resetStage3ClimbBirds(stage);
                for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
                    stage->poops[i].active = false;
                    stage->poops[i].landed = false;
                    stage->poops[i].speedY = 0.0f;
                    stage->poops[i].rotationZ = 0.0f;
                }
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
                player->invincibilityTimer = 0.0f;
                damagePlayer(player, 220.0f);
                stage->puddles[i].active = false;
                break;
            }

            if (playerOnPuddle && stage->puddles[i].canLockPlayer) {
                stage->puddleLockTimer = 0.52f;
                stage->puddlePushVelocity = 700.0f;
                stage->puddles[i].canLockPlayer = false;
                break;
            } else if (!playerOnPuddle) {
                stage->puddles[i].canLockPlayer = true;
            }
        }
        
    } else if (stage->state == STAGE3_CLIMBING) {
        stage->ambientSpawningEnabled = false;
        stage->scrollY = -clampFloat(TOWER_BASE_Y - player->position.y, 0.0f, towerDrawHeight(stage));

        clearPuddlesFromTowerArea(stage);

        float climbAutoSpeed = CLIMB_AUTO_SPEED;
        player->velocity.x = 0.0f;

        if (climbAutoMove == CLIMB_MOVE_NONE) {
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                startClimbChallenge(CLIMB_MOVE_UP);
            } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                startClimbChallenge(CLIMB_MOVE_LEFT);
            } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                startClimbChallenge(CLIMB_MOVE_RIGHT);
            }
        }

        if (climbChallengeActive) {
            climbChallengeTimer += deltaTime;
            climbPointerAngle = normalizeDegrees(climbPointerAngle + climbPointerSpeed * deltaTime);
            if (climbChallengeTimer >= CLIMB_CHALLENGE_LIMIT) {
                failClimbChallenge(player);
            }
            if (IsKeyPressed(KEY_SPACE)) {
                resolveClimbChallenge(player);
            }
        }

        if (climbAutoMove != CLIMB_MOVE_NONE) {
            float step = climbAutoSpeed * deltaTime;
            if (step > climbAutoDistance) {
                step = climbAutoDistance;
            }

            if (climbAutoMove == CLIMB_MOVE_UP) {
                player->position.y -= step;
                player->state = PLAYER_STATE_JUMPING;
            } else if (climbAutoMove == CLIMB_MOVE_LEFT) {
                player->position.x -= step;
                player->velocity.x = -climbAutoSpeed;
                player->direction = 'L';
            } else if (climbAutoMove == CLIMB_MOVE_RIGHT) {
                player->position.x += step;
                player->velocity.x = climbAutoSpeed;
                player->direction = 'R';
            }

            climbAutoDistance -= step;
            if (climbAutoDistance <= 0.0f) {
                climbAutoMove = CLIMB_MOVE_NONE;
                climbAutoDistance = 0.0f;
            }
        }

        float climbBottom = TOWER_BASE_Y - player->height;
        float climbTop = stage->towerPosition.y + 60.0f;
        if (player->position.y > climbBottom) player->position.y = climbBottom;
        if (player->position.y < climbTop) player->position.y = climbTop;
        
        if (player->position.x < stage->towerHitbox.x) player->position.x = stage->towerHitbox.x;
        if (player->position.x + PLAYER_WIDTH > stage->towerHitbox.x + stage->towerHitbox.width) player->position.x = stage->towerHitbox.x + stage->towerHitbox.width - PLAYER_WIDTH;
        
        if (player->position.y <= stage->towerPosition.y + 80.0f) { 
            stage->state = STAGE3_FINISHED;
            player->isClimbing = false; // finalizou subida
        }
    }
    
    // ===== GERENCIAMENTO DE COCO DOS PASSAROS =====
    bool birdPoopEnabled = stage->ambientSpawningEnabled && stage->state == STAGE3_APPROACH;
    if (birdPoopEnabled) {
        for (int b = 0; b < STAGE3_MAX_BIRDS; b++) {
            stage->birds[b].poopTimer += deltaTime;
            if (stage->birds[b].poopTimer >= stage->birds[b].poopInterval) {
                stage->birds[b].poopTimer = 0.0f;
                // Randomiza o próximo intervalo individual do pássaro
                stage->birds[b].poopInterval = 1.0f + (rand() % 350) / 100.0f;
                
                // Spawna apenas se o pássaro estiver na tela
                if (stage->birds[b].position.x > -20.0f && stage->birds[b].position.x < GetScreenWidth() + 20.0f) {
                    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
                        if (!stage->poops[i].active) {
                            float poopOffsetX = stage->state == STAGE3_CLIMBING
                                ? (stage->birds[b].speed < 0.0f ? 52.0f : 22.0f)
                                : (stage->birds[b].speed < 0.0f ? 30.0f : 8.0f);
                            stage->poops[i].position = (Vector2){ stage->birds[b].position.x + poopOffsetX, stage->birds[b].position.y + 10.0f };
                            stage->poops[i].active = true;
                            stage->poops[i].landed = false;
                            stage->poops[i].groundTimer = 0.0f;
                            stage->poops[i].speedY = 0.0f;
                            stage->poops[i].rotationZ = 0.0f;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    syncStage3PlayerHitbox(player);

    // Atualiza poops
    Rectangle playerScreenHitbox = {0};
    if (stage->state == STAGE3_CLIMBING) {
        Rectangle climbPlayerDest = finalClimbPlayerDest(stage, player, (float)GetScreenWidth(), (float)GetScreenHeight());
        playerScreenHitbox = (Rectangle){
            climbPlayerDest.x + climbPlayerDest.width * 0.24f,
            climbPlayerDest.y + climbPlayerDest.height * 0.12f,
            climbPlayerDest.width * 0.52f,
            climbPlayerDest.height * 0.78f
        };
    } else {
        Camera2D collisionCamera = {0};
        collisionCamera.target = cameraTargetForStage(stage, player);
        collisionCamera.offset = (Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
        collisionCamera.rotation = 0.0f;
        collisionCamera.zoom = stage3CameraZoom();
        Vector2 playerScreenPosition = GetWorldToScreen2D((Vector2){ player->hitbox.x, player->hitbox.y }, collisionCamera);
        playerScreenHitbox = (Rectangle){
            playerScreenPosition.x,
            playerScreenPosition.y,
            player->hitbox.width * collisionCamera.zoom,
            player->hitbox.height * collisionCamera.zoom
        };
    }

    if (stage->state == STAGE3_CLIMBING) {
        for (int i = 0; i < STAGE3_MAX_BIRDS; i++) {
            Rectangle birdHitbox = stage3BirdHitbox(stage, i);
            if (CheckCollisionRecs(playerScreenHitbox, birdHitbox)) {
                damagePlayer(player, 0.0f);
                resetStage3ClimbBird(&stage->birds[i], i);
            }
        }
    }

    for (int i = 0; i < STAGE3_MAX_BIRD_POOPS; i++) {
        if (stage->poops[i].active) {
            stage->poops[i].speedY += 400.0f * deltaTime;
            stage->poops[i].position.y += stage->poops[i].speedY * deltaTime;
            stage->poops[i].rotationZ += 360.0f * deltaTime;

            if (stage->poops[i].rotationZ >= 360.0f) {
                stage->poops[i].rotationZ = fmodf(stage->poops[i].rotationZ, 360.0f);
            }

            if (stage->poops[i].position.y >= GetScreenHeight() - 64.0f) {
                stage->poops[i].active = false;
                continue;
            }
            
            float poopSize = stage->state == STAGE3_CLIMBING ? CLIMB_POOP_SIZE : STAGE3_POOP_SIZE;
            Rectangle poopHitbox = {
                stage->poops[i].position.x - poopSize * 0.5f,
                stage->poops[i].position.y - poopSize * 0.5f,
                poopSize,
                poopSize
            };
            if (CheckCollisionRecs(playerScreenHitbox, poopHitbox)) {
                if (stage->state == STAGE3_CLIMBING) {
                    player->lives = 0;
                    player->state = PLAYER_STATE_DEAD;
                    resetClimbChallenge();
                } else if (player->hasUmbrella <= 0) {
                    applySlowDown(player, 50.0f, 2.0f);
                }
                stage->poops[i].active = false;
            }
        }
    }

    syncStage3PlayerHitbox(player);
    updateStage3PlayerVisualState(player);
}

void drawStage3(Stage3 *stage, Player *player) {
    if (stage->state == STAGE3_CLIMBING || stage->state == STAGE3_FINISHED) {
        drawFinalClimbScene(stage, player);
        return;
    }

    Camera2D camera = {0};
    camera.target = cameraTargetForStage(stage, player);
    camera.offset = (Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
    camera.rotation = 0.0f;
    camera.zoom = stage3CameraZoom();

    drawStage3Background(stage, player);

    BeginMode2D(camera);
    
    float floorY = TOWER_BASE_Y;
    
    drawStage3Floor(stage, player, floorY);

    for (int i = 0; i < STAGE3_MAX_PUDDLES; i++) {
        drawPuddle(stage->puddles[i]);
        drawBottle(stage->puddles[i]);
    }
    
    for (int i = 0; i < STAGE3_MAX_CLOUDS; i++) {
        Vector2 cloudPosition = ambientToWorld(stage, player, stage->clouds[i].position);
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

    drawStage3Player(player);

    EndMode2D();

    drawStage3ScreenHazards(stage);
}

void unloadStage3(Stage3 *stage) {
    if (stage->towerTexture.id > 0) UnloadTexture(stage->towerTexture);
    if (stage->cloudTexture.id > 0) UnloadTexture(stage->cloudTexture);
    if (stage->birdTexture.id > 0) UnloadTexture(stage->birdTexture);
    if (stage->birdTextureAlt.id > 0) UnloadTexture(stage->birdTextureAlt);
    if (stage->poopTexture.id > 0) UnloadTexture(stage->poopTexture);
    unloadFinalClimbTextures();
    unloadStage3MapTextures();
}
