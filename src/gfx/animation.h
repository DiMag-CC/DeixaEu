#ifndef ANIMATION_H
#define ANIMATION_H

#include <raylib.h>

// ========== FRAME DE ANIMAÇÃO ==========
typedef struct {
    Texture2D texture;
    const char* filename;
    int width;
    int height;
} AnimationFrame;

// ========== SEQUÊNCIA DE ANIMAÇÃO ==========
typedef struct {
    AnimationFrame frames[4];
    int frame_count;
    float fps;
    int loop;
    float elapsed_time;
    int current_frame_index;
} AnimationSequence;

// ========== SET DIRECIONAL DE ANIMAÇÃO ==========
typedef struct {
    AnimationSequence left;
    AnimationSequence right;
    char direction; // 'L' ou 'R'
} DirectionalAnimationSet;

// ========== FUNÇÕES DE CARREGAMENTO ==========
AnimationSequence animation_load_sequence(const char* base_name, char direction, float fps, int loop);
DirectionalAnimationSet animation_load_directional(const char* base_name, float fps, int loop);

// ========== FUNÇÕES DE ATUALIZAÇÃO ==========
void animation_sequence_update(AnimationSequence* seq, float delta_time);
void directional_animation_update(DirectionalAnimationSet* set, float delta_time);

// ========== FUNÇÕES DE RENDERIZAÇÃO ==========
void animation_sequence_render(AnimationSequence* seq, float x, float y, float scale, Color tint);
void directional_animation_render(DirectionalAnimationSet* set, float x, float y, float scale, Color tint);

// ========== LIMPEZA ==========
void animation_sequence_unload(AnimationSequence* seq);
void directional_animation_unload(DirectionalAnimationSet* set);

#endif
