#include "raindrop.h"

Raindrop createRaindrop(Vector2 position) {
    Raindrop raindrop;
    raindrop.position = position;
    raindrop.hitbox = (Rectangle){
        position.x,
        position.y,
        RAINDROP_WIDTH,
        RAINDROP_HEIGHT
    };
    raindrop.speed = RAINDROP_SPEED;
    raindrop.isActive = 1;
    
    return raindrop;
}


void updateRaindrop(Raindrop* raindrop, float deltaTime) {
    if (!raindrop->isActive) return;
    
    // Cair verticalmente
    raindrop->position.y += raindrop->speed * deltaTime;
    raindrop->hitbox.y = raindrop->position.y;
    
    // Desativar quando chega ao chão
    if (raindrop->position.y > 450) {  // Abaixo da tela
        raindrop->isActive = 0;
    }
}

void drawRaindrop(Raindrop raindrop) {
    if (!raindrop.isActive) return;
    DrawRectangleRec(raindrop.hitbox, BLUE);  
}