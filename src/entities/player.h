#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include "../gfx/animation.h"

// ========== CONSTANTES GLOBAIS ==========
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define PLAYER_WIDTH 30.0f
#define PLAYER_HEIGHT 40.0f

// ========== ESTADOS DO JOGADOR ==========
typedef enum {
    PLAYER_STATE_IDLE             = 0,
    PLAYER_STATE_RUNNING          = 1,
    PLAYER_STATE_JUMPING          = 2,
    PLAYER_STATE_FALLING          = 3,
    PLAYER_STATE_HIT              = 4,  // Knockback visual
    PLAYER_STATE_UMBRELLA_BUFF    = 5,  // Com proteção de guarda-chuva
    PLAYER_STATE_DEAD             = 6
} PlayerState;

// ========== ESTRUTURA DO JOGADOR ==========
typedef struct {
    Vector2 position;           // Posição (x, y)
    Vector2 velocity;           // Velocidade (vx, vy)
    Vector2 acceleration;       // Aceleração (ax, ay)
    Rectangle hitbox;           // Hitbox pra colisão

    float speed;                // Velocidade atual (m/s)
    float maxSpeed;             // Velocidade máxima
    float width;                // Largura
    float height;               // Altura
    float scale;                // Escala do sprite (multiplicador)

    int lives;                  // Número de vidas (0-3)
    float score;                // Pontuação

    int isGrounded;             // 1 = no chão, 0 = no ar
    int isJumping;              // 1 = saltando, 0 = não
    float jumpPower;            // Força do pulo
    float fallSpeed;            // Velocidade de queda

    PlayerState state;          // Estado atual
    float animationTimer;       // Timer de animação
    int animationFrame;         // Frame de animação

    // Power-ups
    int hasUmbrella;            // 1 = tem guarda-chuva
    float umbrellaTimer;        // Tempo restante de proteção

    // Knockback
    float knockbackSpeed;       // Velocidade de knockback
    float knockbackTimer;       // Timer de knockback

    // Debuffs
    float slowEffectTimer;      // Timer de efeito de lentidão
    float slowEffectDuration;   // Duração total do efeito
    float speedMultiplier;      // Multiplicador de velocidade (0.5 = metade)

    // Stage3 específico
    int isClimbing;             // 1 = escalando, 0 = não
    int movementControlledExternally;  // 1 = stage controla, 0 = player controla
    int grounded;               // Alias para isGrounded (compatibilidade stage3)

    // Sprites e texturas — sprites únicos diretos (não multi-frame)
    Texture2D spriteStandingR, spriteStandingL;
    Texture2D spriteMovingR, spriteMovingL;
    Texture2D spriteBikeStandingR, spriteBikeStandingL;
    Texture2D spriteBikeMovingR, spriteBikeMovingL;
    int spritesLoaded;          // 1 = texturas carregaram

    // Animações direcionais (fallback, podem estar vazias)
    DirectionalAnimationSet anim_standing;
    DirectionalAnimationSet anim_moving;
    DirectionalAnimationSet anim_bike_standing;
    DirectionalAnimationSet anim_bike_moving;

    char direction; // 'L' ou 'R' - direção atual
    int on_bike;    // 1 = na bike, 0 = a pé

} Player;

// ========== FUNÇÕES DO JOGADOR ==========
Player createPlayer(Vector2 startPos, float startSpeed, int lives);
void updatePlayer(Player *player, float deltaTime);
void drawPlayer(Player player);
void damagePlayer(Player *player, float knockback);
void healPlayer(Player *player);
void addUmbrellaShield(Player *player, float duration);
void applySlowDown(Player *player, float amount, float duration);
void unloadPlayerResources(Player *player);

#endif
