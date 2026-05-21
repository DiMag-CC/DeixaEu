#include "bus.h"

#define BUS_SPEED_MULTIPLIER 1.5f


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

    // Escalar para 150px largura x 60px altura
    const float BUS_TARGET_WIDTH = 150.0f;
    const float BUS_TARGET_HEIGHT = 60.0f;

    float scaleX = BUS_TARGET_WIDTH / bus.texture.width;
    float scaleY = BUS_TARGET_HEIGHT / bus.texture.height;

    float scaledWidth = BUS_TARGET_WIDTH;
    float scaledHeight = BUS_TARGET_HEIGHT;

    // Alinhar ao chão (GROUND_Y = 380)
    float busGroundY = 380.0f - scaledHeight / 2;

    if (bus.spriteLoaded && bus.texture.id != 0) {
        Rectangle source = { 0, 0, (float)bus.texture.width, (float)bus.texture.height };
        Rectangle dest = {
            bus.position.x - scaledWidth / 2,
            busGroundY,
            scaledWidth,
            scaledHeight
        };
        DrawTexturePro(bus.texture, source, dest, (Vector2){0, 0}, 0, WHITE);
    } else {
        // Placeholder: retângulo amarelo (ônibus)
        float drawX = bus.position.x - scaledWidth / 2;
        float drawY = busGroundY;
        DrawRectangle((int)drawX, (int)drawY, (int)scaledWidth, (int)scaledHeight, ORANGE);
        DrawRectangleLinesEx((Rectangle){ drawX, drawY, scaledWidth, scaledHeight }, 2, BLACK);

        // Janelas
        DrawRectangle((int)(drawX + 15), (int)(drawY + 8), 15, 12, SKYBLUE);
        DrawRectangle((int)(drawX + 38), (int)(drawY + 8), 15, 12, SKYBLUE);
        DrawRectangle((int)(drawX + 61), (int)(drawY + 8), 15, 12, SKYBLUE);

        // Rodas
        DrawCircle((int)(drawX + 25), (int)(drawY + scaledHeight - 8), 6, BLACK);
        DrawCircle((int)(drawX + scaledWidth - 25), (int)(drawY + scaledHeight - 8), 6, BLACK);
    }
}

// ========== DESCARREGAR RECURSOS ==========
void unloadBusResources(Bus *bus) {
    if (bus->spriteLoaded) {
        UnloadTexture(bus->texture);
        bus->spriteLoaded = 0;
    }
}
