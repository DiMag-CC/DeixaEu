#include "bike.h"
#include <math.h>

Bike createBike(void) {
    Bike bike;
    bike.position = (Vector2){ 0, 0 };
    bike.wheelAngle = 0.0f;
    bike.wheelSpeed = 0.0f;
    bike.scale = 0.8f;

    bike.spriteLoaded = 0;
    bike.bikeTexture = LoadTexture("assets/img/bikeStanding.png");
    if (bike.bikeTexture.id != 0) {
        bike.spriteLoaded = 1;
    }

    return bike;
}

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
        float scaledWidth = BIKE_WIDTH * bike.scale;
        float scaledHeight = BIKE_HEIGHT * bike.scale;
        DrawTextureEx(bike.bikeTexture,
                     (Vector2){ bikeX - scaledWidth / 2, bikeY - scaledHeight },
                     0, bike.scale, WHITE);
    } else {
        // Placeholder: desenhar bicicleta como formas simples
        float scaledWidth = BIKE_WIDTH * bike.scale;
        float scaledHeight = BIKE_HEIGHT * bike.scale;
        float scaledWheelRadius = WHEEL_RADIUS * bike.scale;

        // Quadro
        DrawRectangle(bikeX - scaledWidth / 2, bikeY - scaledHeight, scaledWidth, scaledHeight / 2, ORANGE);

        // Roda dianteira
        DrawCircleLines(bikeX + scaledWidth / 4, bikeY, scaledWheelRadius, BLACK);
        float wheelX1 = bikeX + scaledWidth / 4 + scaledWheelRadius * cosf((bike.wheelAngle * PI) / 180.0f);
        float wheelY1 = bikeY + scaledWheelRadius * sinf((bike.wheelAngle * PI) / 180.0f);
        DrawLine(bikeX + scaledWidth / 4, bikeY, wheelX1, wheelY1, BLACK);

        // Roda traseira
        DrawCircleLines(bikeX - scaledWidth / 4, bikeY, scaledWheelRadius, BLACK);
        float wheelX2 = bikeX - scaledWidth / 4 + scaledWheelRadius * cosf((bike.wheelAngle * PI) / 180.0f);
        float wheelY2 = bikeY + scaledWheelRadius * sinf((bike.wheelAngle * PI) / 180.0f);
        DrawLine(bikeX - scaledWidth / 4, bikeY, wheelX2, wheelY2, BLACK);
    }
}

// ========== DESCARREGAR RECURSOS ==========
void unloadBikeResources(Bike *bike) {
    if (bike->spriteLoaded) {
        UnloadTexture(bike->bikeTexture);
        bike->spriteLoaded = 0;
    }
}
