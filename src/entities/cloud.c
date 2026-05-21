#include "cloud.h"
#include <stdlib.h>
#include <math.h>

#define CLOUD_SPEED_MIN 40.0f
#define CLOUD_SPEED_MAX 80.0f
#define RAINDROP_SPEED_MIN 200.0f
#define RAINDROP_SPEED_MAX 350.0f
#define CLOUD_SPAWN_INTERVAL 3.0f


CloudSystem createCloudSystem(void) {
    CloudSystem system;
    system.cloudCount = 0;
    system.spawnTimer = 0.0f;
    system.spawnInterval = CLOUD_SPAWN_INTERVAL;
    system.rainIntensity = 1.0f;

    for (int i = 0; i < MAX_CLOUDS; i++) {
        system.clouds[i].active = 0;
    }

    return system;
}

static CloudEntity createCloudAtPosition(Vector2 position, float depth) {
    CloudEntity cloud;
    cloud.position = position;
    cloud.active = 1;
    cloud.depth = depth;  // 0.0 = distante (pouco parallax), 1.0 = próxima (muito parallax)
    cloud.speed = CLOUD_SPEED_MIN + (float)(rand() % 40);
    cloud.rainTimer = 0.0f;
    cloud.rainInterval = 0.1f / depth;  // Nuvens distantes chovem menos
    cloud.intensity = 0.5f + (depth * 1.5f);  // Nuvens distantes chovem fraco
    cloud.scale = 0.5f + (depth * 0.5f);  // Variar tamanho com depth
    cloud.dropCount = 0;

    // Cor mais escura para nuvens mais próximas
    float darkFactor = depth * 0.3f;
    cloud.color = (Color){ (unsigned char)(220 - darkFactor * 50),
                          (unsigned char)(220 - darkFactor * 50),
                          (unsigned char)(240 - darkFactor * 30),
                          255 };

    cloud.hitbox = (Rectangle){
        cloud.position.x - (CLOUD_WIDTH * cloud.scale) / 2,
        cloud.position.y - (CLOUD_HEIGHT * cloud.scale) / 2,
        CLOUD_WIDTH * cloud.scale,
        CLOUD_HEIGHT * cloud.scale
    };

    // Inicializar gotas
    for (int i = 0; i < MAX_RAINDROPS_PER_CLOUD; i++) {
        cloud.drops[i].active = 0;
        cloud.drops[i].position = (Vector2){ 0, 0 };
        cloud.drops[i].depth = depth;
        cloud.drops[i].speed = RAINDROP_SPEED_MIN + (float)(rand() % 150);
    }

    return cloud;
}

// ========== DESENHAR NUVEM (PLACEHOLDER) ==========
static void drawCloud(CloudEntity cloud) {
    if (!cloud.active) return;

    float scaledWidth = CLOUD_WIDTH * cloud.scale;

    // Desenhar nuvem com 3 círculos
    float cx = cloud.position.x;
    float cy = cloud.position.y;
    float size = scaledWidth / 3;

    DrawCircle(cx - size / 2, cy, size, cloud.color);
    DrawCircle(cx, cy, size * 1.2f, cloud.color);
    DrawCircle(cx + size / 2, cy, size, cloud.color);

    // Contorno
    DrawCircleLines(cx - size / 2, cy, size, (Color){ 100, 100, 120, 255 });
    DrawCircleLines(cx, cy, size * 1.2f, (Color){ 100, 100, 120, 255 });
    DrawCircleLines(cx + size / 2, cy, size, (Color){ 100, 100, 120, 255 });
}

// ========== ATUALIZAR SISTEMA DE NUVENS ==========
void updateCloudSystem(CloudSystem *system, float scrollSpeed, float deltaTime) {
    // Spawnar novas nuvens
    system->spawnTimer += deltaTime;
    if (system->spawnTimer >= system->spawnInterval && system->cloudCount < MAX_CLOUDS) {
        system->spawnTimer = 0.0f;

        // Spawn com depth aleatória (layering)
        float randomDepth = 0.3f + (float)(rand() % 70) / 100.0f;
        Vector2 spawnPos = { SCREEN_WIDTH + 50, 50.0f + (float)(rand() % 100) };

        for (int i = 0; i < MAX_CLOUDS; i++) {
            if (!system->clouds[i].active) {
                system->clouds[i] = createCloudAtPosition(spawnPos, randomDepth);
                system->cloudCount++;
                break;
            }
        }
    }

    // Atualizar cada nuvem
    for (int i = 0; i < MAX_CLOUDS; i++) {
        CloudEntity *cloud = &system->clouds[i];
        if (!cloud->active) continue;

        // Movimento horizontal (paralaxe baseado em depth)
        cloud->position.x -= (scrollSpeed * cloud->depth) * deltaTime;

        // Gerar chuva
        cloud->rainTimer += deltaTime;
        if (cloud->rainTimer >= cloud->rainInterval) {
            cloud->rainTimer = 0.0f;

            // Encontrar espaço livre para nova gota
            for (int j = 0; j < MAX_RAINDROPS_PER_CLOUD; j++) {
                if (!cloud->drops[j].active) {
                    cloud->drops[j].position = (Vector2){
                        cloud->position.x + (float)(rand() % (int)(CLOUD_WIDTH * cloud->scale)),
                        cloud->position.y + CLOUD_HEIGHT * cloud->scale / 2
                    };
                    cloud->drops[j].speed = RAINDROP_SPEED_MIN + (float)(rand() % 150);
                    cloud->drops[j].active = 1;
                    cloud->dropCount++;
                    break;
                }
            }
        }

        // Atualizar gotas
        for (int j = 0; j < MAX_RAINDROPS_PER_CLOUD; j++) {
            if (cloud->drops[j].active) {
                cloud->drops[j].position.y += cloud->drops[j].speed * deltaTime;

                // Remover gota fora da tela
                if (cloud->drops[j].position.y > GROUND_LEVEL || cloud->drops[j].position.x < -20) {
                    cloud->drops[j].active = 0;
                    cloud->dropCount--;
                }
            }
        }

        // Atualizar hitbox
        cloud->hitbox.x = cloud->position.x - (CLOUD_WIDTH * cloud->scale) / 2;
        cloud->hitbox.y = cloud->position.y - (CLOUD_HEIGHT * cloud->scale) / 2;

        // Remover nuvem fora da tela
        if (cloud->position.x < -CLOUD_WIDTH * 2) {
            cloud->active = 0;
            system->cloudCount--;
        }
    }
}

// ========== DESENHAR SISTEMA DE NUVENS ==========
void drawCloudSystem(CloudSystem system) {
    // Desenhar em duas camadas: primeiro nuvens distantes, depois gotas, depois nuvens próximas

    // Camada 1: Nuvens distantes (depth < 0.5)
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (system.clouds[i].active && system.clouds[i].depth < 0.5f) {
            drawCloud(system.clouds[i]);
        }
    }

    // Camada 2: Gotas de chuva (todas as nuvens)
    for (int i = 0; i < MAX_CLOUDS; i++) {
        CloudEntity cloud = system.clouds[i];
        if (!cloud.active) continue;

        for (int j = 0; j < MAX_RAINDROPS_PER_CLOUD; j++) {
            if (cloud.drops[j].active) {
                float alpha = cloud.depth * 0.8f;  // Gotas distantes são mais transparentes
                Color rainColor = (Color){
                    (unsigned char)(100 + cloud.depth * 100),
                    (unsigned char)(150 + cloud.depth * 100),
                    (unsigned char)(255),
                    (unsigned char)(200 * alpha)
                };

                // Desenhar gota como linha inclinada
                float lineLen = 8.0f + (cloud.depth * 4.0f);
                DrawLine(
                    cloud.drops[j].position.x,
                    cloud.drops[j].position.y,
                    cloud.drops[j].position.x + 2,
                    cloud.drops[j].position.y + lineLen,
                    rainColor
                );
            }
        }
    }

    // Camada 3: Nuvens próximas (depth >= 0.5)
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (system.clouds[i].active && system.clouds[i].depth >= 0.5f) {
            drawCloud(system.clouds[i]);
        }
    }
}

// ========== RESETAR SISTEMA DE NUVENS ==========
void resetCloudSystem(CloudSystem *system) {
    system->cloudCount = 0;
    system->spawnTimer = 0.0f;

    for (int i = 0; i < MAX_CLOUDS; i++) {
        system->clouds[i].active = 0;
    }
}

// ========== CONFIGURAR INTENSIDADE DA CHUVA ==========
void setRainIntensity(CloudSystem *system, float intensity) {
    if (intensity < 0.5f) intensity = 0.5f;
    if (intensity > 2.0f) intensity = 2.0f;
    system->rainIntensity = intensity;
}
