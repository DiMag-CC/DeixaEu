#ifndef PLAYER_H
#define PLAYER_H

#include "../gfx/animation.h"
#include <raylib.h>

#define SCREEN_WIDTH 1920.0f
#define SCREEN_HEIGHT 1080.0f
#define PLAYER_WIDTH 30.0f
#define PLAYER_HEIGHT 40.0f

typedef enum {
  PLAYER_STATE_IDLE = 0,
  PLAYER_STATE_RUNNING = 1,
  PLAYER_STATE_JUMPING = 2,
  PLAYER_STATE_FALLING = 3,
  PLAYER_STATE_HIT = 4,
  PLAYER_STATE_UMBRELLA_BUFF = 5,
  PLAYER_STATE_DEAD = 6
} PlayerState;

typedef struct {
  Vector2 position;
  Vector2 velocity;
  Vector2 acceleration;
  Rectangle hitbox;

  float speed;
  float maxSpeed;
  float width;
  float height;
  float scale;
  float coyoteTimer;
  float jumpBufferTimer;
  float jumpHoldTime;
  float airAccelerationMult;

  int lives;
  float score;

  int isGrounded;
  int isJumping;
  int isPerformingStunt;
  int canDoubleJump;

  float jumpPower;
  float fallSpeed;

  PlayerState state;
  float animationTimer;
  int animationFrame;

  int hasUmbrella;
  float umbrellaTimer;

  float knockbackSpeed;
  float knockbackTimer;
  float invincibilityTimer;

  float slowEffectTimer;
  float slowEffectDuration;
  float speedMultiplier;

  int isClimbing;
  int movementControlledExternally;
  int grounded;

  Texture2D spriteStandingR, spriteStandingL;
  Texture2D spriteMovingR, spriteMovingL;
  Texture2D spriteJumpingR, spriteJumpingL;
  Texture2D spriteBikeStandingR, spriteBikeStandingL;
  Texture2D spriteBikeMovingR, spriteBikeMovingL;
  Texture2D spriteBikeStuntL, spriteBikeStuntR;

  Texture2D spriteBikeStandingUmbrellaR, spriteBikeStandingUmbrellaL;
  Texture2D spriteBikeMovingUmbrellaR, spriteBikeMovingUmbrellaL;
  Texture2D spriteBikeStuntUmbrellaR, spriteBikeStuntUmbrellaL;

  int spritesLoaded;

  DirectionalAnimationSet anim_standing;
  DirectionalAnimationSet anim_moving;
  DirectionalAnimationSet anim_bike_standing;
  DirectionalAnimationSet anim_bike_moving;

  char direction;
  int on_bike;

} Player;

Player createPlayer(Vector2 startPos, float startSpeed, int lives);
void updatePlayer(Player *player, float deltaTime);
void drawPlayer(Player player);
void damagePlayer(Player *player, float knockback);
void healPlayer(Player *player);
void addUmbrellaShield(Player *player, float duration);
void applySlowDown(Player *player, float amount, float duration);
void unloadPlayerResources(Player *player);

#endif