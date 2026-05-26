#include "animation.h"
#include <stdio.h>
#include <string.h>

AnimationSequence animation_load_sequence(const char* base_name, char direction, float fps, int loop) {
    AnimationSequence seq = {0};
    seq.fps = fps;
    seq.loop = loop;
    seq.frame_count = 0;
    seq.elapsed_time = 0.0f;
    seq.current_frame_index = 0;

    // Buscar frames: base_name1[dir].png, base_name2[dir].png, etc
    for (int frame = 1; frame <= 4; frame++) {
        char filename[256];

        if (direction == 'L' || direction == 'R') {
            snprintf(filename, sizeof(filename), "assets/img/%s%d%c.png", base_name, frame, direction);
        } else {
            // Para animações sem direção (como jellyfish)
            snprintf(filename, sizeof(filename), "assets/img/%s%d.png", base_name, frame);
        }

        // Tentar carregar textura
        if (FileExists(filename)) {
            Texture2D tex = LoadTexture(filename);
            if (tex.id != 0) {
                seq.frames[seq.frame_count].texture = tex;
                seq.frames[seq.frame_count].filename = filename;
                seq.frames[seq.frame_count].width = tex.width;
                seq.frames[seq.frame_count].height = tex.height;
                seq.frame_count++;
            }
        }
    }

    return seq;
}

DirectionalAnimationSet animation_load_directional(const char* base_name, float fps, int loop) {
    DirectionalAnimationSet set = {0};
    set.left = animation_load_sequence(base_name, 'L', fps, loop);
    set.right = animation_load_sequence(base_name, 'R', fps, loop);
    set.direction = 'R';
    return set;
}

void animation_sequence_update(AnimationSequence* seq, float delta_time) {
    if (seq->frame_count == 0) return;

    seq->elapsed_time += delta_time;

    float time_per_frame = 1.0f / seq->fps;
    int frame_index = (int)(seq->elapsed_time / time_per_frame);

    if (seq->loop) {
        frame_index = frame_index % seq->frame_count;
    } else {
        if (frame_index >= seq->frame_count) {
            frame_index = seq->frame_count - 1;
        }
    }

    seq->current_frame_index = frame_index;
}

void directional_animation_update(DirectionalAnimationSet* set, float delta_time) {
    animation_sequence_update(&set->left, delta_time);
    animation_sequence_update(&set->right, delta_time);
}

void animation_sequence_render(AnimationSequence* seq, float x, float y, float scale, Color tint) {
    if (seq->frame_count == 0) return;

    AnimationFrame* frame = &seq->frames[seq->current_frame_index];
    if (frame->texture.id == 0) return;

    Rectangle source = {0, 0, (float)frame->width, (float)frame->height};
    Rectangle dest = {
        x - (frame->width * scale) / 2,
        y - frame->height * scale,
        frame->width * scale,
        frame->height * scale
    };

    DrawTexturePro(frame->texture, source, dest, (Vector2){0, 0}, 0, tint);
}

void directional_animation_render(DirectionalAnimationSet* set, float x, float y, float scale, Color tint) {
    AnimationSequence* seq = (set->direction == 'L') ? &set->left : &set->right;
    animation_sequence_render(seq, x, y, scale, tint);
}

void animation_sequence_unload(AnimationSequence* seq) {
    for (int i = 0; i < seq->frame_count; i++) {
        if (seq->frames[i].texture.id != 0) {
            UnloadTexture(seq->frames[i].texture);
        }
    }
    seq->frame_count = 0;
}

void directional_animation_unload(DirectionalAnimationSet* set) {
    animation_sequence_unload(&set->left);
    animation_sequence_unload(&set->right);
}
