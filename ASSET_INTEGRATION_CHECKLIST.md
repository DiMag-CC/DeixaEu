# ✅ CHECKLIST DE INTEGRAÇÃO DE ASSETS

## 📊 STATUS ATUAL

| Etapa | Status | Arquivo |
|-------|--------|---------|
| Extração de Assets | ✅ COMPLETO | `assets/img/` (27 PNGs) |
| Análise Completa | ✅ COMPLETO | `ASSETS_ANALYSIS.md` |
| Especificação Sistema | ✅ COMPLETO | `ANIMATION_SYSTEM_SPEC.md` |
| **Implementação** | ⏳ PENDENTE | ↓ |

---

## 🎯 IMPLEMENTAÇÃO - ORDEM SUGERIDA

### FASE 1: CORE ANIMATION SYSTEM (Semana 1)
- [ ] Criar `src/gfx/animation.h`
- [ ] Implementar estruturas: `AnimationFrame`, `AnimationSequence`, `DirectionalAnimationSet`
- [ ] Implementar `load_animation_sequence()` - carregador automático
- [ ] Implementar `animation_sequence_update()` - deltaTime
- [ ] Implementar `render_animation_sequence()` - renderizar frame atual
- [ ] **Teste:** Animar pombo (pigeon1L/R → pigeon2L/R)
- [ ] **Commit:** `feat: implementa sistema de animação por frames`

### FASE 2: PLAYER ANIMATIONS (Semana 1)
- [ ] Implementar `PlayerAnimationState` em player.h
- [ ] Carregar idle (CharacterStandingL/R) com 8 FPS
- [ ] Carregar moving (characterMovingL1/R) com 12 FPS ⚠️ Frame 2 faltando!
- [ ] Implementar `player_animation_update()` - automática direção
- [ ] Testar mudança automática L/R baseado em velocity_x
- [ ] **Teste:** Player parado → correndo → parado (com animação fluida)
- [ ] **Commit:** `feat: melhora movimentação e animações do player`

### FASE 3: BIKE SYSTEM (Semana 1)
- [ ] Carregar bike idle (CharacterBikeStandingL/R) - **SEM animação**
- [ ] Carregar bike moving (CharacterBikeMovingL/R) - **COM animação de rodas** (15 FPS)
- [ ] Implementar transição automática idle → moving quando em bike
- [ ] Testar seleção correta de sprite conforme estado
- [ ] **Teste:** Player sobe bike → rodas giram durante movimento
- [ ] **Commit:** `feat: implementa sistema de animação de bicicleta`

### FASE 4: ENEMY ANIMATIONS (Semana 2)
- [ ] Pigeon: Carregar pigeon1L/R + pigeon2L/R (12 FPS, loop)
- [ ] Pigeon: Implementar direção automática baseado em movimento
- [ ] Pigeon: Testar animação de asas (batidas fluidas)
- [ ] Jellyfish: Carregar jellyfish1 + jellyfish2 (10 FPS, loop)
- [ ] Jellyfish: Testar flutuação vertical suave
- [ ] **Teste:** Múltiplos pombos + águas-vivas animando independentemente
- [ ] **Commit:** `fix: corrige sistema visual dos pombos e águas-vivas`

### FASE 5: BACKGROUND & RESPONSIVENESS (Semana 2)
- [ ] Carregar landscapeLevel1.png como background
- [ ] Implementar parallax (velocidade 0.3x player)
- [ ] Implementar scroll infinito (tiling)
- [ ] Carregar plataformLevel1.png como chão
- [ ] Implementar GroundLevel global = viewport_height - 80
- [ ] Testar fullscreen responsiveness
- [ ] **Teste:** Background não distorce em fullscreen, parallax funciona
- [ ] **Commit:** `feat: implementa cenário responsivo e parallax`

### FASE 6: PROJECTILES & HAZARDS (Semana 2)
- [ ] Pigeon poop: Carregar pigeonPoop.png
- [ ] Implementar física: queda vertical com gravidade
- [ ] Implementar colisão com player (reduz velocidade 15%)
- [ ] Testar desaparecimento ao tocar chão
- [ ] Bus: Carregar bus.png (sem animação, sprite único)
- [ ] Implementar spawn fora da tela
- [ ] Implementar velocidade rápida (450+ px/s)
- [ ] **Teste:** Fezes caem fluidamente, ônibus passa rapidamente
- [ ] **Commit:** `fix: corrige física de fezes e ônibus`

### FASE 7: HAZARDS DIVERSOS (Semana 3)
- [ ] Hole: Carregar hole.png, verificar escala
- [ ] Crab: Carregar crab1.png, implementar movimento horizontal
- [ ] Shark: Carregar shark1.png, implementar para Level 2 oceânico
- [ ] Coconut: Carregar coconut.png, implementar coleta de pontos
- [ ] Jellyfish: Movimento vertical fluido
- [ ] **Teste:** Todos os hazards funcionando individualmente
- [ ] **Commit:** `feat: adiciona hazards diversos com animação`

### FASE 8: LEVEL 2 ASSETS (Semana 3)
- [ ] Carregar landscapeLevel2.png (praia)
- [ ] Carregar landscapeOceanlevel2.png (submarino)
- [ ] Carregar plataformLevel2.png (areia)
- [ ] Implementar sistema de switch de level automático
- [ ] Testar parallax em Level 2
- [ ] **Teste:** Trocar de level visualmente coerente
- [ ] **Commit:** `feat: adiciona assets de Level 2`

### FASE 9: POLISH & FULLSCREEN (Semana 3)
- [ ] Implementar HUD responsivo (não distorcer)
- [ ] Implementar câmera side-scrolling
- [ ] Testar fullscreen em múltiplas resoluções
- [ ] Debug visual: hitboxes, grid, render layers
- [ ] Testar performance (60 FPS em movimento)
- [ ] **Teste:** Tudo funciona em fullscreen 1920x1080, 1280x720, 1024x768
- [ ] **Commit:** `feat: adiciona responsividade e suporte a fullscreen`

### FASE 10: ADDITIONAL ASSETS (Semana 4)
- [ ] Umbrella: Carregar umbrella.png (pickup item)
- [ ] Farol (brenadFinal.png): Landmark visual no cenário
- [ ] Implementar "spawn points" customizáveis
- [ ] Testar rendering order (10 camadas)
- [ ] **Teste:** Todos os assets renderizando na ordem correta
- [ ] **Commit:** `feat: adiciona assets adicionais (umbrella, farol)`

---

## 🐛 BUGS CONHECIDOS & SOLUÇÕES

### ⚠️ CRÍTICO
| Bug | Asset | Impacto | Solução |
|-----|-------|--------|---------|
| Frame 2 faltando | `characterMovingL1/R1` | 🔴 Movimento travado | **Criar** `characterMovingL2/R2` |
| Rodas estáticas | `CharacterBikeMovingL/R` | 🔴 Bike parece parada | **Criar** frame 2 com rodas giradas |
| Sem chuva | Sistema partículas | 🟡 Falta ambiance | Implementar particle system |

### 🟡 IMPORTANTE
| Item | Prioridade | Ação |
|------|-----------|------|
| Escala hole.png | MÉDIA | Verificar proporção em-game vs Level 2 |
| DeltaTime em animações | ALTA | Garantir acoplamento com física |
| Fullscreen aspect ratio | ALTA | Testar em múltiplas resoluções |

---

## 📐 DIMENSÕES FINAIS (CONFIRMADAS)

```
PLAYER:
  Standing:  140x180px
  Moving:    150x180px
  On Bike:   200x150px

ENEMIES:
  Pigeon:    80x70px
  Jellyfish: 60x80px
  Crab:      120x100px
  Shark:     120x70px

HAZARDS:
  Bus:       250x100px
  Hole:      100x100px
  Poop:      20x25px
  Coconut:   50x50px

GROUND:
  Level 1:   1200x80px (asfalto + linhas)
  Level 2:   1200x60px (areia)

BACKGROUNDS:
  All:       1200x600px (scroll infinito)
```

---

## 🎬 FPS DE ANIMAÇÃO (FINALIZADOS)

```
Player Idle:       8 FPS   (sem movimento, parado)
Player Moving:    12 FPS   (corrida natural)
Pigeon:           12 FPS   (bater de asas)
Jellyfish:        10 FPS   (lento aquático)
Bike Wheels:      15 FPS   (rotação rápida)
Crab:              8 FPS   (caminhada lenta)
Shark:            12 FPS   (movimento rápido)
```

---

## 🔧 CONFIGURAÇÃO DE AMBIENTE

### Build & Compile
```bash
# Verificar SDL2 instalado
sdl2-config --cflags --libs

# Compilar com suporte a PNG
gcc -o game src/main.c src/gfx/animation.c -lSDL2 -lSDL2_image

# Testar com debug
CFLAGS="-DDEBUG -g" make
```

### Estrutura de Pastas Esperada
```
RecifeGame/
├── assets/
│   ├── img/                    (✅ 27 PNGs aqui)
│   ├── music/
│   └── sfx/
├── src/
│   ├── main.c
│   ├── entities/
│   ├── steps/
│   ├── gfx/
│   │   ├── animation.h         (👈 CRIAR)
│   │   └── animation.c         (👈 CRIAR)
│   ├── structure/
│   └── utils/
├── ASSETS_ANALYSIS.md          (✅ Criado)
├── ANIMATION_SYSTEM_SPEC.md    (✅ Criado)
└── ASSET_INTEGRATION_CHECKLIST (👈 Este arquivo)
```

---

## 📋 TESTES SUGERIDOS

### Teste 1: Carregamento de Assets
```c
// Verificar se todos os 27 PNGs carregam
printf("Loading %d assets...\n", 27);
assets_init();
printf("✓ Todos os assets carregados\n");
```

### Teste 2: Animação com DeltaTime
```c
AnimationSequence pigeon = 
  load_animation_sequence("pigeon", 'L', 12.0f, true);

for (float t = 0; t < 1.0f; t += 0.016f) { // ~60 FPS
  animation_sequence_update(&pigeon, 0.016f);
  printf("Frame: %d (t=%.2f)\n", pigeon.current_frame_index, t);
  // Esperado: alternância entre 0 e 1 em ~250ms
}
```

### Teste 3: Direção Automática
```c
// Simular movimento esquerda
player.velocity_x = -100;
player_animation_update(&player.anim, 0.016f);
assert(player.anim.direction == 'L');

// Simular movimento direita
player.velocity_x = 100;
player_animation_update(&player.anim, 0.016f);
assert(player.anim.direction == 'R');
```

### Teste 4: Fullscreen
```c
// Testar em resoluções:
// 1920x1080, 1280x720, 1024x768, 800x600
// Verificar: sem distorção, sem clipping, HUD alinhado
```

---

## 🚀 PRÓXIMOS PASSOS

### Imediato (Hoje)
1. ✅ Análise completa de assets
2. ✅ Especificação de animação
3. ⏳ Revisar documentação com times

### Curto Prazo (Esta semana)
1. Criar `src/gfx/animation.h` com estruturas
2. Implementar carregador automático de assets
3. Animar pigeon (teste simples)
4. Animar player (idle + moving)

### Médio Prazo (Próximas 2 semanas)
1. Todas as animações de entidades
2. Background + parallax
3. Fullscreen responsiveness

### Longo Prazo (Mês 1-2)
1. Polish visual (partículas, efeitos)
2. Otimização de performance
3. Release candidate

---

## 📞 DOCUMENTAÇÃO DE REFERÊNCIA

- **Análise de Assets:** `ASSETS_ANALYSIS.md`
- **Sistema de Animação:** `ANIMATION_SYSTEM_SPEC.md`
- **Este Checklist:** `ASSET_INTEGRATION_CHECKLIST.md`

---

## ✨ RESUMO FINAL

| Item | Quantidade | Status |
|------|-----------|--------|
| PNGs Analisados | 27 | ✅ |
| Animações Identificadas | 8 | ✅ |
| Frames de Animação | 16 | ✅ |
| Entidades com Assets | 12 | ✅ |
| Problemas Identificados | 3 | 🔴 |
| Documentação | 3 arquivos | ✅ |
| **Pronto para Implementação** | **SIM** | ✅ |

---

**Data de Análise:** 2026-05-20  
**Atualizado:** Conforme evolução do projeto  
**Status Global:** 🟢 PRONTO PARA DESENVOLVIMENTO
