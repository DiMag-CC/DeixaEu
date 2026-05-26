#include "bus.h"
#include "../utils/gameConstants.h"

#define BUS_SPEED_MULTIPLIER 1.5f

Bus createBus(Vector2 position) {

    Bus bus;

    bus.position = position;

    bus.active = 1;

    bus.speed = 150.0f * BUS_SPEED_MULTIPLIER;

    bus.scale = 1.0f;

    bus.spriteLoaded = 0;

    bus.texture = LoadTexture(
        "assets/img/busR.png"
    );

    if (bus.texture.id != 0) {
        bus.spriteLoaded = 1;
    }

    const float BUS_TARGET_WIDTH = 320.0f;
    const float BUS_TARGET_HEIGHT = 140.0f;

    bus.hitbox = (Rectangle){

        bus.position.x - BUS_TARGET_WIDTH / 2,

        bus.position.y - BUS_TARGET_HEIGHT,

        BUS_TARGET_WIDTH,

        BUS_TARGET_HEIGHT
    };

    return bus;
}


void updateBus(Bus *bus, float scrollSpeed, float deltaTime) {

    if (!bus->active) return;

    // Movimento lateral
    bus->position.x -=
        (scrollSpeed + bus->speed) * deltaTime;

    // Escala visual usada também na hitbox
    const float BUS_TARGET_WIDTH = 320.0f;
    const float BUS_TARGET_HEIGHT = 140.0f;

    // Ground global
    float groundY =
        GLOBAL_GROUND_LEVEL;

    // Atualizar hitbox
    bus->hitbox.x =
        bus->position.x - BUS_TARGET_WIDTH / 2;

    bus->hitbox.y =
        groundY - BUS_TARGET_HEIGHT;

    bus->hitbox.width =
        BUS_TARGET_WIDTH;

    bus->hitbox.height =
        BUS_TARGET_HEIGHT;

    // Desativar fora da tela
    if (bus->position.x < -BUS_TARGET_WIDTH) {
        bus->active = 0;
    }
}

void drawBus(Bus bus) {
    if (!bus.active) return;

    const float BUS_TARGET_WIDTH = 420.0f;
    const float BUS_TARGET_HEIGHT = 320.0f;

    float scaledWidth = BUS_TARGET_WIDTH;
    float scaledHeight = BUS_TARGET_HEIGHT;

    // Posicionar alinhado ao chão
    float groundY = GLOBAL_GROUND_LEVEL + 140.0f;

    Rectangle dest = {
        bus.position.x - scaledWidth / 2,
        groundY - scaledHeight,
        scaledWidth,
        scaledHeight
    };

    if (bus.spriteLoaded && bus.texture.id != 0) {

        Rectangle source = {
            0,
            0,
            (float)bus.texture.width,
            (float)bus.texture.height
        };

        DrawTexturePro(
            bus.texture,
            source,
            dest,
            (Vector2){0, 0},
            0,
            WHITE
        );

    } else {

        DrawRectangleRec(dest, ORANGE);
        DrawRectangleLinesEx(dest, 2, BLACK);

        // Rodas
        DrawCircle(
            (int)(dest.x + 50),
            (int)(dest.y + dest.height - 12),
            14,
            BLACK
        );

        DrawCircle(
            (int)(dest.x + dest.width - 50),
            (int)(dest.y + dest.height - 12),
            14,
            BLACK
        );

        // Janelas
        DrawRectangle(
            (int)(dest.x + 40),
            (int)(dest.y + 25),
            45,
            30,
            SKYBLUE
        );

        DrawRectangle(
            (int)(dest.x + 100),
            (int)(dest.y + 25),
            45,
            30,
            SKYBLUE
        );

        DrawRectangle(
            (int)(dest.x + 160),
            (int)(dest.y + 25),
            45,
            30,
            SKYBLUE
        );
    }
}

void unloadBusResources(Bus *bus) {
    if (bus->spriteLoaded) {
        UnloadTexture(bus->texture);
        bus->spriteLoaded = 0;
    }
}
