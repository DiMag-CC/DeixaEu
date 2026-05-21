#include "bus.h"

#define BUS_SPEED_MULTIPLIER 1.2f

Bus createBus(Vector2 position) {
    Bus bus;
    bus.position = position;
    bus.active = 1;
    bus.speed = 150.0f * BUS_SPEED_MULTIPLIER;
    bus.scale = 0.45f;

    bus.spriteLoaded = 0;
    // Tentar carregar texture (busR.png é o disponível)
    bus.texture = LoadTexture("assets/img/busR.png");
    if (bus.texture.id != 0) {
        bus.spriteLoaded = 1;
    }

    // Hitbox
    bus.hitbox = (Rectangle){
        bus.position.x - BUS_WIDTH / 2,
        bus.position.y - BUS_HEIGHT / 2,
        BUS_WIDTH,
        BUS_HEIGHT
    };

    return bus;
}


void updateBus(Bus *bus, float scrollSpeed, float deltaTime) {
    if (!bus->active) return;

    // Mover com scroll + velocidade própria
    bus->hitbox.x = bus->position.x - scaledWidth / 2;

    bus->hitbox.y =
        bus->position.y - scaledHeight;

    bus->hitbox.width = scaledWidth;

    bus->hitbox.height = scaledHeight;

    // Atualizar hitbox
    bus->hitbox.x = bus->position.x - BUS_WIDTH / 2;
    

    // Deativar se sair da tela
    if (bus->position.x < -BUS_WIDTH) {
        bus->active = 0;
    }
}

void drawBus(Bus bus) {
    if (!bus.active) return;

    const float BUS_TARGET_WIDTH = 320.0f;
    const float BUS_TARGET_HEIGHT = 140.0f;

    float scaledWidth = BUS_TARGET_WIDTH;
    float scaledHeight = BUS_TARGET_HEIGHT;

    // Posicionar alinhado ao chão
    float groundY = 520.0f;

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
