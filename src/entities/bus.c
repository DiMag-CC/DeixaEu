#include "bus.h"

#define BUS_SPEED_MULTIPLIER 1.5f

// ========== CRIAR ÔNIBUS ==========
Bus createBus(Vector2 position) {
    Bus bus;
    bus.position = position;
    bus.active = 1;
    bus.speed = 150.0f * BUS_SPEED_MULTIPLIER;

    bus.spriteLoaded = 0;
    bus.texture = LoadTexture("assets/img/bus.png");
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

// ========== ATUALIZAR ÔNIBUS ==========
void updateBus(Bus *bus, float scrollSpeed, float deltaTime) {
    if (!bus->active) return;

    // Mover com scroll + velocidade própria
    bus->position.x -= (scrollSpeed + bus->speed) * deltaTime;

    // Atualizar hitbox
    bus->hitbox.x = bus->position.x - BUS_WIDTH / 2;
    bus->hitbox.y = bus->position.y - BUS_HEIGHT / 2;

    // Deativar se sair da tela
    if (bus->position.x < -BUS_WIDTH) {
        bus->active = 0;
    }
}

// ========== DESENHAR ÔNIBUS ==========
void drawBus(Bus bus) {
    if (!bus.active) return;

    if (bus.spriteLoaded) {
        DrawTextureEx(bus.texture,
                     (Vector2){ bus.position.x - BUS_WIDTH / 2, bus.position.y - BUS_HEIGHT / 2 },
                     0, 1.0f, WHITE);
    } else {
        // Placeholder: retângulo amarelo (ônibus)
        DrawRectangleRec(bus.hitbox, YELLOW);
        DrawRectangleLinesEx(bus.hitbox, 2, BLACK);

        // Janelas
        DrawRectangle(bus.hitbox.x + 10, bus.hitbox.y + 5, 10, 8, SKYBLUE);
        DrawRectangle(bus.hitbox.x + 25, bus.hitbox.y + 5, 10, 8, SKYBLUE);
        DrawRectangle(bus.hitbox.x + 40, bus.hitbox.y + 5, 10, 8, SKYBLUE);

        // Rodas
        DrawCircle(bus.hitbox.x + 15, bus.hitbox.y + BUS_HEIGHT - 3, 3, BLACK);
        DrawCircle(bus.hitbox.x + BUS_WIDTH - 15, bus.hitbox.y + BUS_HEIGHT - 3, 3, BLACK);
    }

    // DrawRectangleLinesEx(bus.hitbox, 1, RED);
}

// ========== DESCARREGAR RECURSOS ==========
void unloadBusResources(Bus *bus) {
    if (bus->spriteLoaded) {
        UnloadTexture(bus->texture);
        bus->spriteLoaded = 0;
    }
}
