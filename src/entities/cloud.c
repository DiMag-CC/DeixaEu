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

    system.spawnInterval =
        CLOUD_SPAWN_INTERVAL;

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

    cloud.depth = depth;

    cloud.speed =
        CLOUD_SPEED_MIN +
        (float)(rand() % 40);

    cloud.rainTimer = 0.0f;

    cloud.rainInterval =
        0.1f / depth;

    cloud.intensity =
        0.5f + (depth * 1.5f);

    cloud.scale =
        0.5f + (depth * 0.5f);

    cloud.dropCount = 0;

    float darkFactor = depth * 0.3f;

    cloud.color = (Color){
        (unsigned char)(220 - darkFactor * 50),
        (unsigned char)(220 - darkFactor * 50),
        (unsigned char)(240 - darkFactor * 30),
        255
    };

    cloud.hitbox = (Rectangle){
        cloud.position.x - (CLOUD_WIDTH * cloud.scale) / 2,
        cloud.position.y - (CLOUD_HEIGHT * cloud.scale) / 2,
        CLOUD_WIDTH * cloud.scale,
        CLOUD_HEIGHT * cloud.scale
    };

        cloud.cloudTexture =
        LoadTexture("assets/img/RainCloud.png");

    cloud.textureLoaded =
        (cloud.cloudTexture.id != 0);

    for (int i = 0; i < MAX_RAINDROPS_PER_CLOUD; i++) {

        cloud.drops[i].active = 0;

        cloud.drops[i].position =
            (Vector2){ 0, 0 };

        cloud.drops[i].depth = depth;

        cloud.drops[i].speed =
            RAINDROP_SPEED_MIN +
            (float)(rand() % 150);

        cloud.drops[i].dropTexture =
            LoadTexture("assets/img/RainDrops.png");

        cloud.drops[i].textureLoaded =
            (cloud.drops[i].dropTexture.id != 0);
    }

    return cloud;
}

static void drawCloud(
    CloudEntity cloud
) {

    if (!cloud.active) return;

    if (
        cloud.textureLoaded &&
        cloud.cloudTexture.id != 0
    ) {

        float width =
            CLOUD_WIDTH *
            cloud.scale * 3.5f;

        float height =
            CLOUD_HEIGHT *
            cloud.scale * 2.8f;

        Rectangle source = {
            0.0f,
            0.0f,
            (float)cloud.cloudTexture.width,
            (float)cloud.cloudTexture.height
        };

        Rectangle dest = {
            cloud.position.x - width * 0.5f,
            cloud.position.y - height * 0.5f,
            width,
            height
        };

        DrawTexturePro(
            cloud.cloudTexture,
            source,
            dest,
            (Vector2){0,0},
            0.0f,
            WHITE
        );

    } else {

        DrawCircle(
            cloud.position.x,
            cloud.position.y,
            40,
            LIGHTGRAY
        );
    }
}

void updateCloudSystem(
    CloudSystem *system,
    float scrollSpeed,
    float deltaTime
) {

    system->spawnTimer += deltaTime;

    if (
        system->spawnTimer >= system->spawnInterval &&
        system->cloudCount < MAX_CLOUDS
    ) {

        system->spawnTimer = 0.0f;

        float randomDepth =
            0.3f +
            (float)(rand() % 70) / 100.0f;

        Vector2 spawnPos = {
            SCREEN_WIDTH + 50,
            62.0f + (float)(rand() % 180)
        };

        for (int i = 0; i < MAX_CLOUDS; i++) {

            if (!system->clouds[i].active) {

                system->clouds[i] =
                    createCloudAtPosition(
                        spawnPos,
                        randomDepth
                    );

                system->cloudCount++;

                break;
            }
        }
    }

    for (int i = 0; i < MAX_CLOUDS; i++) {

        CloudEntity *cloud =
            &system->clouds[i];

        if (!cloud->active) continue;

        cloud->position.x -=
            (scrollSpeed * cloud->depth) *
            deltaTime;

        cloud->rainTimer += deltaTime;

        if (cloud->rainTimer >= cloud->rainInterval) {

            cloud->rainTimer = 0.0f;

            for (int j = 0; j < MAX_RAINDROPS_PER_CLOUD; j++) {

                if (!cloud->drops[j].active) {

                    cloud->drops[j].position =
                        (Vector2){

                        cloud->position.x +
                        (float)(rand() %
                        (int)(CLOUD_WIDTH * cloud->scale)),

                        cloud->position.y +
                        CLOUD_HEIGHT *
                        cloud->scale / 2
                    };

                    cloud->drops[j].speed =
                        RAINDROP_SPEED_MIN +
                        (float)(rand() % 150);

                    cloud->drops[j].active = 1;

                    cloud->dropCount++;

                    break;
                }
            }
        }

        for (int j = 0; j < MAX_RAINDROPS_PER_CLOUD; j++) {

            if (cloud->drops[j].active) {

                cloud->drops[j].position.y +=
                    cloud->drops[j].speed *
                    deltaTime;

                if (
                    cloud->drops[j].position.y > SCREEN_HEIGHT ||
                    cloud->drops[j].position.x < -20
                ) {

                    cloud->drops[j].active = 0;

                    cloud->dropCount--;
                }
            }
        }

        cloud->hitbox.x =
            cloud->position.x -
            (CLOUD_WIDTH * cloud->scale) / 2;

        cloud->hitbox.y =
            cloud->position.y -
            (CLOUD_HEIGHT * cloud->scale) / 2;

        if (cloud->position.x < -CLOUD_WIDTH * 2) {

            cloud->active = 0;

            system->cloudCount--;
        }
    }
}

void drawCloudSystem(CloudSystem system) {

    // Nuvens distantes
    for (int i = 0; i < MAX_CLOUDS; i++) {

        if (
            system.clouds[i].active &&
            system.clouds[i].depth < 0.5f
        ) {

            drawCloud(system.clouds[i]);
        }
    }

    // Gotas de chuva
    for (int i = 0; i < MAX_CLOUDS; i++) {

        CloudEntity cloud =
            system.clouds[i];

        if (!cloud.active) continue;

        for (int j = 0; j < MAX_RAINDROPS_PER_CLOUD; j++) {

            if (cloud.drops[j].active) {

                Rectangle source = {
                    .x = 0,
                    .y = 0,
                    (float)cloud.drops[j].dropTexture.width,
                    (float)cloud.drops[j].dropTexture.height
                };

                float size =
                    21.0f +
                    (cloud.depth * 16.0f);

                Rectangle dest = {
                    cloud.drops[j].position.x,
                    cloud.drops[j].position.y,
                    size,
                    size * 3.0f
                };

                DrawTexturePro(
                    cloud.drops[j].dropTexture,
                    source,
                    dest,
                    (Vector2){0,0},
                    12.0f,
                    WHITE
                );
            }
        }
    }

    // Nuvens próximas
    for (int i = 0; i < MAX_CLOUDS; i++) {

        if (
            system.clouds[i].active &&
            system.clouds[i].depth >= 0.5f
        ) {

            drawCloud(system.clouds[i]);
        }
    }
}

void resetCloudSystem(CloudSystem *system) {

    system->cloudCount = 0;

    system->spawnTimer = 0.0f;

    for (int i = 0; i < MAX_CLOUDS; i++) {

        system->clouds[i].active = 0;
    }

    for (int i = 0; i < MAX_CLOUDS; i++) {

    if (
        system->clouds[i].textureLoaded &&
        system->clouds[i].cloudTexture.id != 0
    ) {

        UnloadTexture(
            system->clouds[i].cloudTexture
        );
    }

    for (int j = 0; j < MAX_RAINDROPS_PER_CLOUD; j++) {

        if (
            system->clouds[i].drops[j].textureLoaded &&
            system->clouds[i].drops[j].dropTexture.id != 0
        ) {

            UnloadTexture(
                system->clouds[i].drops[j].dropTexture
            );
        }
    }
}
}

void setRainIntensity(
    CloudSystem *system,
    float intensity
) {

    if (intensity < 0.5f) {
        intensity = 0.5f;
    }

    if (intensity > 2.0f) {
        intensity = 2.0f;
    }

    system->rainIntensity = intensity;
}