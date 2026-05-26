#include "bus.h"
#include "../utils/gameConstants.h"

#define BUS_SPEED_MULTIPLIER 1.5f
#define BUS_TARGET_WIDTH 320.0f
#define BUS_TARGET_HEIGHT 140.0f
#define BUS_TOP_SURFACE_HEIGHT 30.0f  

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

    // ===== HITBOX PRINCIPAL =====
    bus.hitbox = (Rectangle){
        bus.position.x - BUS_TARGET_WIDTH / 2,
        bus.position.y - BUS_TARGET_HEIGHT,
        BUS_TARGET_WIDTH,
        BUS_TARGET_HEIGHT
    };

    // ===== HITBOX DO TOPO (Onde player pode ficar em pé) =====
    // Apenas os 30px de cima do ônibus
    bus.topHitbox = (Rectangle){
        bus.position.x - BUS_TARGET_WIDTH / 2,
        bus.position.y - BUS_TARGET_HEIGHT,  // Topo do ônibus
        BUS_TARGET_WIDTH,
        BUS_TOP_SURFACE_HEIGHT  // 30px de altura
    };

    // ===== NOVOS CAMPOS =====
    bus.playerOnTop = 0;
    bus.playerStandingTime = 0.0f;

    return bus;
}


void updateBus(Bus *bus, float scrollSpeed, float deltaTime) {

    if (!bus->active) return;

    // Movimento lateral
    bus->position.x -=
        (scrollSpeed + bus->speed) * deltaTime;

    // Ground global
    float groundY = GLOBAL_GROUND_LEVEL;

    // ===== ATUALIZAR HITBOX PRINCIPAL =====
    bus->hitbox.x =
        bus->position.x - BUS_TARGET_WIDTH / 2;

    bus->hitbox.y =
        groundY - BUS_TARGET_HEIGHT;

    bus->hitbox.width =
        BUS_TARGET_WIDTH;

    bus->hitbox.height =
        BUS_TARGET_HEIGHT;

    // ===== ATUALIZAR HITBOX DO TOPO =====
    bus->topHitbox.x =
        bus->position.x - BUS_TARGET_WIDTH / 2;

    bus->topHitbox.y =
        groundY - BUS_TARGET_HEIGHT;  // Topo do ônibus

    bus->topHitbox.width =
        BUS_TARGET_WIDTH;

    bus->topHitbox.height =
        BUS_TOP_SURFACE_HEIGHT;

    // ===== ATUALIZAR TEMPO EM PÉ =====
    if (bus->playerOnTop) {
        bus->playerStandingTime += deltaTime;
    } else {
        bus->playerStandingTime = 0.0f;
    }

    // Desativar fora da tela
    if (bus->position.x < -BUS_TARGET_WIDTH) {
        bus->active = 0;
    }
}

void drawBus(Bus bus) {
    if (!bus.active) return;

    const float BUS_DRAW_WIDTH = 420.0f;
    const float BUS_DRAW_HEIGHT = 320.0f;

    // Posicionar alinhado ao chão
    float groundY = GLOBAL_GROUND_LEVEL + 140.0f;

    Rectangle dest = {
        bus.position.x - BUS_DRAW_WIDTH / 2,
        groundY - BUS_DRAW_HEIGHT,
        BUS_DRAW_WIDTH,
        BUS_DRAW_HEIGHT
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

    // DEBUG: Mostrar hitbox do topo (descomente para debug)
    // DrawRectangleLinesEx(bus.topHitbox, 1, LIME);
}

void unloadBusResources(Bus *bus) {
    if (bus->spriteLoaded) {
        UnloadTexture(bus->texture);
        bus->spriteLoaded = 0;
    }
}