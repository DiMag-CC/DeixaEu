#include "player.h"
#include "../gfx/animation.h"
#include "../utils/gameConstants.h"
#include <stdlib.h>
#include <math.h>

#define GRAVITY 550.0f
#define JUMP_FORCE 620.0f
#define FRICTION 0.95f
#define PLAYER_MAX_SPEED 450.0f
#define KNOCKBACK_DURATION 1.2f
#define TEXTURE_VALID(tex) ((tex).id > 0)
#define GRAVITY_FALLING_MULT 1.8f
#define AIR_ACCEL_MULT 0.6f
#define COYOTE_TIME 0.1f
#define JUMP_BUFFER_TIME 0.05f
#define MAX_JUMP_HOLD_TIME 0.15f

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

    player.canDoubleJump = 1;
    player.isGrounded = 1;
    player.isJumping = 0;
    player.isPerformingStunt = 0;
    player.jumpPower = JUMP_FORCE;
    player.fallSpeed = 0.0f;

    player.coyoteTimer = 0.0f;
    player.jumpBufferTimer = 0.0f;
    player.jumpHoldTime = 0.0f;
    player.airAccelerationMult = 1.0f;

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

    player.spriteBikeStuntR = 
        LoadTexture("assets/img/CharacterBikeStuntR.png");
        
    player.spriteBikeStuntL = 
        LoadTexture("assets/img/CharacterBikeStuntL.png");

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

    SetTextureFilter(
        player.spriteBikeStuntR, 
        TEXTURE_FILTER_POINT
    );

    SetTextureFilter(
        player.spriteBikeStuntL, 
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
        TEXTURE_VALID(player.spriteBikeMovingL) &&
        TEXTURE_VALID(player.spriteBikeStuntR) &&    
        TEXTURE_VALID(player.spriteBikeStuntL)     
    ) {
        player.spritesLoaded = 1;
    }

    player.width =
        player.spriteBikeMovingR.width * player.scale + 50.0f;

    player.height =
        player.spriteBikeMovingR.height * player.scale + 50.0f;

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

    // ===== RESET: Se está groundado, pode fazer double jump =====
    if (player->isGrounded) {
        player->canDoubleJump = 1;
    }
 
    float moveInput = 0.0f;
 
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        moveInput = -1.0f;
    }
 
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        moveInput = 1.0f;
    }
 
    float baseAccel = 800.0f;
    float accelAmount = player->isGrounded ? baseAccel : (baseAccel * AIR_ACCEL_MULT);
 
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
 
    if (IsKeyDown(KEY_W)) {
        player->isPerformingStunt = 1;
    } else {
        player->isPerformingStunt = 0;
    }
 
    if (IsKeyPressed(KEY_SPACE)) {
        player->jumpBufferTimer = JUMP_BUFFER_TIME;
        player->jumpHoldTime = 0.0f;  
    }
 
    if (player->jumpBufferTimer > 0.0f) {
        player->jumpBufferTimer -= deltaTime;
    } else {
        player->jumpBufferTimer = 0.0f;
    }
 
    if (player->isGrounded) {
        player->coyoteTimer = COYOTE_TIME;
    } else {
        player->coyoteTimer -= deltaTime;
    }
 
    if (IsKeyDown(KEY_SPACE)) {
        player->jumpHoldTime += deltaTime;
        if (player->jumpHoldTime > MAX_JUMP_HOLD_TIME) {
            player->jumpHoldTime = MAX_JUMP_HOLD_TIME;
        } 
    } else {
        player->jumpHoldTime = 0.0f;  
    }
 
    // ===== SISTEMA DE PULO =====
    int canJumpNormally = 
        (player->isGrounded || player->coyoteTimer > 0.0f || player->jumpBufferTimer > 0.0f) &&
        IsKeyPressed(KEY_SPACE);

    int canDoubleJump = 
        (!player->isGrounded && player->canDoubleJump) &&
        IsKeyPressed(KEY_SPACE);

    // ===== EXECUTAR PULO =====
    if (canJumpNormally || canDoubleJump) {

        float jumpMultiplier = player->jumpHoldTime / MAX_JUMP_HOLD_TIME;
        if (jumpMultiplier < 0.8f) jumpMultiplier = 0.8f; 
 
        player->velocity.y = -player->jumpPower * jumpMultiplier;
 
        player->isJumping = 1;
        player->isGrounded = 0;

        if (canDoubleJump) {
            player->canDoubleJump = 0;
        }

        if (canJumpNormally) {
            player->coyoteTimer = 0.0f;
            player->jumpBufferTimer = 0.0f;
            player->canDoubleJump = 1;
        }

        player->jumpHoldTime = 0.0f;
 
        player->state = PLAYER_STATE_JUMPING;
    }
 
    if (player->velocity.y > 0.0f) {
        player->acceleration.y = GRAVITY * GRAVITY_FALLING_MULT;
    } else {
        player->acceleration.y = GRAVITY;
    }
 
    player->velocity.y += player->acceleration.y * deltaTime;
 
    player->position.x += player->velocity.x * deltaTime;
    player->position.y += player->velocity.y * deltaTime;
 
    float groundY =
        GLOBAL_GROUND_LEVEL - player->height + 300.0f;
 
    if (player->position.y >= groundY) {
 
        player->position.y = groundY + 160.0f;
 
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
 
    // ===== ATUALIZAR HITBOX A CADA FRAME =====
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

        // Se está fazendo stunt (W pressionado)
        if (player.isPerformingStunt) {
            currentSprite = (player.direction == 'R') 
                ? player.spriteBikeStuntR 
                : player.spriteBikeStuntL;
        }
        // Se está em movimento
        else if (fabs(player.velocity.x) > 10.0f) {
            currentSprite = (player.direction == 'R')
                ? player.spriteBikeMovingR
                : player.spriteBikeMovingL;
        }
        //Em repouso
        else {
            currentSprite = (player.direction == 'R')
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
            player.position.y - player.height + 18.0f,
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

void addUmbrellaShield(Player *player, float duration) {
    player->hasUmbrella = 1;
    player->umbrellaTimer = duration;
}

void applySlowDown(Player *player, float amount, float duration) {
    player->slowEffectTimer = duration;
    player->slowEffectDuration = duration;

    float multiplier = (100.0f - amount) / 100.0f;
    if (multiplier < 0.1f) multiplier = 0.1f;

    player->speedMultiplier = multiplier;
}

void unloadPlayerResources(Player *player) {
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

    directional_animation_unload(&player->anim_standing);
    directional_animation_unload(&player->anim_moving);
    directional_animation_unload(&player->anim_bike_standing);
    directional_animation_unload(&player->anim_bike_moving);
}