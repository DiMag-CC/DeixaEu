#include "bike.h"
#include <math.h>

// ========== CRIAR BICICLETA ==========
Bike createBike(void) {
    Bike bike;
    bike.position = (Vector2){ 0, 0 };
    bike.wheelAngle = 0.0f;
    bike.wheelSpeed = 0.0f;

    bike.spriteLoaded = 0;
    bike.bikeTexture = LoadTexture("assets/img/bike.png");
    if (bike.bikeTexture.id != 0) {
        bike.spriteLoaded = 1;
    }

    return bike;
}

// ========== ATUALIZAR BICICLETA ==========
void updateBike(Bike *bike, Player *player, float deltaTime) {
    // Bike segue o player
    bike->position = player->position;

    // Atualizar velocidade de rotação das rodas
    float bikeSpeedRatio = fabsf(player->velocity.x) / 200.0f;
    if (bikeSpeedRatio > 1.0f) bikeSpeedRatio = 1.0f;

    bike->wheelSpeed = bikeSpeedRatio * 3600.0f;
    bike->wheelAngle += bike->wheelSpeed * deltaTime;

    if (bike->wheelAngle >= 360.0f) {
        bike->wheelAngle = 0.0f;
    }
}

// ========== DESENHAR BICICLETA ==========
void drawBike(Bike bike, Player player) {
    float bikeX = player.position.x;
    float bikeY = player.position.y;

    if (bike.spriteLoaded) {
        DrawTextureEx(bike.bikeTexture,
                     (Vector2){ bikeX - BIKE_WIDTH / 2, bikeY - BIKE_HEIGHT },
                     0, 1.0f, WHITE);
    } else {
        // Placeholder: desenhar bicicleta como formas simples
        // Quadro
        DrawRectangle(bikeX - BIKE_WIDTH / 2, bikeY - BIKE_HEIGHT, BIKE_WIDTH, BIKE_HEIGHT / 2, ORANGE);

        // Roda dianteira
        DrawCircleLines(bikeX + BIKE_WIDTH / 4, bikeY, WHEEL_RADIUS, BLACK);
        float wheelX1 = bikeX + BIKE_WIDTH / 4 + WHEEL_RADIUS * cosf((bike.wheelAngle * PI) / 180.0f);
        float wheelY1 = bikeY + WHEEL_RADIUS * sinf((bike.wheelAngle * PI) / 180.0f);
        DrawLine(bikeX + BIKE_WIDTH / 4, bikeY, wheelX1, wheelY1, BLACK);

        // Roda traseira
        DrawCircleLines(bikeX - BIKE_WIDTH / 4, bikeY, WHEEL_RADIUS, BLACK);
        float wheelX2 = bikeX - BIKE_WIDTH / 4 + WHEEL_RADIUS * cosf((bike.wheelAngle * PI) / 180.0f);
        float wheelY2 = bikeY + WHEEL_RADIUS * sinf((bike.wheelAngle * PI) / 180.0f);
        DrawLine(bikeX - BIKE_WIDTH / 4, bikeY, wheelX2, wheelY2, BLACK);
    }
}

// ========== DESCARREGAR RECURSOS ==========
void unloadBikeResources(Bike *bike) {
    if (bike->spriteLoaded) {
        UnloadTexture(bike->bikeTexture);
        bike->spriteLoaded = 0;
    }
}
