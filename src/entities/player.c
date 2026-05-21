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

    // Carregar sprites únicos diretos
    player.spritesLoaded = 0;
    player.spriteStandingR = LoadTexture("assets/img/CharacterStandingR.png");
    player.spriteStandingL = LoadTexture("assets/img/CharacterStandingL.png");
    player.spriteMovingR = LoadTexture("assets/img/CharacterMovingR1.png");
    player.spriteMovingL = LoadTexture("assets/img/CharacterMovingL1.png");
    player.spriteBikeStandingR = LoadTexture("assets/img/CharacterBikeStandingR.png");
    player.spriteBikeStandingL = LoadTexture("assets/img/CharacterBikeStandingL.png");
    player.spriteBikeMovingR = LoadTexture("assets/img/CharacterBikeMovingR.png");
    player.spriteBikeMovingL = LoadTexture("assets/img/CharacterBikeMovingL.png");

    if (player.spriteStandingR.id != 0 && player.spriteStandingL.id != 0 &&
        player.spriteMovingR.id != 0 && player.spriteMovingL.id != 0 &&
        player.spriteBikeStandingR.id != 0 && player.spriteBikeStandingL.id != 0 &&
        player.spriteBikeMovingR.id != 0 && player.spriteBikeMovingL.id != 0) {
        player.spritesLoaded = 1;
    }

    // Tentar carregar animações multi-frame (pode falhar se arquivos não existem)
    player.anim_standing = animation_load_directional("CharacterStanding", 8.0f, 0);
    player.anim_moving = animation_load_directional("CharacterMoving", 12.0f, 1);
    player.anim_bike_standing = animation_load_directional("CharacterBikeStanding", 8.0f, 0);
    player.anim_bike_moving = animation_load_directional("CharacterBikeMoving", 15.0f, 1);

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

    // ===== ATUALIZAR STATE MACHINE FINAL =====
    // Determinar estado final baseado em situações especiais
    if (player->knockbackTimer > 0) {
        // Knockback visual override
        player->state = PLAYER_STATE_HIT;
    } else if (player->hasUmbrella > 0 && player->state != PLAYER_STATE_DEAD) {
        // Proteção de guarda-chuva visual
        if (fabs(player->velocity.x) > 10.0f) {
            player->state = PLAYER_STATE_RUNNING;  // Mantém running mas com visual diferente
        } else {
            player->state = PLAYER_STATE_IDLE;
        }
    } else if (player->state != PLAYER_STATE_JUMPING &&
               player->state != PLAYER_STATE_FALLING &&
               player->state != PLAYER_STATE_DEAD) {
        // Estado normal: IDLE ou RUNNING baseado em velocidade
        if (fabs(player->velocity.x) > 10.0f) {
            player->state = PLAYER_STATE_RUNNING;
        } else {
            player->state = PLAYER_STATE_IDLE;
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
    // Determinar cor baseada em estado
    Color drawColor = WHITE;
    if (player.state == PLAYER_STATE_HIT) {
        float blinkPhase = fmod(player.knockbackTimer * 20.0f, 1.0f);
        drawColor = (blinkPhase < 0.5f) ? WHITE : (Color){200, 200, 200, 255};
    } else if (player.state == PLAYER_STATE_UMBRELLA_BUFF) {
        drawColor = (Color){150, 200, 255, 255};
    } else if (player.state == PLAYER_STATE_DEAD) {
        drawColor = (Color){255, 100, 100, 255};
    }

    // Selecionar sprite apropriado baseado em estado
    Texture2D spriteToRender = {0};

    if (player.spritesLoaded) {
        if (player.on_bike) {
            if (fabs(player.velocity.x) > 10.0f) {
                spriteToRender = (player.direction == 'R') ? player.spriteBikeMovingR : player.spriteBikeMovingL;
            } else {
                spriteToRender = (player.direction == 'R') ? player.spriteBikeStandingR : player.spriteBikeStandingL;
            }
        } else {
            if (fabs(player.velocity.x) > 10.0f) {
                spriteToRender = (player.direction == 'R') ? player.spriteMovingR : player.spriteMovingL;
            } else {
                spriteToRender = (player.direction == 'R') ? player.spriteStandingR : player.spriteStandingL;
            }
        }

        if (spriteToRender.id != 0) {
            // Escalar sprite para altura máxima de 80px
            const float PLAYER_HEIGHT_PIXELS = 80.0f;
            float scaleY = PLAYER_HEIGHT_PIXELS / spriteToRender.height;
            float scaleX = scaleY;  // Manter aspect ratio

            float scaledWidth = spriteToRender.width * scaleX;
            float scaledHeight = spriteToRender.height * scaleY;

            Rectangle source = {0, 0, (float)spriteToRender.width, (float)spriteToRender.height};
            Rectangle dest = {
                player.position.x - scaledWidth / 2,
                player.position.y - scaledHeight,
                scaledWidth,
                scaledHeight
            };

            DrawTexturePro(spriteToRender, source, dest, (Vector2){0, 0}, 0, drawColor);
        }
    }

    // Fallback se sprites não carregarem
    if (!player.spritesLoaded || spriteToRender.id == 0) {
        Rectangle scaledHitbox = player.hitbox;
        scaledHitbox.width = 80.0f;  // Largura proporcional para 80px de altura
        scaledHitbox.height = 80.0f;
        scaledHitbox.x = player.position.x - scaledHitbox.width / 2;
        scaledHitbox.y = player.position.y - scaledHitbox.height;
        DrawRectangleRec(scaledHitbox, drawColor);
        DrawRectangleLinesEx(scaledHitbox, 2, BLACK);
    }

    // ===== RENDERIZAR GUARDA-CHUVA SE ATIVO =====
    if (player.hasUmbrella > 0) {
        // Desenhar guarda-chuva acima do player
        float umbrellaX = player.position.x;
        float umbrellaY = player.position.y - (player.height * 0.6f);
        float umbrellaSize = 30.0f;

        // Semicírculo (guarda-chuva)
        DrawCircle((int)umbrellaX, (int)umbrellaY, umbrellaSize, (Color){100, 150, 255, 200});
        DrawCircleLines((int)umbrellaX, (int)umbrellaY, umbrellaSize, (Color){50, 100, 200, 200});

        // Haste
        DrawLine((int)umbrellaX, (int)umbrellaY + 5, (int)umbrellaX, (int)(player.position.y - player.height * 0.2f),
                 (Color){100, 100, 100, 200});
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
    // Descarregar sprites únicos
    if (player->spritesLoaded) {
        if (player->spriteStandingR.id != 0) UnloadTexture(player->spriteStandingR);
        if (player->spriteStandingL.id != 0) UnloadTexture(player->spriteStandingL);
        if (player->spriteMovingR.id != 0) UnloadTexture(player->spriteMovingR);
        if (player->spriteMovingL.id != 0) UnloadTexture(player->spriteMovingL);
        if (player->spriteBikeStandingR.id != 0) UnloadTexture(player->spriteBikeStandingR);
        if (player->spriteBikeStandingL.id != 0) UnloadTexture(player->spriteBikeStandingL);
        if (player->spriteBikeMovingR.id != 0) UnloadTexture(player->spriteBikeMovingR);
        if (player->spriteBikeMovingL.id != 0) UnloadTexture(player->spriteBikeMovingL);
        player->spritesLoaded = 0;
    }

    // Descarregar animações
    directional_animation_unload(&player->anim_standing);
    directional_animation_unload(&player->anim_moving);
    directional_animation_unload(&player->anim_bike_standing);
    directional_animation_unload(&player->anim_bike_moving);
}
