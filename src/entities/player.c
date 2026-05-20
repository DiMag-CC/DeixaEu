#include <raylib.h>
#include <stdio.h>
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
    player.grounded = true;

    player.hasUmbrella = 0;
    player.umbrellaTimer = 0.0f;

    return player;

}

void updatePlayer(Player *player, float deltaTime) {
    player->hitbox.x = player->position.x;
    player->hitbox.y = player->position.y;
    player->hitbox.width = PLAYER_WIDTH;
    player->hitbox.height = player->height;
    player->position.x += player->speed * deltaTime;

    if (IsKeyDown(KEY_A)) { // Esquerda
        player->position.x -= 300 * deltaTime;
    }

    if (IsKeyDown(KEY_D)) { // Direita
        player->position.x += 300 * deltaTime;
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

    if (IsKeyPressed(KEY_DOWN)) { // Abaixar
        player->height = PLAYER_CROUCH_HEIGHT;
    } else {
        player->height = PLAYER_STANDING_HEIGHT;
    }

    // Verificar colisões
}


void drawPlayer(Player player) {
    DrawRectangleRec(player.hitbox, (Color){0, 100, 200, 255});
    
    DrawRectangleLinesEx(player.hitbox, 2, BLACK);
    
    char livesStr[4];
    sprintf(livesStr, "%d", player.lives);
    DrawText(livesStr, 
             player.hitbox.x + player.hitbox.width/2 - 5,
             player.hitbox.y + player.hitbox.height/2 - 10,
             20, WHITE);
}
