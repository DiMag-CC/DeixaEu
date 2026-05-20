# 🎬 ESPECIFICAÇÃO DO SISTEMA DE ANIMAÇÃO

## Visão Geral

O sistema de animação deve ser **procedural**, **baseado em deltaTime** e **automaticamente gerenciado** pelos componentes de entidade, usando a convenção de nomenclatura como diretiva de implementação.

---

## 1. ESTRUTURAS DE DADOS

### 1.1 AnimationFrame
```c
typedef struct {
  const char* filename;      // e.g., "pigeon1L.png"
  SDL_Texture* texture;      // Carregada automaticamente
  uint32_t width;
  uint32_t height;
} AnimationFrame;
```

### 1.2 AnimationSequence
```c
typedef struct {
  AnimationFrame frames[4];  // Máximo 4 frames (1, 2, 3, 4)
  int frame_count;
  float fps;                 // Frames por segundo
  bool loop;                 // Loop automático?
  float elapsed_time;        // Acumulador deltaTime
  int current_frame_index;   // Frame atual (0, 1, 2, 3)
} AnimationSequence;
```

### 1.3 DirectionalAnimationSet
```c
typedef struct {
  AnimationSequence left;    // Frames com sufixo 'L'
  AnimationSequence right;   // Frames com sufixo 'R'
  char direction;            // 'L' ou 'R' - direção atual
} DirectionalAnimationSet;
```

### 1.4 CompleteCharacterAnimationState
```c
typedef struct {
  DirectionalAnimationSet idle;      // Standing frames
  DirectionalAnimationSet moving;    // Moving frames
  DirectionalAnimationSet bike_idle; // Bike Standing
  DirectionalAnimationSet bike_move; // Bike Moving (rodas animadas!)
  
  int state;                 // 0=idle, 1=moving, 2=bike_idle, 3=bike_moving
  float elapsed_time;        // Acumulador global
} CharacterAnimationState;
```

---

## 2. NOMES DE ARQUIVOS → MAPEAMENTO DIRETO

A convenção de nomenclatura **DEFINE** como carregar e animar:

```
PADRÃO:           pigeon[FRAME][DIRECTION].png
EXEMPLO:          pigeon1L.png, pigeon2R.png

EXTRAÇÃO:
- Frame:          Número entre nome e sufixo (1, 2, 3, 4)
- Direction:      Último char antes de .png (L ou R)
```

### 2.1 Parser de Nomenclatura
```c
typedef struct {
  char base_name[64];     // e.g., "pigeon"
  int frame_numbers[4];   // Frames encontrados: [1, 2, 0, 0]
  int frame_count;        // Quantos frames: 2
  bool has_left;          // Tem 'L'?
  bool has_right;         // Tem 'R'?
} AssetNamingInfo;

// Implementação
AssetNamingInfo parse_animation_name(const char* filename) {
  // Extract: pigeon1L.png → base="pigeon", frame=1, dir='L'
  // Buscar todos os arquivos pigeon[1-4][LR].png
  // Retornar info estruturada
}
```

---

## 3. SISTEMA DE CARREGAMENTO AUTOMÁTICO

### 3.1 Asset Loader
```c
// Carregar TUDO automaticamente baseado em nomes
AnimationSequence load_animation_sequence(
  const char* base_name,
  char direction,          // 'L' ou 'R'
  float fps,
  bool loop
) {
  AnimationSequence seq = { 0 };
  seq.fps = fps;
  seq.loop = loop;
  seq.frame_count = 0;
  
  // Buscar pigeon1L.png, pigeon2L.png, pigeon3L.png, pigeon4L.png
  for (int frame = 1; frame <= 4; frame++) {
    char filename[256];
    snprintf(filename, sizeof(filename), 
             "assets/img/%s%d%c.png", 
             base_name, frame, direction);
    
    SDL_Texture* tex = load_texture(filename);
    if (tex) {
      seq.frames[seq.frame_count].filename = strdup(filename);
      seq.frames[seq.frame_count].texture = tex;
      seq.frame_count++;
    }
  }
  
  return seq;
}
```

### 3.2 Carregador de Conjunto Direcional
```c
DirectionalAnimationSet load_directional_animation(
  const char* base_name,
  float fps,
  bool loop
) {
  DirectionalAnimationSet set;
  set.left = load_animation_sequence(base_name, 'L', fps, loop);
  set.right = load_animation_sequence(base_name, 'R', fps, loop);
  set.direction = 'R'; // Padrão
  return set;
}

// Uso:
DirectionalAnimationSet pigeon = 
  load_directional_animation("pigeon", 12.0f, true);
```

---

## 4. ATUALIZAÇÃO DE ANIMAÇÃO COM DELTATIME

### 4.1 Update Animation
```c
void animation_sequence_update(
  AnimationSequence* seq,
  float delta_time
) {
  if (seq->frame_count == 0) return;
  
  // Acumular tempo
  seq->elapsed_time += delta_time;
  
  // Calcular frame baseado em FPS
  float time_per_frame = 1.0f / seq->fps;
  int frame_index = (int)(seq->elapsed_time / time_per_frame);
  
  // Loop automático
  if (seq->loop) {
    frame_index = frame_index % seq->frame_count;
  } else {
    if (frame_index >= seq->frame_count) {
      frame_index = seq->frame_count - 1; // Parar no último
    }
  }
  
  seq->current_frame_index = frame_index;
}

// Uso em game loop:
void game_update(float delta_time) {
  animation_sequence_update(&pigeon.left, delta_time);
  animation_sequence_update(&pigeon.right, delta_time);
  // ... todos os outros
}
```

### 4.2 Update Direcional
```c
void directional_animation_update(
  DirectionalAnimationSet* set,
  float delta_time
) {
  animation_sequence_update(&set->left, delta_time);
  animation_sequence_update(&set->right, delta_time);
}
```

---

## 5. RENDERIZAÇÃO

### 5.1 Render Frame Atual
```c
void render_animation_sequence(
  AnimationSequence* seq,
  int x, int y,
  float scale
) {
  if (seq->frame_count == 0) return;
  
  AnimationFrame* frame = &seq->frames[seq->current_frame_index];
  if (!frame->texture) return;
  
  SDL_Rect dest = {
    x, y,
    (int)(frame->width * scale),
    (int)(frame->height * scale)
  };
  
  SDL_RenderCopy(renderer, frame->texture, NULL, &dest);
}

// Uso:
render_animation_sequence(&pigeon.left, player_x, player_y, 2.0f);
```

### 5.2 Render Direcional Automático
```c
void render_directional_animation(
  DirectionalAnimationSet* set,
  int x, int y,
  float scale
) {
  AnimationSequence* seq = (set->direction == 'L') 
    ? &set->left 
    : &set->right;
  
  render_animation_sequence(seq, x, y, scale);
}

// Uso - automático:
render_directional_animation(&pigeon, pombo_x, pombo_y, 1.5f);
```

---

## 6. SISTEMA DE ESTADO DO PLAYER

### 6.1 Player Animation State
```c
typedef struct {
  DirectionalAnimationSet idle;
  DirectionalAnimationSet moving;
  DirectionalAnimationSet bike_idle;
  DirectionalAnimationSet bike_moving;
  
  enum {
    PLAYER_IDLE = 0,
    PLAYER_MOVING = 1,
    PLAYER_BIKE_IDLE = 2,
    PLAYER_BIKE_MOVING = 3
  } state;
  
  char direction; // 'L' ou 'R'
} PlayerAnimationState;
```

### 6.2 Update Player State
```c
void player_animation_update(
  PlayerAnimationState* anim,
  Player* player,
  float delta_time
) {
  // Atualizar todas as sequências
  animation_sequence_update(&anim->idle.left, delta_time);
  animation_sequence_update(&anim->idle.right, delta_time);
  animation_sequence_update(&anim->moving.left, delta_time);
  animation_sequence_update(&anim->moving.right, delta_time);
  animation_sequence_update(&anim->bike_idle.left, delta_time);
  animation_sequence_update(&anim->bike_idle.right, delta_time);
  animation_sequence_update(&anim->bike_moving.left, delta_time);
  animation_sequence_update(&anim->bike_moving.right, delta_time);
  
  // Atualizar direção baseado em velocidade
  if (player->velocity_x > 0) {
    anim->direction = 'R';
  } else if (player->velocity_x < 0) {
    anim->direction = 'L';
  }
  
  // Atualizar estado
  if (player->on_bike) {
    anim->state = (player->velocity_x != 0) 
      ? PLAYER_BIKE_MOVING 
      : PLAYER_BIKE_IDLE;
  } else {
    anim->state = (player->velocity_x != 0) 
      ? PLAYER_MOVING 
      : PLAYER_IDLE;
  }
}
```

### 6.3 Render Player
```c
void player_animation_render(
  PlayerAnimationState* anim,
  Player* player
) {
  DirectionalAnimationSet* current_set;
  
  switch (anim->state) {
    case PLAYER_IDLE:        current_set = &anim->idle; break;
    case PLAYER_MOVING:      current_set = &anim->moving; break;
    case PLAYER_BIKE_IDLE:   current_set = &anim->bike_idle; break;
    case PLAYER_BIKE_MOVING: current_set = &anim->bike_moving; break;
  }
  
  // Forçar direção correta
  current_set->direction = anim->direction;
  
  render_directional_animation(
    current_set,
    player->x, player->y,
    player->scale
  );
}
```

---

## 7. ANIMAÇÕES POR TIPO DE ENTIDADE

### 7.1 Pigeon
```c
DirectionalAnimationSet pigeon_animation = 
  load_directional_animation("pigeon", 12.0f, true);

// Atualizar em game loop
animation_sequence_update(&pigeon_animation.left, delta_time);
animation_sequence_update(&pigeon_animation.right, delta_time);

// Renderizar
render_directional_animation(&pigeon_animation, pigeon_x, pigeon_y, 1.5f);
```

### 7.2 Jellyfish
```c
AnimationSequence jellyfish_animation = 
  load_animation_sequence("jellyfish", ' ', 10.0f, true);
// Nota: Jellyfish não tem L/R, só um único arquivo com frame

// Renderizar
render_animation_sequence(&jellyfish_animation, jelly_x, jelly_y, 1.2f);
```

### 7.3 Bike Wheels (Special)
```c
// Rodas animadas automaticamente quando moving!
DirectionalAnimationSet bike_moving = 
  load_directional_animation("CharacterBikeMoving", 15.0f, true);
// Quando on_bike && moving, usar esta com frame de rodas

// Renderizar
render_directional_animation(&bike_moving, player_x, player_y, 2.0f);
```

---

## 8. INTEGRAÇÃO COM ENTIDADES EXISTENTES

### 8.1 Estrutura de Entidade Genérica
```c
typedef struct Entity {
  // ... posição, velocidade, etc
  AnimatedSprite animation;
  float animation_time;
} Entity;

// Update
void entity_update(Entity* e, float delta_time) {
  // ... lógica de movimento
  
  // Atualizar animação
  animated_sprite_update(&e->animation, delta_time);
}

// Render
void entity_render(Entity* e) {
  render_animated_sprite(&e->animation, e->x, e->y, 1.0f);
}
```

### 8.2 Registro Global de Assets
```c
typedef struct {
  // Player
  PlayerAnimationState player;
  
  // Enemies
  DirectionalAnimationSet pigeon;
  AnimationSequence jellyfish;
  
  // Projectiles
  AnimationSequence poop;
  
  // Others
  DirectionalAnimationSet bike;
} GlobalAnimationAssets;

GlobalAnimationAssets g_assets;

// Inicialização
void assets_init() {
  g_assets.player.idle = 
    load_directional_animation("Character", 8.0f, false);
  g_assets.player.moving = 
    load_directional_animation("characterMoving", 12.0f, true);
  g_assets.pigeon = 
    load_directional_animation("pigeon", 12.0f, true);
  // ... etc
}
```

---

## 9. FPS DE ANIMAÇÃO POR TIPO

| Entidade | FPS | Razão |
|----------|-----|-------|
| Player Idle | 8 | Estático, sem movimento |
| Player Moving | 12 | Movimento natural |
| Pigeon | 12 | Bater de asas fluido |
| Jellyfish | 10 | Movimentoaquático lento |
| Bike Wheels | 15 | Rotação rápida |
| Crab | 8 | Caminhada lenta |
| Shark | 12 | Movimento rápido |

---

## 10. PROBLEMAS E SOLUÇÕES

### Problema: Missing Frame 2 para Moving
```
❌ Arquivo: characterMovingL1.png
✅ Solução: Criar characterMovingL2.png (sprite frame 2)

Sem frame 2, o ciclo fica:
- 0-0.4s: Frame 1 (50% tempo)
- 0.4-0.5s: Frame 1 (congelado!)

Com frame 2:
- 0-0.4s: Frame 1 (50% tempo)
- 0.4-0.5s: Frame 2 (50% tempo) ✅ Fluido
```

### Problema: Direção e Espelhamento
```
❌ Carregar pigeon1L.png e depois espelhar horizontalmente
❌ Causa distorção de visão em pixel art

✅ Usar pigeon1L.png e pigeon1R.png separados
✅ Renderizar correto sem transformação
```

### Problema: DeltaTime
```
❌ Usar frame count direto: frame = (int)(game_frame % 2)
❌ Desacoplado de velocidade do game

✅ elapsed_time += delta_time
✅ frame = (int)(elapsed_time * fps) % frame_count
✅ Acoplado à física real
```

---

## 11. EXEMPLO DE IMPLEMENTAÇÃO COMPLETA

```c
// === INICIALIZAÇÃO ===
void game_init() {
  assets_init();
}

void assets_init() {
  g_assets.pigeon = load_directional_animation("pigeon", 12.0f, true);
}

// === GAME LOOP ===
void game_update(float delta_time) {
  // Atualizar animações
  animation_sequence_update(&g_assets.pigeon.left, delta_time);
  animation_sequence_update(&g_assets.pigeon.right, delta_time);
  
  // Atualizar direção baseado em lógica
  if (pigeon.x > player.x) {
    g_assets.pigeon.direction = 'L';
  } else {
    g_assets.pigeon.direction = 'R';
  }
}

void game_render() {
  // Renderizar pombo com frame animado correto
  render_directional_animation(
    &g_assets.pigeon,
    pigeon.x, pigeon.y,
    1.5f
  );
}
```

---

## 12. CONVERSÃO DE ENTIDADES EXISTENTES

Para cada entidade em `src/entities/`:

1. **Adicionar campo `AnimationSequence`** ou `DirectionalAnimationSet`
2. **Remover carregamento de texture manual**
3. **Adicionar call em `_update()`** para `animation_sequence_update()`
4. **Substituir `SDL_RenderCopy()` por `render_animation_sequence()`**
5. **Testar FPS visualmente**

---

**Status:** ✅ Especificação Completa  
**Próximo:** Implementar `animation.h/c` no projeto
