#ifndef STAGE3_H
#define STAGE3_H

#include <raylib.h>
#include "../entities/player.h"

#define MAX_CLOUDS 2
#define MAX_BIRDS 3
#define MAX_BIRD_POOPS 10
#define MAX_PUDDLES 7

typedef enum {
    STAGE3_APPROACH, // Camera horizontal ate encontrar a torre
    STAGE3_CLIMBING, // Jogador escala, scroll vertical
    STAGE3_FINISHED
} Stage3State;

typedef struct {
    Vector2 position;
    float speed;
    float scale;
} Cloud;

typedef struct {
    Vector2 position;
    float speed;
    float poopTimer;
    float poopInterval;
} Bird;

typedef struct {
    Vector2 position;
    bool active;
    bool landed;
    float speedY;
    float groundTimer;
} BirdPoop;

typedef struct {
    Vector2 position;
    Rectangle hitbox;
    Vector2 bottlePosition;
    Rectangle bottleHitbox;
    bool active;
    bool canLockPlayer;
} Puddle;

typedef struct Stage3 {
    Stage3State state;
    Texture2D towerTexture;
    Texture2D cloudTexture;
    Texture2D birdTexture;
    
    float scrollX;
    float scrollY;
    float puddleLockTimer;
    float puddlePushVelocity;
    bool ambientSpawningEnabled;
    
    Rectangle towerHitbox;
    Vector2 towerPosition;
    
    Cloud clouds[MAX_CLOUDS];
    Bird birds[MAX_BIRDS];
    BirdPoop poops[MAX_BIRD_POOPS];
    Puddle puddles[MAX_PUDDLES];
    
} Stage3;

void initStage3(Stage3 *stage, Player *player);
void updateStage3(Stage3 *stage, Player *player, float deltaTime);
void drawStage3(Stage3 *stage, Player *player);
void unloadStage3(Stage3 *stage);

#endif
