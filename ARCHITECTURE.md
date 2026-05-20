# 🏗️ ARQUITETURA - DEIXA EU

## Visão Geral

DEIXA EU é um jogo 2D em C com Raylib seguindo arquitetura **modular, sem frameworks externos** e com foco em código limpo e funcional.

```
┌─────────────────────────────────────┐
│         GAME LOOP (main.c)          │
│  - Menu System                      │
│  - Stage Management                 │
│  - Input Handling                   │
│  - Rendering Pipeline               │
└──────────────┬──────────────────────┘
               │
      ┌────────┴────────┐
      │                 │
   STAGE1          STAGE2/3
  (Recife)     (Boa Viagem/Esculturas)
      │
  ┌───┴─────────────────────┐
  │                         │
ENTITIES            DATA STRUCTURES
  │                         │
  ├─ Player          ├─ ObstacleQueue
  ├─ Bike            └─ PhaseList
  ├─ Obstacles       
  ├─ Rain            
  └─ Collectibles    
```

## Componentes Principais

### 1. CORE LOOP (`src/main.c`)

**Responsabilidades:**
- Inicializar Raylib e janela
- Gerenciar estados (Menu/Gameplay/GameOver)
- Update principal (deltaTime-based)
- Renderização e apresentação
- Limpeza de recursos

**Pseudo-código:**
```c
while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    
    if (inMenu) {
        updateMenu();
    } else {
        updateStage1();
        updatePlayer();
        handleCollisions();
        handleGameOver();
    }
    
    render();
}
```

### 2. ENTITIES (Entidades de Jogo)

Cada entidade segue padrão **Create-Update-Draw-Unload**:

#### Player (`entities/player.h/c`)
- **Estado:** Position, Velocity, Lives, Speed, State
- **Mecânicas:** Movimentação, Pulo, Gravidade, Colisão, Knockback
- **Constantes:** GRAVITY=600, JUMP_FORCE=400, MAX_SPEED=400

```c
Player player = createPlayer(pos, speed, lives);
updatePlayer(&player, dt);      // Lógica + colisão do mundo
drawPlayer(player);             // Renderização
unloadPlayerResources(&player); // Limpeza
```

#### Bike (`entities/bike.h/c`)
- **Função:** Segue o player, rodas giram com velocidade
- **Renderização:** Placeholder (retângulo + círculos) ou sprite

#### Obstacles
- **Hole** (`obstacle.h/c`): Buraco estático na rua
- **Bus** (`bus.h/c`): Ônibus rápido (1.5x scroll speed)
- **Pigeon** (`pigeon.h/c`): Pombo que voa e solta fezes
  - Fezes (`Poop`): Desacelera player

#### Collectibles
- **Umbrella** (`umbrella.h/c`): Power-up temporário (5s)

#### Ambience
- **Rain** (`raindrop.h/c`): 200 gotas max, spawn procedural

### 3. DATA STRUCTURES (Estruturas de Dados)

#### ObstacleQueue (`structure/obstacleQueue.h/c`)

**Implementação:** Fila dinâmica FIFO (linked list)

```c
typedef struct QueueNode {
    QueueObstacle obstacle;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} ObstacleQueue;
```

**Operações (6+):**
- `initObstacleQueue()` - Inicializar vazia
- `enqueueObstacle()` - Adicionar ao fim (O(1))
- `dequeueObstacle()` - Remover do início (O(1))
- `isObstacleQueueEmpty()` - Verificar vazia
- `obstacleQueueSize()` - Contar elementos
- `removeOffscreenObstacles()` - Limpeza em batch (O(n))

**Uso em Stage1:**
```c
Stage1 stage;
initStage1(&stage);  // Inicializa queue

// Durante gameplay:
enqueueObstacle(&stage->obstacleQueue, newObs);
updateObstacles(stage, player, dt);
handleCollisions(stage, player);
```

#### PhaseList (`structure/stepList.h/c`)

**Implementação:** Lista circular duplamente encadeada

```c
typedef struct Phase {
    int phaseNumber;
    char phaseName[50];
    struct Phase *next;    // Circular
    struct Phase *prev;    // Circular
} Phase;
```

**Operações:**
- `createPhase(num, name)` - Criar nó
- `insertPhase(&head, phase)` - Inserir mantendo circularidade
- `nextPhase(cur)` - Próximo (vai para primeiro se no último)
- `prevPhase(cur)` - Anterior (vai para último se no primeiro)
- `phaseCount(head)` - Contar fases

**Propriedade Circular:**
```
[Fase1] <-> [Fase2] <-> [Fase3]
   ^                       |
   +----------- (conectado circulando)
```

### 4. STAGES (Fases)

#### Stage1 (`steps/stage1.h/c`) - "Recife Chuvoso"

**Estado da Fase:**
```c
typedef struct {
    float scrollSpeed;              // Aumenta com tempo
    float distanceTraveled;         // 0 -> 8000m
    float spawnInterval;            // 1.5s -> 0.7s
    float difficultyMultiplier;     // 1.0 -> 2.0
    
    Bike bike;
    RainSystem rain;
    ObstacleQueue obstacleQueue;    // Fila central
    
    Texture2D backgroundTexture;
    float backgroundScroll;
    
    int stage1Complete;
    int stage1Failed;
} Stage1;
```

**Mecânica de Spawn:**
```c
static void spawnRandomObstacle(Stage1 *stage) {
    int roll = rand() % 100;
    
    if (roll < 40)      // 40% Hole
    else if (roll < 70) // 30% Bus
    else if (roll < 95) // 25% Pigeon
    else                // 5% Umbrella (TODO)
}
```

**Progressão de Dificuldade:**
```
progress = distanceTraveled / 8000.0f
difficulty = 1.0 + progress * 1.0         // 1.0 -> 2.0
scrollSpeed = 200 + (200 * progress)       // 200 -> 400 px/s
spawnInterval = 1.5 - (0.8 * progress)    // 1.5 -> 0.7s
```

**Colisões:**
```
Player hitbox <-> Obstacle hitbox
    ↓
    ├─ Hole      → damagePlayer(200)
    ├─ Bus       → damagePlayer(300)
    ├─ Pigeon    → damagePlayer(100)
    └─ Poop      → applySlowDown(50, 2s)
```

### 5. MENU SYSTEM

**Arquivo:** `src/menu.c/h`

Estados:
- MENU_MAIN: Play / Options / Quit
- MENU_OPTIONS: Settings

### 6. CONSTANTS & DEFINES

**Global** (`player.h`):
```c
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define GROUND_LEVEL 350.0f
#define PLAYER_WIDTH 30.0f
#define PLAYER_HEIGHT 40.0f
```

**Physics** (`player.c`):
```c
#define GRAVITY 600.0f
#define JUMP_FORCE 400.0f
#define PLAYER_MAX_SPEED 400.0f
```

**Stage1** (`stage1.h`):
```c
#define STAGE1_TARGET_DISTANCE 8000.0f
#define STAGE1_BASE_SCROLL_SPEED 200.0f
#define STAGE1_MAX_SCROLL_SPEED 400.0f
```

## Fluxo de Execução

### Frame Principal
```
1. Input          → Player keyboard input
2. Update         → Physics, Entities, Obstacles
3. Collision      → CheckCollisionRecs()
4. Game Logic     → Difficulty, Spawn, Win/Lose
5. Render         → Draw Stage, Player, HUD
6. Sleep          → Target FPS = 60
```

### Controles
- **A/LEFT:** Mover esquerda
- **D/RIGHT:** Mover direita
- **SPACE/UP/W:** Pular
- **D:** Toggle debug mode
- **F:** Toggle fullscreen
- **ENTER:** Confirm menu

## Memory Management

**Alocação dinâmica:**
- `ObstacleQueue`: `malloc()` cada node ao enqueue
- `PhaseList`: `malloc()` cada phase
- `Textures`: `LoadTexture()` do Raylib

**Liberação:**
- `freeObstacleQueue()`: `free()` cada node
- `freePhaseList()`: `free()` cada phase
- `unloadStage1()`: `UnloadTexture()`
- `unloadPlayerResources()`: `UnloadTexture()`

**Zero Memory Leaks:** Cada `malloc()` tem `free()` pareado

## Extensibilidade

### Adicionar Novo Obstáculo

1. Criar `entities/myobstacle.h/c`
2. Definir tipo em `obstacleQueue.h`:
   ```c
   typedef enum {
       ...
       QUEUE_OBS_MYOBSTACLE = 4
   } QueueObstacleType;
   ```
3. Adicionar à union em `QueueObstacle`
4. Implementar spawn em `stage1.c:spawnRandomObstacle()`
5. Implementar colisão em `stage1.c:handleCollisions()`

### Adicionar Nova Stage

1. Criar `steps/stage2.h/c` (padrão initStage2/updateStage2/drawStage2)
2. Inserir na `PhaseList` em `main.c`
3. Adicionar switch no game loop para chamar `updateStage2()`

## Padrões de Código

### Include Guards
```c
#ifndef FILE_H
#define FILE_H
... código ...
#endif
```

### Função Entity Template
```c
// Criar
Entity createEntity(...) {
    Entity e;
    e.position = ...;
    e.texture = LoadTexture(...);
    return e;
}

// Atualizar
void updateEntity(Entity *e, float dt) { ... }

// Desenhar
void drawEntity(Entity e) { ... }

// Limpar
void unloadEntityResources(Entity *e) { ... }
```

### Colisão Padrão
```c
if (CheckCollisionRecs(player->hitbox, obstacle.hitbox)) {
    // Aplicar efeito
    // Marcar como inativo
}
```

## Performance

- **Entities:** ~20 simultâneos (Player + ~15 Obstacles)
- **Rain:** 200 partículas max
- **Frame Rate:** 60 FPS estável
- **Memory:** < 50MB (sem assets pesados)

## Build System

**Makefile Flags:**
```makefile
CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags raylib)
LDFLAGS = $(shell pkg-config --libs raylib) -lm
```

**Targets:**
- `make` = Compilar
- `make run` = Executar
- `make clean` = Limpar build/
