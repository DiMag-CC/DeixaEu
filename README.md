# 🚲🌧️ DEIXA EU 💥

**Deixa Eu** é um jogo em **C**, no estilo **2D Platform + Endless Runner**, inspirado nos clássicos *Super Mario* e *Subway Surfers*.

O jogador controla um jovem recifense que foge de casa após discutir com sua mãe e atravessa cenários icônicos do Recife enfrentando obstáculos urbanos, naturais e situações caóticas típicas da cidade.

O objetivo do jogo é completar todas as fases, superar os obstáculos e chegar ao topo do Parque das Esculturas para contemplar a paisagem do Recife e provar que:

> **“Valeu a pena, mãe.”**

---

# 🕹️ Gameplay

* Pressione ENTER para iniciar e Q para sair a qualquer momento.
* O jogador controla o personagem usando as teclas **W, A, S, D** ou setas do teclado.
* O jogo possui movimentação lateral, obstáculos dinâmicos e progressão por fases.
* Cada fase possui desafios únicos inspirados na cultura e no cotidiano recifense.
* O tempo total da partida será registrado em um sistema de ranking.
* Obstáculos aparecem dinamicamente utilizando estruturas de dados integradas à gameplay.

---

# 🗺️ Fases do Jogo

---

## 🚲 FASE 1 — PEDALANDO NO CAOS

O personagem foge pelas ruas chuvosas do Recife utilizando uma bicicleta Itaú laranja.

### Obstáculos

* Buracos;
* Fezes de pombo;
* Ônibus;
* Chuva intensa.

### **Power Up**

Guarda-chuva:

* evita damage da chuva;

### **Objetivo**

Chegar até a Orla de Boa Viagem sem colidir.

---

## 🦀🌊 FASE 2 — BOA VIAGEM

O jogador atravessa a praia de Boa Viagem correndo pela areia e posteriormente entrando no mar.

### Obstáculos

* Caranguejos;
* Ondas;
* Água-viva;
* Tubarão.

### Power-up

Água de coco:

* aumenta velocidade;
* melhora desempenho temporariamente.

### Objetivo

Escapar do tubarão e alcançar o Recife Antigo.

---

## 🗿 FASE 3 — PARQUE DAS ESCULTURAS

O jogador chega ao Parque das Esculturas Francisco Brennand e inicia a escalada final.

### Obstáculos

* Ventania;
* Chuva;
* Partes escorregadias;

### Objetivo

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
* Controle de objetos na tela;
* Atualização dinâmica.

### Funcionamento

FIFO (First In First Out).

---

## 📊 Insertion Sort

Responsável pela ordenação do ranking de jogadores.

### Critérios

* Menor tempo;
* Maior pontuação;
* Menor número de colisões.

---

# 📁 Estrutura do Projeto

```text
DeixaEu/
├── assets/
│   ├── sprites/
│   ├── audio/
│   ├── backgrounds/
│   └── fonts/
│
├── src/
│   ├── main.c
│   ├── jogo.c
│   ├── player.c
│   ├── fases.c
│   ├── lista_fases.c
│   ├── fila_obstaculos.c
│   ├── ranking.c
│   ├── colisao.c
│   └── interface.c
│
├── docs/
│   ├── especificacao.md
│   ├── estruturas_de_dados.md
│   └── apresentacao/
│
├── README.md
└── Makefile
```

---

# ⚙️ Tecnologias

## Linguagem

* C

## Biblioteca gráfica

* Raylib

---

# ▶️ Executando o Jogo

## ⚠️ Pré-requisitos

* GCC instalado;
* Raylib instalada e configurada no sistema.

Mais informações:
https://www.raylib.com/

---

# 🚀 Clonando o Repositório

```bash
git clone https://github.com/DiMag-CC/DeixaEu.git
```

Entre na pasta do projeto:

```bash
cd DeixaEu
```

---

# 🛠️ Compilação

## Entre na pasta src/

```bash
cd src
```

## Compile o projeto

```bash
gcc -o jogo *.c -lraylib -lm
```

---

# ▶️ Execução

Volte para a pasta raiz:

```bash
cd ..
```

Execute o jogo:

```bash
./src/jogo
```

---

# 🎨 Estilo Visual

* Pixel Art 2D;
* Atmosfera urbana recifense;
* Humor regional;
* Clima chuvoso;
* Referências culturais locais.

---

# 🎵 Trilha Sonora

Mistura de:

* Frevo;
* Manguebeat;
* Percussão nordestina;
* Música urbana;
* Sons de chuva.

---

# 👥 Time de Desenvolvimento

* **Arthur Moury**
* **Diego Magnata**
* **Luiza Barbosa**
* **Helio de Moraes**
* **Maria Augusta**

---

# 💡 Diferencial do Projeto

O diferencial técnico do projeto é que as estruturas de dados não são utilizadas apenas para armazenamento, mas fazem parte diretamente da gameplay.

As estruturas influenciam:

* progressão;
* obstáculos;
* movimentação;
* ranking;
* fluxo do jogo.

---

# 🚀 Possíveis Expansões Futuras

* Multiplayer local;
* Ranking online;
* Mais fases inspiradas no Recife;
* Sistema de skins;
* Novos power-ups;
* IA para obstáculos;
* Sistema de missões.

---

# 🤝 Como Contribuir

1. Faça um fork do projeto;
2. Crie uma branch:

```bash
git checkout -b feature/minha-feature
```

3. Faça suas alterações;
4. Commit:

```bash
git commit -m "Feat: adiciona nova funcionalidade"
```

5. Push da branch:

```bash
git push origin feature/minha-feature
```

6. Abra um Pull Request.

---

# 🙏 Agradecimento

Muito obrigado por acompanhar e apoiar o projeto **Deixa Eu**.

Esse jogo é uma homenagem divertida ao caos, à cultura e à identidade do Recife, misturando humor regional, gameplay arcade e estruturas de dados aplicadas na prática.

E no final de toda aventura:

>  ❤️  **“Valeu a pena, mãe.”**
