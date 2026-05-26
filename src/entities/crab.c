#include <raylib.h>
#include "crab.h"

Crab createCrab(Vector2 position, int frameSpeed) {
    Crab crab;
    crab.position = position;
    // CRAB_WIDTH e CRAB_HEIGHT devem estar definidos no seu crab.h (ex: 140.0f e 85.0f)
    crab.hitbox = (Rectangle){position.x, position.y, CRAB_WIDTH, CRAB_HEIGHT};
    crab.currentFrame = 0;
    crab.frameCounter = 0;
    crab.frameSpeed = frameSpeed;
    return crab;
}

void updateCrab(Crab* crab) {
    crab->frameCounter++;
    if (crab->frameCounter >= crab->frameSpeed) {
        crab->currentFrame++;
        // CRAB_NUM_FRAMES deve ser 2, já que temos 2 imagens separadas
        if (crab->currentFrame >= CRAB_NUM_FRAMES) {
            crab->currentFrame = 0;
        }
        crab->frameCounter = 0;
    }
}

// Agora aceita um ponteiro/array contendo as duas texturas carregadas
void drawCrab(Crab crab, Texture2D crabTextures[]) {
    Texture2D texturaAtual = crabTextures[crab.currentFrame];
    
    Rectangle sourceRec = {0.0f, 0.0f, (float)texturaAtual.width, (float)texturaAtual.height};
    Rectangle destRec = {crab.position.x, crab.position.y, CRAB_WIDTH, CRAB_HEIGHT};
    Vector2 origin = {0.0f, 0.0f};
    
    DrawTexturePro(texturaAtual, sourceRec, destRec, origin, 0.0f, WHITE);
}