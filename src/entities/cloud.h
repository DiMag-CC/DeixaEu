#ifndef CLOUD_H
#define CLOUD_H

#include <raylib.h>

#define MAX_CLOUDS 5
#define MAX_RAINDROPS_PER_CLOUD 50
#define CLOUD_WIDTH 100.0f
#define CLOUD_HEIGHT 40.0f
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define GROUND_LEVEL 350.0f

// ========== ESTRUTURA DE GOTA DE CHUVA ==========
typedef struct {
    Vector2 position;
    float speed;
    float depth;        // 0.0 = fundo, 1.0 = frente (parallax)
    int active;
} RainDrop;

// ========== ESTRUTURA DA NUVEM ==========
typedef struct {
    Vector2 position;
    float speed;        // Velocidade horizontal
    float depth;        // Parallax: quanto mais baixo, mais rápido move
    Rectangle hitbox;
    int active;
    float rainTimer;    // Timer para gerar chuva
    float rainInterval; // Intervalo entre gotas
    float intensity;    // Intensidade da chuva (1.0 = normal, 2.0 = forte)

    // Gotas de chuva gerenciadas por essa nuvem
    RainDrop drops[MAX_RAINDROPS_PER_CLOUD];
    int dropCount;

    // Escala e aparência
    float scale;
    Color color;

} Cloud;

// ========== SISTEMA DE NUVENS ==========
typedef struct {
    Cloud clouds[MAX_CLOUDS];
    int cloudCount;
    float spawnTimer;
    float spawnInterval;
    float rainIntensity;  // Intensidade geral da chuva (0.5 a 2.0)
} CloudSystem;

// ========== FUNÇÕES ==========
CloudSystem createCloudSystem(void);
void updateCloudSystem(CloudSystem *system, float scrollSpeed, float deltaTime);
void drawCloudSystem(CloudSystem system);
void resetCloudSystem(CloudSystem *system);
void setRainIntensity(CloudSystem *system, float intensity);

#endif
