# Registro de Alterações (ALTERACOES.md)

Este arquivo registra todas as modificações, correções e novas funcionalidades implementadas no projeto **Deixa Eu**.

---

## 🖥️ 1. Modo Tela Cheia (Fullscreen)
* **Status**: Implementado e Testado.
* **Descrição**: Adicionada a funcionalidade que permite ao jogador alternar entre o modo janela e o modo tela cheia (fullscreen) de maneira dinâmica.
* **Detalhes da Implementação**:
  - Inserido um detector de teclado no loop principal (`while (!WindowShouldClose())`) no arquivo [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c).
  - Pressionar a tecla **F** aciona a função nativa `ToggleFullscreen()` da Raylib.
* **Arquivos Modificados**:
  - [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)

---

## 🐛 2. Correção Geral de Erros de Compilação (fase3)
* **Status**: Resolvido.
* **Descrição**: Correção de múltiplos erros de sintaxe, escopo, referências nulas e constantes não declaradas que impediam a compilação do projeto.
* **Detalhes da Implementação**:
  - **[obstacle.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/obstacle.c)**:
    - Ajustado o acesso a propriedades de ponteiros usando `->` em vez de `.` para o parâmetro `Obstacle* obstacle` na função `updateObstacle`.
    - Substituída a variável indefinida `position` por `obstacle->position`.
    - Ajustado o acesso ao union para `obstacle->data.bus` em vez de `obstacle->data->bus`.
    - Removido o `case OBSTACLE_SHARK` duplicado da função `drawObstacle`.
  - **[raindrop.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/raindrop.c)**:
    - Substituído `LIGHTBLUE` (que não existe na Raylib) pela constante de cor padrão `BLUE`.
  - **[player.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.c)**:
    - Corrigido o warning substituindo a expressão de subtração `player.hasUmbrella - 0;` pela atribuição correta `player.hasUmbrella = 0;`.
  - **[stage1.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage1.h)**:
    - Adicionadas as definições das constantes `MAX_PIGEONS` e `MAX_RAINDROPS`.
    - Declarados todos os campos que estavam ausentes na estrutura `Stage1` (como `difficultyMultiplier`, `pigeons`, `raindrops`, `umbrellas`, etc.).
    - Alterado o tamanho do vetor `buildings` de `5` para `8` para evitar possíveis erros de *out-of-bounds* de memória durante a renderização (já que a fase tentava desenhar até 8 prédios).
    - Incluídos os arquivos de cabeçalho dos pombos, gotas de chuva e guarda-chuva.
    - Modificada a assinatura de `drawStage1` para receber o ponteiro de `Player` para desenhar o HUD adequadamente.
  - **[stage1.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage1.c)**:
    - Removida a linha incorreta que tentava alterar a propriedade inexistente `.hitbox` em `Pigeon` (a colisão é apenas com o objeto do tipo fezes).
    - Ajustada a assinatura de `drawStage1` para coincidir com a nova declaração.
  - **[main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)**:
    - Ajustada a chamada de `drawStage1(&stage)` para `drawStage1(&stage, &player)`.

---

## 🧗‍♂️ 3. Fase 3: A Torre de Cristal (Parque das Esculturas)
* **Status**: Implementado e Testado.
* **Descrição**: Inclusão de uma nova fase com mecânica dupla: progressão horizontal seguida por uma escalada vertical contínua na Torre de Cristal.
* **Detalhes da Implementação**:
  - **Criação de Assets Gráficos via IA**: Foram gerados e incluídos assets em Pixel Art para a Torre de Cristal, Nuvens e Pássaros.
  - **Nova Mecânica de Movimento (`player.h` e `player.c`)**: Adicionada a flag `isClimbing` ao struct `Player`. Quando verdadeira, a gravidade é desabilitada e o jogador pode se movimentar vertical e horizontalmente ao longo da superfície da torre.
  - **Desenvolvimento da Fase 3 (`stage3.h` e `stage3.c`)**:
    - **Fase Horizontal (Approach)**: O jogador corre para a direita enquanto o cenário (Torre) avança pela esquerda.
    - **Fase Vertical (Climbing)**: Ao colidir com a base da Torre de Cristal, a tela passa a rolar verticalmente enquanto o jogador sobe rumo ao topo.
    - Efeito de *Parallax Scrolling* infinito para nuvens e pássaros em diferentes velocidades.
  - **Integração (`main.c`)**: A fase 3 foi incluída nas importações e inicializada para carregar diretamente ao iniciar o jogo no menu para fins de teste rápidos.

---

## ☁️ 4. Ajustes das Nuvens e Pássaros (Fase 3)
* **Status**: Concluído.
* **Descrição**: Otimização estética e mecânica dos elementos de fundo.
* **Detalhes da Implementação**:
  * **Nuvens no Topo**: A geração vertical ($Y$) das nuvens foi redefinida entre as alturas `10` e `40` (garantindo que fiquem no céu e nunca encostem no chão).
  * **Tamanho Reduzido**: As escalas visuais foram reduzidas para `0.2` para as nuvens e `0.1` para os pássaros, melhorando a harmonia estética do cenário.
  * **Staggered Spawning**: Nuvens e pássaros são gerados e resetados com espaçamentos predefinidos para mantê-los bem distribuídos na tela.

---

## 🪶 5. Bando de Pássaros, Cocô com Física e Escalada Manual
* **Status**: Concluído.
* **Descrição**: Introdução de perigos aéreos de forma independente e refatoração da entrada na Torre.
* **Detalhes da Implementação**:
  * **Escalada Manual por Teclado**: O jogador agora pode caminhar na frente da torre e cruzá-la livremente. Ele é bloqueado apenas no canto direito da torre, e para começar a escalar, ele deve se posicionar em frente a ela e pressionar **W** ou **Seta para Cima**.
  * **Três Pássaros Ativos**: Elevado o limite de pássaros para `3`, com voos e velocidades independentes.
  * **Fezes dos Pássaros (Game Over)**: Cada pássaro possui seu próprio timer e intervalo randômico de cocô (`poopInterval`). Os dejetos caem com gravidades/velocidades verticais aleatórias (`150.0f` a `300.0f` px/s). Se o cocô atingir o hitbox do jogador, as vidas são zeradas instantaneamente (Derrota).
  * **Ajuste da Torre e Chão**: Eliminado o gap visual de 60px entre o chão verde e o pé da torre, afundando a torre em `20px` sob o plano do solo para garantir contato físico e perpendicular perfeito.

---

## 🔍 6. Visualização Expandida (Câmera 2D com Zoom-Out)
* **Status**: Concluído.
* **Descrição**: Implementação de uma câmera com zoom afastado para ver maior área do mapa e gameplay.
* **Detalhes da Implementação**:
  * **Camera2D da Raylib**: No desenho do jogo em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c), foi implementada uma `Camera2D` configurada com `zoom = 0.8f` e centralizada no meio da tela.
  * **Preenchimento do Chão**: Para cobrir a nova área exibida pela câmera, o chão verde foi expandido horizontalmente de `-200px` até `1000px`, e a verificação do limite vertical de reset foi ajustada para `560px`.
  * **HUD Fixo**: Toda a HUD de estatísticas (Vidas, Pontos, Tempo) é desenhada fora do modo de câmera, mantendo o texto nítido e estático no tamanho padrão da tela.

---

## 🎛️ 7. Correção da Navegação e Redirecionamento do Menu
* **Status**: Concluído.
* **Descrição**: Correção na lógica de transição e navegação dos botões do menu principal para evitar vazamento de inputs e comportamentos inesperados.
* **Detalhes da Implementação**:
  * **Isolamento de Estado**: Refatorada a função `updateMenu` em [menu.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/menu.c) para restringir as teclas de direção (`W/S`/`UP/DOWN`) exclusivamente à tela `MENU_MAIN`. Isso impede que o jogador altere as opções selecionadas em segundo plano enquanto visualiza a tela de Créditos.
  * **Retorno Preciso de Foco**: Ao fechar a tela de Créditos (seja com `Left/A`, `Escape` ou pressionando `Enter` na tela de créditos), o cursor retorna com precisão posicionado na própria opção "Créditos" (`menu->selectedOption = 1`), eliminando saltos indesejados.
  * **Limpeza de Código**: Removida a checagem redundante e duplicada de tecla `Escape` que ficava solta no arquivo [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c).

---

## 🚧 8. Remoção de Barreira Invisível e Scroll Lateral Fluido (Fase 3)
* **Status**: Concluído.
* **Descrição**: Correção da sensação de "barreira no meio do mapa" e expansão da área explorável após a Torre de Cristal.
* **Detalhes da Implementação**:
  * **Scroll com Fim de Curso**: O scroll automático de câmera (`scrollX`) da Fase 3 agora cessa completamente de forma inteligente quando atinge `650.0f` (ponto ideal em que a Torre está perfeitamente centralizada e visível a `550px` na tela).
  * **Movimentação Livre Pós-Scroll**: Uma vez que a câmera para de rolar, a restrição de `400.0f` no eixo X da tela é desabilitada. O jogador ganha liberdade total para caminhar livremente em frente à torre, atravessá-la e explorar o espaço extra.
  * **Barreira Inteligente Expandida**: Substituído o clamp de colisão antigo (que empurrava o jogador para trás conforme a torre se movia). Agora, a barreira física foi movida `150px` para a direita da Torre (`barrierX = stage->towerPosition.x + towerWidth + 150.0f`). O jogador pode caminhar no espaço extra expandido após a torre e é bloqueado de forma limpa e estática exatamente na borda direita visível da tela.

---

## 🖥️ 9. Ativação da Categoria Tela Cheia (Botão Nativo do OS)
* **Status**: Concluído.
* **Descrição**: Ativação do botão natural de maximizar e colocar em tela cheia da janela (botão verde no macOS) que se encontrava desabilitado.
* **Detalhes da Implementação**:
  * **Flags do Sistema**: Adicionada a diretiva `SetConfigFlags(FLAG_WINDOW_RESIZABLE)` antes da chamada de `InitWindow` no arquivo principal [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c).
  * **Resultado**: Habilitou perfeitamente o redimensionamento nativo e o botão verde padrão do macOS de colocar a aplicação em tela cheia/maximizar de forma completamente fluida e integrada ao sistema operacional.

---

## 🏆 10. Tela de Vitória Premium e Redirecionamento Livre de Memory Leak
* **Status**: Concluído.
* **Descrição**: Correção da falta de redirecionamento/softlock ao vencer o jogo e prevenção de vazamentos de textura.
* **Detalhes da Implementação**:
  * **Overlay de Vitória HUD**: Criada uma tela de vitória elegante em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c) quando o estado `STAGE3_FINISHED` é atingido. A renderização foi movida para fora do viewport da câmera 2D para manter os textos nítidos e centralizados em HSL/RGB em qualquer resolução de janela, com overlay translúcido e tom dourado.
  * **Redirecionamento Teclado (ENTER)**: Adicionada a detecção da tecla `ENTER` no estado de vitória para redefinir as variáveis do jogo, descarregar as texturas da Fase 3 e retornar o jogador ao Menu Principal com segurança.
  * **Eliminação de Leaks de Textura**: Removida a chamada redundante de `initStage3` do loop de reset automático do Game Over. As texturas agora são carregadas **exclusivamente** no clique de início de partida ("Iniciar Jogo"), liberando recursos antigos imediatamente através do `unloadStage3` sem duplicar instâncias de memória de vídeo.

---

## 🧭 11. Responsividade do Menu, Câmera da Fase 3 e Alinhamento da Torre
* **Status**: Concluído e Compilado.
* **Descrição**: Correção de problemas de redimensionamento da janela, enquadramento incorreto da Fase 3 em tela cheia e posicionamento visual da Torre de Cristal em relação ao chão.
* **Detalhes da Implementação**:
  * **Menu Principal Responsivo**: Refatorado o desenho do menu em [menu.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/menu.c) para centralizar título, subtítulo, opções e instruções com base em `GetScreenWidth()` e `GetScreenHeight()`. Isso evita que botões e textos desapareçam quando a janela fica menor.
  * **Escala Dinâmica do Menu**: Criada a função auxiliar `menuScale()` para adaptar tamanho de fonte e espaçamentos à resolução atual, preservando legibilidade tanto em janela reduzida quanto em tela cheia.
  * **Câmera Responsiva da Fase 3**: Substituído o zoom fixo `0.8f` por uma câmera 2D calculada dinamicamente em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c), usando o mundo lógico `800x450` e a dimensão real da janela para centralizar o jogo corretamente.
  * **HUD e Overlays Adaptados à Janela**: Ajustados textos, overlays de Game Over/Vitória, indicadores laterais e FPS para usarem `screenWidth` e `screenHeight`, evitando alinhamento preso às constantes antigas `SCREEN_WIDTH` e `SCREEN_HEIGHT`.
  * **Recorte Visível da Torre de Cristal**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), a torre passou a ser desenhada com `DrawTexturePro()` usando apenas a área visível do PNG (`234x973` dentro da textura `1024x1024`). Isso remove a influência da transparência lateral e vertical no posicionamento do sprite.
  * **Torre Fixada ao Solo Verde**: O cálculo da posição da torre agora usa `TOWER_BASE_Y` e `towerDrawHeight()`, fazendo a base visível da torre encostar corretamente na superfície verde onde o jogador anda.
  * **Hitbox Sincronizada com a Torre Visível**: Criada a função `syncTowerHitbox()` para manter a hitbox alinhada ao recorte visível da torre durante o movimento horizontal e a escalada vertical.
  * **Chão Expandido para a Câmera Responsiva**: A faixa verde do solo foi ampliada horizontal e verticalmente para cobrir toda a área exibida pela câmera em diferentes tamanhos de janela.
  * **Inicialização Correta do Player**: Em [player.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.c), `height` e `hitbox` passaram a ser inicializados em `createPlayer()`, evitando colisões inconsistentes ao iniciar ou reiniciar a fase.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) foi aberto com sucesso, carregando os assets da Fase 3 corretamente.
* **Arquivos Modificados**:
  - [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)
  - [menu.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/menu.c)
  - [player.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.c)
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🛠️ 12. Skill Local para Entendimento do Workspace
* **Status**: Criado.
* **Descrição**: Criação de uma skill local chamada `entenda-espaco` para orientar o Codex a mapear o diretório aberto, ler os principais arquivos e explicar a lógica do projeto antes de implementar alterações.
* **Detalhes da Implementação**:
  * **Nova Skill**: Criada a pasta [entenda-espaco](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/.agents/skills/entenda-espaco) dentro de `.agents/skills`.
  * **Gatilhos de Uso**: O arquivo `SKILL.md` descreve a ativação por frases como `entenda esse espaco` e por invocação explícita como `/entenda-espaco`.
  * **Fluxo de Análise**: A skill instrui a inspecionar a árvore do projeto, ler arquivos de configuração, entrypoints, fontes, testes e documentação, ignorando dependências ou artefatos gerados quando não forem relevantes.
  * **Metadados de Interface**: Criado o arquivo `agents/openai.yaml` com nome exibido, descrição curta e prompt padrão para uso da skill.
* **Arquivos Criados**:
  - [SKILL.md](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/.agents/skills/entenda-espaco/SKILL.md)
  - [openai.yaml](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/.agents/skills/entenda-espaco/agents/openai.yaml)

---

## 🌆 13. Fundo Pixelado, Piso Laranja e Movimento Contínuo da Fase 3
* **Status**: Concluído e Compilado.
* **Descrição**: Correção da sensação de barreira durante a aproximação da Torre de Cristal e substituição do fundo simples da fase por um cenário pixelado inspirado na imagem de referência, com piso laranja como superfície visual de caminhada.
* **Detalhes da Implementação**:
  * **Movimento Residual do Player**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), o jogador deixou de ficar rigidamente preso ao centro durante o scroll horizontal. Agora ele continua avançando dentro de uma faixa de movimento enquanto a torre se aproxima.
  * **Parallax Durante o Scroll**: Nuvens e pássaros passaram a receber deslocamento proporcional ao `scrollX`, evitando a sensação de elementos parados enquanto o jogador anda.
  * **Fundo Pixelado por Código**: Criadas funções auxiliares para desenhar céu noturno em degradê, lua/sol baixo, camadas de nuvens pixeladas e massas de nuvens ao fundo, aproximando a composição visual da imagem de referência sem depender de novo asset externo.
  * **Piso Laranja Pixelado**: O chão verde foi substituído por um piso em ladrilhos laranja/azul/dourado, desenhado em grade e com offset baseado no scroll para reforçar visualmente o deslocamento do personagem.
  * **Superfície de Caminhada**: A área inferior da tela agora funciona visualmente como o piso da fase, alinhada ao trecho onde o personagem se desloca e onde a torre encosta.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🧗 14. Escalada com Distanciamento do Piso e Encerramento dos Pássaros
* **Status**: Concluído e Compilado.
* **Descrição**: Ajustes finos na Fase 3 para melhorar a percepção de altura durante a escalada, remover travamentos visuais no piso durante a aproximação da torre e impedir geração de pássaros após a chegada na Torre de Cristal.
* **Detalhes da Implementação**:
  * **Céu com Movimento Vertical**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), o fundo passou a usar `scrollY` para deslocar nuvens, camadas do céu e astro luminoso durante a escalada, reforçando a sensação de subida.
  * **Piso se Distancia na Escalada**: O piso deixou de ter topo visual fixo e passou a ser desenhado com base em `floorY`, fazendo com que ele desça e se afaste mais conforme o jogador sobe a torre.
  * **Correção do Rollback do Piso**: O padrão dos ladrilhos agora usa uma coluna base derivada do scroll (`baseCol`) para manter as cores e detalhes estáveis durante o loop horizontal, evitando a impressão de rollback/travamento.
  * **Pássaros Desativados na Torre**: Adicionado controle de geração em [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h). Quando o jogador alcança a área da torre, novos obstáculos aéreos deixam de ser gerados e dejetos ativos são removidos.
  * **Sem Spawn Durante Escalada**: O spawn de fezes dos pássaros agora só ocorre enquanto a geração do ambiente está ativa, garantindo que a torre não gere novos obstáculos aéreos após a chegada do jogador.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🌧️ 15. Fundo em Fullscreen, Chuva, Nuvens PNG e Pós-Abate
* **Status**: Concluído e Compilado.
* **Descrição**: Novos ajustes na Fase 3 para adaptar o fundo ao modo tela cheia, corrigir retorno à esquerda, preservar elementos já existentes ao chegar na torre, adicionar chuva, restaurar o PNG das nuvens e bloquear o personagem após derrota.
* **Detalhes da Implementação**:
  * **Fundo Ajustado ao Fullscreen**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), foram criadas funções para calcular a área visível real da câmera (`visibleWorldWidth`, `visibleWorldHeight`, `visibleWorldLeft` e `visibleWorldTop`), fazendo o céu e o piso cobrirem a tela em diferentes proporções.
  * **Correção do Retorno à Esquerda**: A movimentação horizontal voltou a permitir que o jogador recue até a margem esquerda (`50px`) após avançar um pouco, evitando a sensação de barreira no retorno.
  * **Respawn Separado do Desenho**: O antigo controle `birdsEnabled` foi substituído por `ambientSpawningEnabled` em [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h). Ao chegar na torre, novos pássaros, nuvens e dejetos deixam de ser gerados, mas os elementos que já existem continuam visíveis e saem naturalmente da tela.
  * **Chuva no Fundo**: Adicionada a função `drawRain()` para desenhar chuva leve sobre o fundo e as nuvens, com deslocamento baseado no parallax e na escalada.
  * **Retorno do PNG das Nuvens**: As nuvens móveis voltaram a usar o asset [nuvem.png](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/assets/img/nuvem.png) quando disponível, mantendo o desenho pixelado apenas como fallback.
  * **Pôr do Sol na Escalada**: O céu agora interpola gradualmente de tons azulados para tons roxos/alaranjados conforme `scrollY` aumenta durante a subida da torre.
  * **Personagem Imóvel Após Derrota**: Em [player.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.c), `updatePlayer()` retorna imediatamente quando `lives <= 0`, zerando a velocidade e impedindo movimento após o abate.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h)
  - [player.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🌦️ 16. Chuva Animada e Mapa Horizontal Duplicado
* **Status**: Concluído e Compilado.
* **Descrição**: Animação da chuva do cenário e expansão da distância horizontal da Fase 3 para que o jogador precise correr aproximadamente duas vezes mais antes de alcançar a Torre de Cristal.
* **Detalhes da Implementação**:
  * **Chuva Animada**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), a função `drawRain()` passou a usar `GetTime()` para deslocar as gotas continuamente no eixo vertical, criando movimento real de queda.
  * **Chuva Diminuindo na Subida**: A intensidade da chuva agora é calculada por `rainIntensity = 1.0f - sunrise`, reduzindo a opacidade das gotas conforme o céu clareia durante a escalada.
  * **Nascer do Sol na Escalada**: O progresso visual da subida passou a usar a variável `sunrise`, reaproveitando `scrollY` para controlar tanto a transição de cor do céu quanto a redução da chuva.
  * **Mapa 2x Mais Longo**: Foram criadas as constantes `TOWER_START_X = 1850.0f` e `APPROACH_SCROLL_DISTANCE = 1300.0f`, duplicando o trecho horizontal antes da chegada à torre.
  * **Loop Visual do Piso e Fundo**: A expansão usa o scroll já existente do piso e do fundo para repetir visualmente o mapa enquanto o jogador percorre a nova distância maior.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 💧 17. Mapa 3x Maior e Poças Escorregadias
* **Status**: Concluído e Compilado.
* **Descrição**: Expansão do mapa horizontal da Fase 3 para três vezes a distância original, inclusão de poças no chão com fundo transparente e correção do respawn de elementos ao se afastar da Torre de Cristal.
* **Detalhes da Implementação**:
  * **Mapa 3x Maior**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), as constantes foram ajustadas para `TOWER_START_X = 2500.0f` e `APPROACH_SCROLL_DISTANCE = 1950.0f`, mantendo a torre no ponto visual correto de chegada enquanto triplica o trecho de corrida em relação ao mapa original.
  * **Poças no Chão**: Criado o tipo `Puddle` em [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h), com posição, hitbox e estado ativo. Sete poças foram distribuídas pelo trecho horizontal antes da Torre de Cristal.
  * **Imagem Sem Fundo Preto**: A poça foi implementada por desenho pixel-art em código (`drawPuddle()`), usando apenas retângulos azulados/translúcidos e sem desenhar qualquer fundo preto.
  * **Escorregão ao Colidir**: Ao detectar colisão entre o hitbox dos pés do jogador e uma poça, a fase ativa `slipTimer` e `slipVelocity`, fazendo o personagem deslizar brevemente na direção do movimento.
  * **Poças Acompanham o Scroll**: Durante o scroll horizontal, as poças são deslocadas junto com o mapa por `movePuddle()`, mantendo alinhamento com o piso.
  * **Respawn ao Voltar da Torre**: O respawn de pássaros e nuvens volta a ser ativado quando o jogador se afasta da área da torre, evitando que os elementos parem de aparecer permanentemente ao chegar e voltar.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🫧 18. Poças Aleatórias e Ambiente Persistente na Escalada
* **Status**: Concluído e Compilado.
* **Descrição**: Ajustes na Fase 3 para manter elementos ambientais visíveis durante a escalada e trocar o comportamento das poças de escorregão contínuo para travamento temporário com geração aleatória.
* **Detalhes da Implementação**:
  * **Poças Aleatórias**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), as poças deixaram de usar posições fixas e passaram a ser distribuídas por espaçamentos aleatórios com `randomPuddleSpacing()`.
  * **Respawn de Poças no Percurso**: Poças que saem pela esquerda podem ser reposicionadas à frente do jogador por `resetPuddleAhead()`, mantendo a presença aleatória delas ao longo do mapa.
  * **Travamento ao Passar na Poça**: O antigo `slipTimer/slipVelocity` foi substituído por `puddleLockTimer` em [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h). Ao pisar em uma poça, o jogador fica temporariamente impedido de andar.
  * **Sem Loop Infinito de Travamento**: Cada poça possui `canLockPlayer`, impedindo que o jogador seja travado repetidamente enquanto permanece sobre a mesma poça; a poça rearma quando ele sai de cima dela.
  * **Poças Durante a Escalada**: As poças agora continuam sendo desenhadas fora do estado `STAGE3_APPROACH` e acompanham o deslocamento vertical do piso durante a escalada.
  * **Ambiente Persistente Após Subir**: Durante `STAGE3_CLIMBING`, a geração do ambiente volta a ficar ativa, permitindo que nuvens e pássaros continuem aparecendo após o início da subida.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🐦 19. Movimento Independente dos Elementos Ambientais
* **Status**: Concluído e Compilado.
* **Descrição**: Desacoplamento de nuvens, pássaros e dejetos do movimento do personagem e do scroll do mapa, garantindo que cada elemento siga apenas seu próprio comportamento.
* **Detalhes da Implementação**:
  * **Nuvens Independentes**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), as nuvens deixaram de receber deslocamento horizontal baseado em `scrollDelta` e não são mais arrastadas pelo deslocamento vertical da escalada no momento do desenho.
  * **Pássaros Independentes**: Removido o deslocamento extra dos pássaros causado pelo scroll horizontal; eles agora se movem apenas pela própria velocidade (`bird.speed`) definida no update.
  * **Dejetos Independentes**: Os dejetos dos pássaros deixaram de ser deslocados verticalmente pelo scroll da escalada e passam a cair apenas por `speedY`.
  * **Separação de Responsabilidades**: Elementos ambientais móveis agora seguem movimento próprio, enquanto elementos presos ao mapa, como poças e torre, continuam acompanhando o piso/cenário.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 💦 20. Rotas Próprias, Empurrão da Poça e Piso Limpo
* **Status**: Concluído e Compilado.
* **Descrição**: Ajustes adicionais na Fase 3 para remover movimentos presos ao personagem, transformar a colisão com poças em um empurrão temporário e limpar a faixa clara do piso.
* **Detalhes da Implementação**:
  * **Fundo com Rota Própria**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), a chuva, as nuvens pixeladas decorativas e as faixas do céu deixaram de usar `scrollX`, `scrollY`, parallax ou deslocamento da escalada. Esses elementos agora se movem por tempo (`GetTime()`), seguindo rota própria.
  * **Chuva Independente do Player**: A função `drawRain()` foi simplificada para receber apenas a área visível e a intensidade, sem depender do movimento horizontal ou vertical do jogador.
  * **Empurrão ao Tocar na Poça**: Adicionado `puddlePushVelocity` em [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h). Ao encostar em uma poça, o jogador é lançado para frente e perde o controle por um curto período.
  * **Empurrão Acompanha o Mapa**: Quando o empurrão acontece na região de scroll, a torre e as poças também são reposicionadas pelo avanço do mapa, evitando travamento visual durante o efeito.
  * **Remoção da Barra Clara**: A faixa retangular desenhada no topo do piso foi removida de `drawStage3Floor()`, deixando apenas o mosaico do chão e sua base escura.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 💎 21. Torre Sem Poças e Aura de Cristal
* **Status**: Concluído e Compilado.
* **Descrição**: Remoção das poças da área da Torre de Cristal e melhoria visual da chegada com brilho, partículas e luz na base da torre.
* **Detalhes da Implementação**:
  * **Zona Livre de Poças**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), foi criada uma margem de segurança ao redor da Torre de Cristal (`TOWER_PUDDLE_CLEARANCE`) para impedir poças nessa região.
  * **Spawn Bloqueado na Torre**: As poças agora verificam `canPlacePuddle()` antes de nascer. Caso a posição fique dentro da área da torre, a poça é desativada em vez de aparecer no chão.
  * **Limpeza Durante o Scroll**: A função `clearPuddlesFromTowerArea()` remove qualquer poça que entre na região da torre por causa do avanço horizontal ou da transição para a escalada.
  * **Aura Visual da Torre**: Adicionada a função `drawTowerAura()` para desenhar halo azulado, feixe vertical, luz na base e pequenos brilhos animados ao redor da Torre de Cristal.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) foi atualizado corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🍾 22. Garrafas nas Poças, Player Centralizado e Piso Laranja
* **Status**: Concluído e Compilado.
* **Descrição**: Inclusão de garrafas de vidro após cada poça, centralização constante do jogador na tela e remoção do chão cinza/escuro para priorizar o piso laranja do fundo.
* **Detalhes da Implementação**:
  * **Garrafa Após Cada Poça**: Em [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h), cada `Puddle` passou a carregar `bottlePosition` e `bottleHitbox`. Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), a garrafa é posicionada sempre ao final da poça.
  * **Garrafa Sem Fundo Preto**: A garrafa foi desenhada como pixel art em código por `drawBottle()`, usando apenas retângulos/triângulos coloridos e sem renderizar qualquer fundo preto.
  * **Colisão Mortal**: Ao encostar no hitbox da garrafa, `player->lives` vai para `0`, acionando o Game Over.
  * **Espaço Válido Entre Obstáculos**: O espaçamento das poças foi recalculado considerando o conjunto poça + garrafa (`PUDDLE_CLUSTER_WIDTH`), deixando uma distância mínima antes do próximo conjunto para o salto ser possível.
  * **Player Sempre Centralizado**: A movimentação horizontal da aproximação agora mantém o jogador em `PLAYER_CENTER_X`; andar para direita/esquerda desloca o mapa, a torre, poças e garrafas, em vez de tirar o player do centro da tela.
  * **Torre Alinhada ao Centro**: O fim do scroll horizontal passou a ser calculado por `approachMaxScroll()`, posicionando a Torre de Cristal no centro do jogador quando ele chega nela.
  * **Piso Laranja como Chão**: O desenho do chão deixou de usar a base cinza/escura inferior e passou a usar uma base/tile set em tons laranja, fazendo a área caminhável bater melhor com o piso do fundo.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) inicializou corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🏃 23. Recuo à Esquerda, Pulo Maior e Poça Empurrando para a Garrafa
* **Status**: Concluído e Compilado.
* **Descrição**: Correção do bloqueio de movimento para a esquerda, aumento da altura do pulo e reforço do escorregão da poça na direção da garrafa.
* **Detalhes da Implementação**:
  * **Movimento à Esquerda Restaurado**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), o jogador deixou de ser travado no centro a cada frame. Agora ele pode recuar até um limite à esquerda da tela e, ao atingir esse limite, o mapa volta junto quando houver scroll disponível.
  * **Movimento à Direita Mantido**: O deslocamento para a direita ainda move o mapa quando o jogador passa da região central, preservando a aproximação da Torre de Cristal.
  * **Pulo Mais Alto**: Em [player.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.h), `PLAYER_JUMP_SPEED` foi aumentado de `420.0f` para `540.0f`.
  * **Escorregão Mais Forte da Poça**: Ao pisar na poça, `puddleLockTimer` e `puddlePushVelocity` foram aumentados para empurrar o jogador/mapa com mais força na direção da garrafa posicionada logo após a poça.
  * **Scroll Reaproveitado**: Criada a função auxiliar `applyHorizontalScroll()` para aplicar o mesmo deslocamento em torre, poças e garrafas tanto no movimento normal quanto no escorregão.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) foi atualizado corretamente.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [player.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🎥 24. Pulo Ampliado e Câmera Seguindo o Jogador
* **Status**: Concluído e Compilado.
* **Descrição**: Aumento da altura e do alcance horizontal do pulo, além da troca da câmera fixa por uma câmera que acompanha o centro do personagem.
* **Detalhes da Implementação**:
  * **Pulo Mais Alto**: Em [player.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.h), `PLAYER_JUMP_SPEED` foi aumentado de `540.0f` para `640.0f`.
  * **Pulo Mais Comprido**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), a velocidade horizontal da Fase 3 passou a ser maior enquanto o jogador está no ar (`315.0f`) do que no chão (`230.0f`), aumentando o alcance do salto.
  * **Câmera no Jogador**: Em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c), `camera.target` passou a usar o centro atual do player, mantendo o personagem no centro da tela conforme ele se movimenta e pula.
  * **Fundo Expandido para a Câmera**: O desenho do céu, chuva e piso foi ampliado em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c) para cobrir a viewport mesmo quando a câmera se desloca horizontal ou verticalmente com o jogador.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) foi atualizado corretamente.
* **Arquivos Modificados**:
  - [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [player.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/entities/player.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🌙 25. Câmera Alta, Espaço entre Poças e Fundo do Menu
* **Status**: Concluído e Compilado.
* **Descrição**: Ajuste da câmera para mirar acima do personagem, reforço da distância mínima entre poças, bloqueio de pulo durante escorregão e criação de fundo pixelado para o menu principal.
* **Detalhes da Implementação**:
  * **Câmera Acima do Player**: Em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c), foi adicionado `CAMERA_VERTICAL_LOOKAHEAD` para posicionar o alvo da câmera um pouco acima do centro do personagem, evitando que a tela fique visualmente rebaixada.
  * **Poças com Distância Mínima**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), foi criada a constante `PUDDLE_MIN_GAP` e o respawn passou a verificar todas as poças ativas antes de posicionar uma nova, evitando duas poças seguidas/coladas.
  * **Sem Pulo Durante Escorregão**: Enquanto `puddleLockTimer` está ativo, o jogador é marcado como sem contato com o chão, bloqueando o pulo enquanto está sendo levado pela poça.
  * **Fundo do Menu Principal**: Em [menu.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/menu.c), foi criado `drawMenuBackground()`, com céu em degradê, lua, nuvens pixeladas e piso laranja em mosaico.
  * **Validação**: O projeto foi recompilado com `make` e o binário [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu) foi atualizado corretamente.
* **Arquivos Modificados**:
  - [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)
  - [menu.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/menu.c)
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🎯 26. Câmera Ainda Mais Alta
* **Status**: Concluído e Compilado.
* **Descrição**: Ajuste fino da câmera da Fase 3 para centralizar a tela ainda mais acima do personagem.
* **Detalhes da Implementação**:
  * **Alvo Vertical Elevado**: Em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c), `CAMERA_VERTICAL_LOOKAHEAD` foi aumentado de `70.0f` para `110.0f`, fazendo a câmera mirar mais acima do centro do player.
  * **Validação**: O projeto foi recompilado com `make`.
* **Arquivos Modificados**:
  - [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🔎 27. Zoom Mais Aberto da Câmera
* **Status**: Concluído e Compilado.
* **Descrição**: Redução leve do zoom da câmera para mostrar mais área ao redor do personagem.
* **Detalhes da Implementação**:
  * **Zoom Reduzido**: Em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c), o `camera.zoom` agora usa `worldScale * 0.90f`, deixando a câmera um pouco mais afastada sem perder o acompanhamento do player.
  * **Validação**: O projeto foi recompilado com `make`.
* **Arquivos Modificados**:
  - [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🎯 28. Câmera Mais Alta com Zoom Aberto
* **Status**: Concluído e Compilado.
* **Descrição**: Novo ajuste fino para centralizar a câmera ainda mais acima do personagem.
* **Detalhes da Implementação**:
  * **Alvo Vertical Mais Alto**: Em [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c), `CAMERA_VERTICAL_LOOKAHEAD` foi aumentado de `110.0f` para `150.0f`.
  * **Zoom Mantido**: O zoom mais aberto (`worldScale * 0.90f`) foi preservado.
  * **Validação**: O projeto foi recompilado com `make`.
* **Arquivos Modificados**:
  - [main.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/main.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🧗 29. Escalada com Elementos Ancorados
* **Status**: Concluído e Compilado.
* **Descrição**: Ajuste da escalada da Torre de Cristal para manter os elementos do mapa ancorados na parte debaixo enquanto o personagem sobe.
* **Detalhes da Implementação**:
  * **Torre e Piso Fixos**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), a escalada deixou de deslocar `towerPosition.y`, evitando que a torre, o piso e os elementos do chão sejam reposicionados verticalmente.
  * **Poças e Garrafas Sem Subida**: Removido o deslocamento vertical das poças/garrafas durante a escalada; elas permanecem no mesmo ponto do chão.
  * **Céu Reage à Subida**: `scrollY` agora é calculado a partir da altura real do personagem na torre, mantendo a transição visual do céu durante a escalada sem arrastar os elementos do mapa.
  * **Final da Escalada por Altura do Player**: A conclusão da fase agora acontece quando o personagem alcança o topo visual da torre, em vez de depender da torre saindo da tela.
  * **Validação**: O projeto foi recompilado com `make`.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🌅 30. Nascer do Sol Corrigido e Dejetos Pixelados
* **Status**: Concluído e Compilado.
* **Descrição**: Correção do fundo durante a escalada da torre e substituição do cocô circular dos pássaros por sprites pixelados transparentes inspirados nas imagens anexadas.
* **Detalhes da Implementação**:
  * **Fundo da Escalada sem Falhas**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), o céu agora é desenhado com base na área real vista pela câmera do jogador, evitando espaços vazios ou cortes durante a subida da torre.
  * **Nascer do Sol Mais Consistente**: A progressão do nascer do sol passou a considerar também a altura atual do personagem na torre, mantendo a mudança de cores alinhada à escalada.
  * **Sprite Caindo**: O cocô do pássaro deixou de ser um círculo simples e agora é desenhado como um sprite pixelado transparente enquanto está caindo.
  * **Sprite no Chão**: Em [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h), `BirdPoop` recebeu os estados `landed` e `groundTimer`, permitindo trocar para o visual espalhado quando o objeto chega ao mesmo nível das poças.
  * **Colisão Atualizada**: Os hitboxes dos dejetos foram ajustados para diferenciar a queda e o estado no chão, mantendo o perigo coerente com o novo desenho.
  * **Validação**: O projeto foi recompilado com `make` sem warnings.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [stage3.h](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.h)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## ⚠️ 31. Ambiente Independente e Garrafa Mais Perigosa
* **Status**: Concluído e Compilado.
* **Descrição**: Ajuste dos dejetos dos pássaros, desacoplamento visual de elementos ambientais da câmera do personagem e redesign da garrafa de vidro.
* **Detalhes da Implementação**:
  * **Dejeto some rápido no chão**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), o tempo do dejeto no chão foi reduzido para `0.5f` segundo por meio de `POOP_LANDED_DURATION`.
  * **Elementos com Rota Própria**: Nuvens, pássaros e dejetos agora usam coordenadas locais da área visível e são convertidos para coordenadas de mundo apenas no desenho e na colisão, evitando que pareçam presos ao movimento do personagem.
  * **Colisão Mantida com a Câmera**: A hitbox dos dejetos também passa pela conversão de coordenadas, preservando o perigo mesmo com a câmera seguindo o player.
  * **Garrafa Mais Ameaçadora**: O desenho da garrafa recebeu cacos, brilho de vidro quebrado, rótulo vermelho e ícone triangular de alerta para comunicar melhor o perigo.
  * **Validação**: O projeto foi recompilado com `make` sem warnings.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🐦 32. Queda Fixa dos Dejetos dos Pássaros
* **Status**: Concluído e Compilado.
* **Descrição**: Correção da rota dos dejetos dos pássaros para impedir que acompanhem o movimento do personagem durante a tentativa de desvio.
* **Detalhes da Implementação**:
  * **Nascimento pela Posição do Pássaro**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), o dejeto ainda nasce a partir da posição visual atual do pássaro.
  * **Rota Própria no Mundo**: Depois de gerado, o dejeto passa a guardar coordenadas de mundo próprias, sem ser reconvertido pela posição atual da câmera ou do jogador.
  * **Desvio Corrigido**: O desenho e a hitbox dos dejetos agora usam essa posição fixa, permitindo que o jogador desvie ao andar para os lados sem que o obstáculo acompanhe o movimento.
  * **Validação**: O projeto foi recompilado com `make` sem warnings.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---

## 🪽 33. Rota Fixa dos Pássaros
* **Status**: Concluído e Compilado.
* **Descrição**: Correção do movimento dos pássaros para impedir que acompanhem o deslocamento do personagem.
* **Detalhes da Implementação**:
  * **Pássaros em Coordenadas do Mundo**: Em [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c), os pássaros deixaram de ser desenhados por conversão relativa à câmera do player e passaram a usar diretamente sua posição própria.
  * **Fluxo Individual Preservado**: Cada pássaro continua usando sua própria velocidade, posição e intervalo de dejeto, mantendo uma rota independente.
  * **Dejeto Nasce da Rota Real**: O cocô agora nasce a partir da posição fixa do pássaro no mundo, mantendo coerência entre pássaro e queda.
  * **Validação**: O projeto foi recompilado com `make` sem warnings.
* **Arquivos Modificados**:
  - [stage3.c](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/src/steps/stage3.c)
  - [deixaeu](file:///Users/godoy/Desktop/Projetos/DeixaEu/DeixaEu/build/deixaeu)

---
