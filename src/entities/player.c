#include <raylib.h>
#include "player.h"

// #define PLAYER_WIDTH 40
// #define PLAYER_HEIGHT 60
// #define PLAYER_JUMP_SPEED 420.0f
// #define GRAVITY 980.0f
// #define GROUND_LEVEL 380.0f
// #define PLAYER_CROUCH_HEIGHT 35
// #define PLAYER_STANDING_HEIGHT PLAYER_HEIGHT

Player createPlayer(Vector2 initialPosition, float initialSpeed,  int initialLives) {
    Player player;
    player.position = initialPosition;
    player.speed = initialSpeed;
    player.lives = initialLives;
    player.score = 0;
    player.height = PLAYER_STANDING_HEIGHT;
    player.hitbox = (Rectangle){ initialPosition.x, initialPosition.y, PLAYER_WIDTH, PLAYER_STANDING_HEIGHT };
    player.grounded = true;

    player.hasUmbrella = 0;
    player.umbrellaTimer = 0.0f;
    player.isClimbing = false;
    player.movementControlledExternally = false;

    return player;

}

void updatePlayer(Player *player, float deltaTime) {
    player->hitbox.x = player->position.x;
    player->hitbox.y = player->position.y;
    player->hitbox.width = PLAYER_WIDTH;
    player->hitbox.height = player->height;

    if (player->lives <= 0) {
        player->velocity = (Vector2){0, 0};
        return;
    }

    if (player->isClimbing) {
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
            player->position.y -= 200 * deltaTime;
        }
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
            player->position.y += 200 * deltaTime;
        }
    } else {
        player->position.x += player->speed * deltaTime;

        if (!player->movementControlledExternally) {
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) { // Esquerda
                player->position.x -= 300 * deltaTime;
            }

            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { // Direita
                player->position.x += 300 * deltaTime;
            }
        }

        if (IsKeyPressed(KEY_SPACE) && player->grounded) { // Pular
            player->velocity.y = -PLAYER_JUMP_SPEED;
            player->grounded = false;
        }

        player->velocity.y += GRAVITY * deltaTime;

        player->position.y += player->velocity.y * deltaTime;

        if (player->position.y >= GROUND_LEVEL) { // Está no chão?
            player->position.y = GROUND_LEVEL;
            player->velocity.y = 0;
            player->grounded = true;
        }
    }

    if (IsKeyPressed(KEY_DOWN)) { // Abaixar
        player->height = PLAYER_CROUCH_HEIGHT;
    } else {
        player->height = PLAYER_STANDING_HEIGHT;
    }

    // Verificar colisões
}

void drawPlayer(Player player) {

    DrawRectangle(player.position.x, player.position.y, PLAYER_WIDTH, PLAYER_HEIGHT, BLACK);
    
}
