#include "player.h"
#include "../gfx/animation.h"
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

    player.slowEffectTimer = 0.0f;
    player.slowEffectDuration = 0.0f;
    player.speedMultiplier = 1.0f;

    player.isClimbing = 0;
    player.movementControlledExternally = 0;
    player.grounded = 1;

    // Direção inicial
    player.direction = 'R';
    player.on_bike = 0;

    // Hitbox
    player.hitbox = (Rectangle){
        player.position.x - player.width / 2,
        player.position.y - player.height,
        player.width,
        player.height
    };

    // Carregar animações do player
    player.anim_standing = animation_load_directional("CharacterStanding", 8.0f, 0);
    player.anim_moving = animation_load_directional("characterMoving", 12.0f, 1);
    player.anim_bike_standing = animation_load_directional("CharacterBikeStanding", 8.0f, 0);
    player.anim_bike_moving = animation_load_directional("CharacterBikeMoving", 15.0f, 1);

    // Carregar textura (placeholder se falhar - deprecated)
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

    // ===== ATUALIZAR DEBUFF DE LENTIDÃO =====
    if (player->slowEffectTimer > 0.0f) {
        player->slowEffectTimer -= deltaTime;
        if (player->slowEffectTimer <= 0.0f) {
            player->speedMultiplier = 1.0f;
            player->slowEffectTimer = 0.0f;
        }
    }

    // ===== APLICAR ATRITO =====
    if (moveInput == 0.0f) {
        player->velocity.x *= FRICTION;
    }

    // ===== APLICAR MULTIPLICADOR DE VELOCIDADE (DEBUFF) =====
    player->velocity.x *= player->speedMultiplier;

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

    // ===== ATUALIZAR ANIMAÇÕES =====
    // Atualizar todas as sequências de animação
    directional_animation_update(&player->anim_standing, deltaTime);
    directional_animation_update(&player->anim_moving, deltaTime);
    directional_animation_update(&player->anim_bike_standing, deltaTime);
    directional_animation_update(&player->anim_bike_moving, deltaTime);

    // Atualizar direção baseado em velocidade
    if (player->velocity.x > 0.1f) {
        player->direction = 'R';
    } else if (player->velocity.x < -0.1f) {
        player->direction = 'L';
    }
    // Se velocidade está próxima de 0, manter direção anterior

    // Atualizar direção em todas as animações
    player->anim_standing.direction = player->direction;
    player->anim_moving.direction = player->direction;
    player->anim_bike_standing.direction = player->direction;
    player->anim_bike_moving.direction = player->direction;

    // ===== INCREMENTAR SCORE POR DISTÂNCIA =====
    player->score += player->speed * deltaTime;
}

// ========== DESENHAR JOGADOR ==========
void drawPlayer(Player player) {
    // Renderizar com animação apropriada baseado no estado
    DirectionalAnimationSet* current_anim = NULL;

    if (player.on_bike) {
        // Na bicicleta
        if (fabs(player.velocity.x) > 10.0f) {
            // Pedalando (movimento)
            current_anim = (DirectionalAnimationSet*)&player.anim_bike_moving;
        } else {
            // Parado na bike
            current_anim = (DirectionalAnimationSet*)&player.anim_bike_standing;
        }
    } else {
        // A pé
        if (fabs(player.velocity.x) > 10.0f) {
            // Correndo
            current_anim = (DirectionalAnimationSet*)&player.anim_moving;
        } else {
            // Parado/Idle
            current_anim = (DirectionalAnimationSet*)&player.anim_standing;
        }
    }

    // Renderizar animação se disponível
    if (current_anim && current_anim->left.frame_count > 0) {
        directional_animation_render(current_anim, player.position.x, player.position.y, player.scale, WHITE);
    } else if (player.spriteLoaded) {
        // Fallback para textura única se animação não carregar
        DrawTextureEx(player.playerTexture,
                     (Vector2){ player.position.x - (player.width * player.scale) / 2,
                               player.position.y - (player.height * player.scale) },
                     0, player.scale, WHITE);
    } else {
        // Placeholder: retângulo colorido
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

// ========== APLICAR DESACELERAÇÃO (DEBUFF TEMPORAL) ==========
void applySlowDown(Player *player, float amount, float duration) {
    // amount = redução de velocidade (ex: 50.0 = 50%)
    // duration = quanto tempo dura o efeito
    player->slowEffectTimer = duration;
    player->slowEffectDuration = duration;

    // Calcular multiplicador (ex: 50 = 0.5, aplicar 50% de redução)
    float multiplier = (100.0f - amount) / 100.0f;
    if (multiplier < 0.1f) multiplier = 0.1f;  // Mínimo 10% de velocidade

    player->speedMultiplier = multiplier;
}

// ========== DESCARREGAR RECURSOS DO JOGADOR ==========
void unloadPlayerResources(Player *player) {
    // Descarregar animações
    directional_animation_unload(&player->anim_standing);
    directional_animation_unload(&player->anim_moving);
    directional_animation_unload(&player->anim_bike_standing);
    directional_animation_unload(&player->anim_bike_moving);

    // Descarregar textura (deprecated)
    if (player->spriteLoaded) {
        UnloadTexture(player->playerTexture);
        player->spriteLoaded = 0;
    }
}
