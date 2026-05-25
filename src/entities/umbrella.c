#include "umbrella.h"
#include "../utils/gameConstants.h"
#include <math.h>

Umbrella createUmbrella(Vector2 position) {
    Umbrella umbrella;
    umbrella.position = position;
    umbrella.active = 1;
    umbrella.speed = 0.0f;
    umbrella.rotationAngle = 0.0f;

    umbrella.spriteLoaded = 0;
    umbrella.texture = LoadTexture("assets/img/umbrella.png");
    if (umbrella.texture.id != 0) {
        umbrella.spriteLoaded = 1;
    }

    // Hitbox
    umbrella.hitbox = (Rectangle){
        umbrella.position.x - UMBRELLA_WIDTH / 2,
        umbrella.position.y - UMBRELLA_HEIGHT / 2,
        UMBRELLA_WIDTH,
        UMBRELLA_HEIGHT
    };

    return umbrella;
}

void updateUmbrella(Umbrella *umbrella, float scrollSpeed, float deltaTime) {
    if (!umbrella->active) return;

    umbrella->position.x -= scrollSpeed * deltaTime;

    // Rotação contínua (animação)
    umbrella->rotationAngle += 360.0f * deltaTime;
    if (umbrella->rotationAngle >= 360.0f) {
        umbrella->rotationAngle = 0.0f;
    }

    // Movimento vertical (flutuar)
    umbrella->position.y += sinf(umbrella->rotationAngle * PI / 180.0f) * 30.0f * deltaTime;

    // Atualizar hitbox
    umbrella->hitbox.x = umbrella->position.x - UMBRELLA_WIDTH / 2;
    umbrella->hitbox.y = umbrella->position.y - UMBRELLA_HEIGHT / 2;

    // Deativar se sair da tela
    if (umbrella->position.x < -UMBRELLA_WIDTH) {
        umbrella->active = 0;
    }
}

void drawUmbrella(Umbrella umbrella) {
    if (!umbrella.active) return;

    float scaledWidth = UMBRELLA_WIDTH;
    float scaledHeight = UMBRELLA_HEIGHT;

    if (umbrella.spriteLoaded) {
        // Usar DrawTexturePro com rotação
        Rectangle source = { 0, 0, (float)umbrella.texture.width, (float)umbrella.texture.height };
        Rectangle dest = {
            umbrella.position.x - scaledWidth / 2,
            umbrella.position.y - scaledHeight / 2,
            scaledWidth,
            scaledHeight
        };
        Vector2 origin = { scaledWidth / 2, scaledHeight / 2 };
        DrawTexturePro(umbrella.texture, source, dest, origin, umbrella.rotationAngle, WHITE);
    } else {
        Vector2 offset = {
            cosf(umbrella.rotationAngle * PI / 180.0f) * 5.0f,
            sinf(umbrella.rotationAngle * PI / 180.0f) * 5.0f
        };
        Vector2 rotatedPos = { umbrella.position.x + offset.x, umbrella.position.y + offset.y };

        DrawCircleSector(rotatedPos, UMBRELLA_WIDTH / 2, umbrella.rotationAngle, umbrella.rotationAngle + 180, 10, GREEN);
        DrawCircleSectorLines(rotatedPos, UMBRELLA_WIDTH / 2, umbrella.rotationAngle, umbrella.rotationAngle + 180, 10, DARKGREEN);

        // Cabo
        DrawLine((int)rotatedPos.x, (int)rotatedPos.y,
                (int)(rotatedPos.x + cosf((umbrella.rotationAngle + 270) * PI / 180.0f) * UMBRELLA_HEIGHT / 2),
                (int)(rotatedPos.y + sinf((umbrella.rotationAngle + 270) * PI / 180.0f) * UMBRELLA_HEIGHT / 2),
                DARKGREEN);
    }
}

void unloadUmbrellaResources(Umbrella *umbrella) {
    if (umbrella->spriteLoaded) {
        UnloadTexture(umbrella->texture);
        umbrella->spriteLoaded = 0;
    }
}
