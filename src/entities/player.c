#include "player.h"
#include "../gfx/animation.h"
#include "../utils/gameConstants.h"
#include <stdlib.h>
#include <math.h>

#define GRAVITY 600.0f
#define JUMP_FORCE 400.0f
#define FRICTION 0.95f
#define PLAYER_MAX_SPEED 400.0f
#define KNOCKBACK_DURATION 0.5f
#define TEXTURE_VALID(tex) ((tex).id > 0)

Player createPlayer(Vector2 startPos, float startSpeed, int lives) {
    Player player = {0};

    player.position = startPos;
    player.velocity = (Vector2){ 0, 0 };
    player.acceleration = (Vector2){ 0, 0 };

    player.speed = startSpeed;
    player.maxSpeed = PLAYER_MAX_SPEED;

    // Escala GLOBAL do player
    player.scale = 0.12f;

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

    player.direction = 'R';

    player.on_bike = 1;

    player.spritesLoaded = 0;

    player.spriteStandingR =
        LoadTexture("assets/img/CharacterStandingR.png");

    player.spriteStandingL =
        LoadTexture("assets/img/CharacterStandingL.png");

    player.spriteMovingR =
        LoadTexture("assets/img/CharacterMovingR1.png");

    player.spriteMovingL =
        LoadTexture("assets/img/CharacterMovingL1.png");

    player.spriteBikeStandingR =
        LoadTexture("assets/img/CharacterBikeStandingR.png");

    player.spriteBikeStandingL =
        LoadTexture("assets/img/CharacterBikeStandingL.png");

    player.spriteBikeMovingR =
        LoadTexture("assets/img/CharacterBikeMovingR.png");

    player.spriteBikeMovingL =
        LoadTexture("assets/img/CharacterBikeMovingL.png");

    SetTextureFilter(
        player.spriteStandingR,
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteStandingL,
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteMovingR,
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteMovingL,
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteBikeStandingR,
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteBikeStandingL,
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteBikeMovingR,
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteBikeMovingL,
        TEXTURE_FILTER_POINT
    );

    if (
    TEXTURE_VALID(player.spriteStandingR) &&
    TEXTURE_VALID(player.spriteStandingL) &&
    TEXTURE_VALID(player.spriteMovingR) &&
    TEXTURE_VALID(player.spriteMovingL) &&
    TEXTURE_VALID(player.spriteBikeStandingR) &&
    TEXTURE_VALID(player.spriteBikeStandingL) &&
    TEXTURE_VALID(player.spriteBikeMovingR) &&
    TEXTURE_VALID(player.spriteBikeMovingL)
    ) {
        player.spritesLoaded = 1;
    }

    player.width =
        player.spriteBikeMovingR.width * player.scale;

    player.height =
        player.spriteBikeMovingR.height * player.scale;

    player.hitbox = (Rectangle){
        player.position.x,
        player.position.y,
        player.width,
        player.height
    };

    return player;
}

void updatePlayer(Player *player, float deltaTime) {

    if (player->lives <= 0) {
        player->state = PLAYER_STATE_DEAD;
        return;
    }

    float moveInput = 0.0f;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        moveInput = -1.0f;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        moveInput = 1.0f;
    }

    float accelAmount = 800.0f;

    player->acceleration.x = moveInput * accelAmount;
    player->velocity.x += player->acceleration.x * deltaTime;

    if (moveInput == 0.0f) {
        player->velocity.x *= FRICTION;
    }

    if (player->slowEffectTimer > 0.0f) {

        player->slowEffectTimer -= deltaTime;

        if (player->slowEffectTimer <= 0.0f) {

            player->slowEffectTimer = 0.0f;
            player->speedMultiplier = 1.0f;
        }
    }

    player->velocity.x *= player->speedMultiplier;

    if (player->velocity.x > player->maxSpeed) {
        player->velocity.x = player->maxSpeed;
    }

    if (player->velocity.x < -player->maxSpeed) {
        player->velocity.x = -player->maxSpeed;
    }

    if (
        player->isGrounded &&
        (
            IsKeyPressed(KEY_SPACE) ||
            IsKeyPressed(KEY_W) ||
            IsKeyPressed(KEY_UP)
        )
    ) {

        player->velocity.y = -player->jumpPower;

        player->isJumping = 1;
        player->isGrounded = 0;

        player->state = PLAYER_STATE_JUMPING;
    }

    player->acceleration.y = GRAVITY;
    player->velocity.y += player->acceleration.y * deltaTime;

    player->position.x += player->velocity.x * deltaTime;
    player->position.y += player->velocity.y * deltaTime;

    float groundY =
        GLOBAL_GROUND_LEVEL - player->height;

    if (player->position.y >= groundY) {

        player->position.y = groundY;

        player->velocity.y = 0.0f;

        player->isGrounded = 1;
        player->grounded = 1;
        player->isJumping = 0;

    } else {

        player->isGrounded = 0;
        player->grounded = 0;
    }

    float minX = player->width * 0.5f;
    float maxX = GetScreenWidth() - player->width * 0.5f;

    if (player->position.x < minX) {

        player->position.x = minX;
        player->velocity.x = 0.0f;
    }

    if (player->position.x > maxX) {

        player->position.x = maxX;
        player->velocity.x = 0.0f;
    }

    player->hitbox.x =
        player->position.x - player->width * 0.35f;

    player->hitbox.y =
        player->position.y - player->height + 20.0f;

    player->hitbox.width =
        player->width * 0.7f;

    player->hitbox.height =
        player->height - 20.0f;

    if (player->knockbackTimer > 0.0f) {

        player->knockbackTimer -= deltaTime;

        if (player->knockbackTimer <= 0.0f) {

            player->knockbackTimer = 0.0f;
            player->knockbackSpeed = 0.0f;
        }
    }

    if (player->hasUmbrella) {

        player->umbrellaTimer -= deltaTime;

        if (player->umbrellaTimer <= 0.0f) {

            player->umbrellaTimer = 0.0f;
            player->hasUmbrella = 0;
        }
    }

    if (player->knockbackTimer > 0.0f) {

        player->state = PLAYER_STATE_HIT;

    } else if (!player->isGrounded) {

        if (player->velocity.y < 0.0f) {
            player->state = PLAYER_STATE_JUMPING;
        } else {
            player->state = PLAYER_STATE_FALLING;
        }

    } else {

        if (fabs(player->velocity.x) > 10.0f) {
            player->state = PLAYER_STATE_RUNNING;
        } else {
            player->state = PLAYER_STATE_IDLE;
        }
    }

    if (player->velocity.x > 0.1f) {
        player->direction = 'R';
    }

    if (player->velocity.x < -0.1f) {
        player->direction = 'L';
    }

    player->score += fabs(player->velocity.x) * deltaTime;
}

void drawPlayer(Player player) {

    Color drawColor = WHITE;

    if (player.state == PLAYER_STATE_HIT) {

        float blink =
            fmod(player.knockbackTimer * 20.0f, 1.0f);

        drawColor =
            (blink < 0.5f)
            ? WHITE
            : (Color){180,180,180,255};
    }

    Texture2D currentSprite = {0};

    if (player.on_bike) {

        if (fabs(player.velocity.x) > 10.0f) {

            currentSprite =
                (player.direction == 'R')
                ? player.spriteBikeMovingR
                : player.spriteBikeMovingL;

        } else {

            currentSprite =
                (player.direction == 'R')
                ? player.spriteBikeStandingR
                : player.spriteBikeStandingL;
        }

    } else {

        if (fabs(player.velocity.x) > 10.0f) {

            currentSprite =
                (player.direction == 'R')
                ? player.spriteMovingR
                : player.spriteMovingL;

        } else {

            currentSprite =
                (player.direction == 'R')
                ? player.spriteStandingR
                : player.spriteStandingL;
        }
    }

    if (
        player.spritesLoaded &&
        TEXTURE_VALID(currentSprite)
    ) {

        Rectangle sourceRect = {
            0,
            0,
            (float) currentSprite.width,
            (float) currentSprite.height
        };

        Rectangle destRect = {
            player.position.x - player.width * 0.5f,
            player.position.y - player.height + 25.0f,
            player.width,
            player.height
        };

        DrawTexturePro(
            currentSprite,
            sourceRect,
            destRect,
            (Vector2){0,0},
            0.0f,
            drawColor
        );

    } else {

        DrawRectangleRec(
            player.hitbox,
            RED
        );
    }

#ifdef DEBUG_MODE

    DrawRectangleLinesEx(
        player.hitbox,
        2,
        GREEN
    );

#endif

    if (player.hasUmbrella) {

        float umbrellaX =
            player.position.x +
            player.width * 0.5f;

        float umbrellaY =
            player.position.y - 20.0f;

        DrawCircle(
            umbrellaX,
            umbrellaY,
            20.0f,
            (Color){100,150,255,180}
        );

        DrawLine(
            umbrellaX,
            umbrellaY,
            umbrellaX,
            umbrellaY + 35,
            DARKGRAY
        );
    }
}


void damagePlayer(Player *player, float knockback) {
    if (player->lives <= 0) return;

    player->lives--;
    player->knockbackSpeed = knockback;
    player->knockbackTimer = KNOCKBACK_DURATION;

    if (player->lives <= 0) {
        player->state = PLAYER_STATE_DEAD;
    }
}

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
