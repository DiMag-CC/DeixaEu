#include "player.h"
#include <stdlib.h>
#include <math.h>

#define GRAVITY 600.0f
#define JUMP_FORCE 400.0f
#define FRICTION 0.95f
#define PLAYER_MAX_SPEED 400.0f
#define KNOCKBACK_DURATION 0.5f

// ========== CRIAR JOGADOR ==========
Player createPlayer(Vector2 startPos, float startSpeed, int lives) {
    Player player;
    player.position = startPos;
    player.velocity = (Vector2){ 0, 0 };
    player.acceleration = (Vector2){ 0, 0 };

    player.speed = startSpeed;
    player.maxSpeed = PLAYER_MAX_SPEED;
    player.width = PLAYER_WIDTH;
    player.height = PLAYER_HEIGHT;
    player.scale = 1.2f;

    player.lives = lives;
    player.score = 0.0f;

    player.isGrounded = 1;
    player.isJumping = 0;
    player.jumpPower = JUMP_FORCE;
    player.fallSpeed = 0.0f;

    player.state = PLAYER_STATE_IDLE;
    player.animationTimer = 0.0f;
    player.animationFrame = 0;

    player.hasUmbrella = 0;
    player.umbrellaTimer = 0.0f;

    player.knockbackSpeed = 0.0f;
    player.knockbackTimer = 0.0f;

    player.isClimbing = 0;
    player.movementControlledExternally = 0;
    player.grounded = 1;

    // Hitbox
    player.hitbox = (Rectangle){
        player.position.x - player.width / 2,
        player.position.y - player.height,
        player.width,
        player.height
    };

    // Carregar textura (placeholder se falhar)
    player.spriteLoaded = 0;
    player.playerTexture = LoadTexture("assets/img/player.png");
    if (player.playerTexture.id != 0) {
        player.spriteLoaded = 1;
    }

    return player;
}

// ========== ATUALIZAR JOGADOR ==========
void updatePlayer(Player *player, float deltaTime) {
    if (player->lives <= 0) {
        player->state = PLAYER_STATE_DEAD;
        return;
    }

    // ===== ENTRADA DO JOGADOR =====
    float moveInput = 0.0f;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        moveInput = -1.0f;
    } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        moveInput = 1.0f;
    }

    // ===== APLICAR ACELERAÇÃO LATERAL =====
    float accelAmount = 800.0f;
    player->acceleration.x = moveInput * accelAmount;
    player->velocity.x += player->acceleration.x * deltaTime;

    // ===== APLICAR ATRITO =====
    if (moveInput == 0.0f) {
        player->velocity.x *= FRICTION;
    }

    // ===== LIMITAR VELOCIDADE =====
    if (player->velocity.x > player->maxSpeed) {
        player->velocity.x = player->maxSpeed;
    } else if (player->velocity.x < -player->maxSpeed) {
        player->velocity.x = -player->maxSpeed;
    }

    // ===== PULO =====
    if (player->isGrounded && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))) {
        player->velocity.y = -player->jumpPower;
        player->isJumping = 1;
        player->isGrounded = 0;
        player->state = PLAYER_STATE_JUMPING;
    }

    // ===== GRAVIDADE =====
    player->acceleration.y = GRAVITY;
    player->velocity.y += player->acceleration.y * deltaTime;

    // ===== MOVIMENTO =====
    player->position.x += player->velocity.x * deltaTime;
    player->position.y += player->velocity.y * deltaTime;

    // ===== COLISÃO COM O CHÃO =====
    if (player->position.y >= GROUND_LEVEL) {
        player->position.y = GROUND_LEVEL;
        player->velocity.y = 0;
        player->isGrounded = 1;
        player->grounded = 1;
        player->isJumping = 0;
        player->state = PLAYER_STATE_RUNNING;
        player->fallSpeed = 0.0f;
    } else {
        player->isGrounded = 0;
        player->grounded = 0;
        if (player->state != PLAYER_STATE_JUMPING) {
            player->state = PLAYER_STATE_FALLING;
        }
    }

    // ===== LIMITES HORIZONTAIS =====
    int screenWidth = GetScreenWidth();
    if (player->position.x < PLAYER_WIDTH / 2) {
        player->position.x = PLAYER_WIDTH / 2;
        player->velocity.x = 0;
    } else if (player->position.x > screenWidth - PLAYER_WIDTH / 2) {
        player->position.x = screenWidth - PLAYER_WIDTH / 2;
        player->velocity.x = 0;
    }

    // ===== ATUALIZAR HITBOX =====
    player->hitbox.x = player->position.x - player->width / 2;
    player->hitbox.y = player->position.y - player->height;
    player->hitbox.width = player->width;
    player->hitbox.height = player->height;

    // ===== KNOCKBACK =====
    if (player->knockbackTimer > 0) {
        player->knockbackTimer -= deltaTime;
        if (player->knockbackTimer <= 0) {
            player->knockbackSpeed = 0.0f;
        }
    }

    // ===== GUARDA-CHUVA =====
    if (player->hasUmbrella > 0) {
        player->umbrellaTimer -= deltaTime;
        if (player->umbrellaTimer <= 0) {
            player->hasUmbrella = 0;
            player->umbrellaTimer = 0.0f;
        }
    }

    // ===== ANIMAÇÃO =====
    player->animationTimer += deltaTime;
    if (player->animationTimer >= 0.1f) {
        player->animationTimer = 0.0f;
        player->animationFrame = (player->animationFrame + 1) % 4;
    }

    // ===== INCREMENTAR SCORE POR DISTÂNCIA =====
    player->score += player->speed * deltaTime;
}

// ========== DESENHAR JOGADOR ==========
void drawPlayer(Player player) {
    // Se tiver sprite, desenhar com textura
    if (player.spriteLoaded) {
        DrawTextureEx(player.playerTexture,
                     (Vector2){ player.position.x - (player.width * player.scale) / 2,
                               player.position.y - (player.height * player.scale) },
                     0, player.scale, WHITE);
    } else {
        // Placeholder: retângulo colorido escalado
        Rectangle scaledHitbox = player.hitbox;
        scaledHitbox.width *= player.scale;
        scaledHitbox.height *= player.scale;
        DrawRectangleRec(scaledHitbox, ORANGE);
        DrawRectangleLinesEx(scaledHitbox, 2, BLACK);
    }
}

// ========== DANIFICAR JOGADOR ==========
void damagePlayer(Player *player, float knockback) {
    if (player->lives <= 0) return;

    player->lives--;
    player->knockbackSpeed = knockback;
    player->knockbackTimer = KNOCKBACK_DURATION;

    if (player->lives <= 0) {
        player->state = PLAYER_STATE_DEAD;
    }
}

// ========== CURAR JOGADOR ==========
void healPlayer(Player *player) {
    if (player->lives < 3) {
        player->lives++;
    }
}

// ========== ADICIONAR ESCUDO DE GUARDA-CHUVA ==========
void addUmbrellaShield(Player *player, float duration) {
    player->hasUmbrella = 1;
    player->umbrellaTimer = duration;
}

// ========== APLICAR DESACELERAÇÃO ==========
void applySlowDown(Player *player, float amount, float duration) {
    (void)duration;
    if (player->speed > amount) {
        player->speed -= amount;
    } else {
        player->speed = 0.0f;
    }
}

// ========== DESCARREGAR RECURSOS DO JOGADOR ==========
void unloadPlayerResources(Player *player) {
    if (player->spriteLoaded) {
        UnloadTexture(player->playerTexture);
        player->spriteLoaded = 0;
    }
}
