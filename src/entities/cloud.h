#ifndef CLOUD_H
#define CLOUD_H

#include <raylib.h>

#define MAX_CLOUDS 8
#define MAX_RAINDROPS_PER_CLOUD 50

#define CLOUD_WIDTH 80.0f
#define CLOUD_HEIGHT 40.0f

#define SCREEN_WIDTH 1920.0f
#define SCREEN_HEIGHT 1080.0f

#define GROUND_LEVEL 360.0f

// ==========================================
// GOTA DE CHUVA
// ==========================================
typedef struct {

    Vector2 position;

    float speed;

    float depth;

    int active;

    Texture2D dropTexture;

    int textureLoaded;

} RainDrop;

// ==========================================
// NUVEM
// ==========================================
typedef struct {

    Vector2 position;

    float speed;

    float depth;

    Rectangle hitbox;

    int active;

    float rainTimer;

    float rainInterval;

    float intensity;

    Texture2D cloudTexture;

    int textureLoaded;

    RainDrop drops[MAX_RAINDROPS_PER_CLOUD];

    int dropCount;

    float scale;

    Color color;

} CloudEntity;

typedef CloudEntity Cloud;

// ==========================================
// SISTEMA DE NUVENS
// ==========================================
typedef struct {

    CloudEntity clouds[MAX_CLOUDS];

    int cloudCount;

    float spawnTimer;

    float spawnInterval;

    float rainIntensity;

} CloudSystem;

// ==========================================
// FUNÇÕES
// ==========================================
CloudSystem createCloudSystem(void);

void updateCloudSystem(
    CloudSystem *system,
    float scrollSpeed,
    float deltaTime
);

void drawCloudSystem(
    CloudSystem system
);

void resetCloudSystem(
    CloudSystem *system
);

void setRainIntensity(
    CloudSystem *system,
    float intensity
);

#endif