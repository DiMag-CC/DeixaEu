#include "bus.h"

#define BUS_SPEED_MULTIPLIER 1.5f

// ========== CRIAR ÔNIBUS ==========
Bus createBus(Vector2 position) {
    Bus bus;
    bus.position = position;
    bus.active = 1;
    bus.speed = 150.0f * BUS_SPEED_MULTIPLIER;
    bus.scale = 1.0f;

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

    float scaledWidth = BUS_WIDTH * bus.scale;
    float scaledHeight = BUS_HEIGHT * bus.scale;
    Vector2 drawPos = { bus.position.x - scaledWidth / 2, bus.position.y - scaledHeight / 2 };

    if (bus.spriteLoaded) {
        DrawTextureEx(bus.texture, drawPos, 0, bus.scale, WHITE);
    } else {
        // Placeholder: retângulo amarelo (ônibus) escalado
        DrawRectangle(drawPos.x, drawPos.y, scaledWidth, scaledHeight, YELLOW);
        DrawRectangleLinesEx((Rectangle){ drawPos.x, drawPos.y, scaledWidth, scaledHeight }, 2, BLACK);

        // Janelas escaladas
        float ww = 10 * bus.scale;
        float wh = 8 * bus.scale;
        DrawRectangle(drawPos.x + 10 * bus.scale, drawPos.y + 5 * bus.scale, ww, wh, SKYBLUE);
        DrawRectangle(drawPos.x + 25 * bus.scale, drawPos.y + 5 * bus.scale, ww, wh, SKYBLUE);
        DrawRectangle(drawPos.x + 40 * bus.scale, drawPos.y + 5 * bus.scale, ww, wh, SKYBLUE);

        // Rodas escaladas
        float wheelRadius = 3.0f * bus.scale;
        DrawCircle(drawPos.x + 15 * bus.scale, drawPos.y + scaledHeight - wheelRadius, wheelRadius, BLACK);
        DrawCircle(drawPos.x + scaledWidth - 15 * bus.scale, drawPos.y + scaledHeight - wheelRadius, wheelRadius, BLACK);
    }
}

// ========== DESCARREGAR RECURSOS ==========
void unloadBusResources(Bus *bus) {
    if (bus->spriteLoaded) {
        UnloadTexture(bus->texture);
        bus->spriteLoaded = 0;
    }
}
