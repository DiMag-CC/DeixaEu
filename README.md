# 🚲🌧️ DEIXA EU 💥

**Deixa Eu** é um jogo em **C**, no estilo **2D Platform + Endless Runner**, inspirado nos clássicos *Super Mario* e *Subway Surfers*.

O jogador controla um jovem recifense que foge de casa após discutir com sua mãe e atravessa cenários icônicos do Recife enfrentando obstáculos urbanos, naturais e situações caóticas típicas da cidade.

O objetivo do jogo é completar todas as fases, superar os obstáculos e chegar ao topo do Parque das Esculturas para contemplar a paisagem do Recife e provar que:

> **“Valeu a pena, mãe.”**

---

## 🕹️ Gameplay

* Pressione ENTER para **iniciar** e Q para **sair** a qualquer momento.
* O jogador controla o personagem utilizando as teclas **W, A, S, D** ou setas do teclado.
* O jogo possui movimentação lateral, obstáculos dinâmicos e progressão contínua por fases.
* Cada fase apresenta desafios únicos inspirados no cotidiano recifense.
* Obstáculos aparecem dinamicamente utilizando estruturas de dados integradas à gameplay.
* O tempo total da partida será registrado em um sistema de ranking.

---

# 🗺️ Fases do Jogo

---

## 🚲 FASE 1 — PEDALANDO NO CAOS

O personagem foge pelas ruas chuvosas do Recife utilizando uma bicicleta Itaú laranja.

### 🚧 Obstáculos

* Buracos;
* Fezes de pombo;
* Ônibus;
* Chuva intensa.

### ⚡ Power-up

🌂 Guarda-chuva:

* reduz os efeitos da chuva;
* protege temporariamente contra lentidão.

### 🏁 Objetivo

Chegar até a Orla de Boa Viagem sem perder todas as vidas.

---

## 🦀🌊 FASE 2 — BOA VIAGEM

O jogador atravessa a praia de Boa Viagem correndo pela areia e posteriormente entrando no mar.

### 🚧 Obstáculos

* Caranguejos;
* Ondas;
* Água-viva;
* Tubarão;
* Correntes marítimas;
* Redes.

### ⚡ Power-up

🥥 Água de coco:

* aumenta velocidade;
* melhora desempenho temporariamente.

### 🏁 Objetivo

Escapar do tubarão e alcançar o Recife Antigo.

---

## 🗿 FASE 3 — PARQUE DAS ESCULTURAS

O jogador chega ao Parque das Esculturas Francisco Brennand e inicia a escalada final.

### 🚧 Obstáculos

* Ventania;
* Chuva pesada;
* Plataformas quebradiças;
* Partes escorregadias.

### 🏁 Objetivo

Alcançar o topo da escultura e finalizar a aventura.

---

# 🧠 Estruturas de Dados e Algoritmos

O projeto utiliza estruturas de dados diretamente integradas à lógica do jogo.

---

## 🔄 Lista Duplamente Encadeada Circular

Responsável pelo gerenciamento das fases do jogo.

```text
[Fase 1] <-> [Fase 2] <-> [Fase 3]
      ^                         |
      |_________________________|
```

### Aplicações

* Progressão entre fases;
* Reinício automático;
* Navegação contínua;
* Modo infinito.

---

## 📥 Filas Dinâmicas

Utilizadas para gerenciamento dos obstáculos.

### Aplicações

* Spawn de obstáculos;
* Controle de entidades na tela;
* Atualização dinâmica.

### Funcionamento

FIFO (*First In First Out*).

---

## 📊 Insertion Sort

Responsável pela ordenação do ranking de jogadores.

### Critérios

* Menor tempo;
* Maior pontuação;
* Menor número de colisões.

---

# 📁 Estrutura do Projeto

Abaixo está a estrutura principal utilizada no projeto:

```text
DeixaEu/
├── assets/
│   ├── img/
│   ├── music/
│   ├── sfx/
│   └── fonts/
│
├── src/
│   ├── main.c
│   │
│   ├── entities/
│   │   ├── player.c
│   │   ├── bike.c
│   │   ├── bus.c
│   │   ├── pigeon.c
│   │   ├── shark.c
│   │   └── obstacle.c
│   │
│   ├── steps/
│   │   ├── stage1.c
│   │   ├── stage2.c
│   │   └── stage3.c
│   │
│   ├── structure/
│   │   ├── obstacleQueue.c
│   │   └── stepList.c
│   │
│   ├── gfx/
│   │   ├── animation.c
│   │   └── sprite.c
│   │
│   └── utils/
│       ├── constants.h
│       └── utils.c
│
├── docs/
│   ├── estruturas_de_dados.md
│   ├── especificacao.md
│   └── apresentacao/
│
├── README.md
└── Makefile
```

---

## 👥 Time de Desenvolvimento

* **Arthur Moury**
* **Diego Magnata**
* **Luiza Barbosa**
* **Helio de Moraes**
* **Maria Augusta**

---

# ⚙️ Tecnologias

## Linguagem

* C

## Biblioteca gráfica

* Raylib

---

# ▶️ Executando o Jogo

## ⚠️ Pré-requisito importante

* Para compilar o jogo, você precisa ter o **Raylib** instalado no sistema.

Mais informações sobre instalação:
https://www.raylib.com/

---

## 🚀 Clonando o Repositório

No terminal:

```bash
git clone https://github.com/DiMag-CC/DeixaEu.git
```

Entre no diretório do projeto:

```bash
cd DeixaEu
```

---

## 🛠️ Compilação

### Compilar utilizando Makefile

```bash
make
```

---

### Compilação manual

```bash
gcc src/main.c \
src/entities/*.c \
src/steps/*.c \
src/structure/*.c \
src/utils/*.c \
src/gfx/*.c \
-I./src/entities \
-I./src/steps \
-I./src/structure \
-I./src/utils \
-I./src/gfx \
-o deixaeu \
$(pkg-config --cflags --libs raylib) \
-lm
```

---

## ▶️ Execução

Execute:

```bash
./deixaeu
```

Ou:

```bash
make run
```

---

# &#x20;🎮 Demonstração do Jogo

---

# 📝 Apresentação de Slides

Acesse os slides completos da apresentação do projeto:

---

  

# 🎨 Estilo Visual

* Pixel Art 2D;
* Atmosfera urbana recifense;
* Humor regional;
* Clima chuvoso;
* Referências culturais locais;
* Visual arcade cinematográfico.

---

# 🎵 Trilha Sonora

Mistura de:

* Frevo;
* Manguebeat;
* Percussão nordestina;
* Música urbana;
* Sons de chuva.

---

# 💡 Diferencial do Projeto

O diferencial técnico do projeto é que as estruturas de dados não são utilizadas apenas para armazenamento interno, mas fazem parte diretamente da gameplay.

As estruturas influenciam:

* progressão;
* movimentação;
* obstáculos;
* ranking;
* dinâmica do jogo.

---

# 🚀 Possíveis Expansões Futuras

* Multiplayer local;
* Ranking online;
* Mais fases inspiradas no Recife;
* Sistema de skins;
* Novos power-ups;
* IA para obstáculos;
* Sistema de missões;
* Boss fights urbanas.

---

# 🤝 Como Contribuir

Obrigado pelo interesse em contribuir com o **Deixa Eu**.

---

## 🛠️ Pré-requisitos

Antes de começar:

1. Instale o GCC;
2. Instale e configure o Raylib corretamente no sistema.

---

## 🚀 Guia de Desenvolvimento

### 1. Faça um Fork

Crie um fork do projeto no GitHub.

---

### 2. Clone seu fork

```bash
git clone https://github.com/SEU-USUARIO/DeixaEu.git
```

Entre no diretório:

```bash
cd DeixaEu
```

---

### 3. Crie uma branch

```bash
git checkout -b feature/minha-feature
```

---

### 4. Compile o projeto

```bash
make
```

---

### 5. Commit das alterações

```bash
git add .

git commit -m "Feat: adiciona nova funcionalidade"
```

---

### 6. Push para o GitHub

```bash
git push origin feature/minha-feature
```

---

### 7. Abra um Pull Request

1. Acesse seu fork no GitHub;
2. Clique em **Compare & pull request**;
3. Explique claramente suas alterações;
4. Clique em **Create pull request**.

---

# 🙏 Agradecimento

Muito obrigado por acompanhar e apoiar o projeto **Deixa Eu**.

Esse jogo é uma homenagem divertida ao caos, à cultura e à identidade do Recife, misturando gameplay arcade, humor regional e estruturas de dados aplicadas na prática.

E no final de toda aventura:

> ❤️ **“Valeu a pena, mãe.”**
