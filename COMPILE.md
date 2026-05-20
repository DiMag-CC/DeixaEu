# 🔧 GUIA DE COMPILAÇÃO - DEIXA EU

## Requisitos

- **GCC** ou compatível (MinGW no Windows, gcc no Linux/WSL)
- **Raylib 5.5** instalado no sistema
- **Make** (opcional, mas recomendado)

## Instalação de Dependências

### Linux/WSL (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential libraylib-dev
```

### macOS
```bash
brew install raylib
```

### Windows (MSYS2/MinGW)
```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-raylib
```

## Compilação

### Opção 1: Com Make (Recomendado)
```bash
cd RecifeGame
make          # Compila
make run      # Executa
make clean    # Limpa build/
```

### Opção 2: Manualmente (Linux/WSL)
```bash
cd RecifeGame
gcc -Wall -Wextra \
  $(pkg-config --cflags raylib) \
  src/main.c src/menu.c \
  src/entities/*.c \
  src/steps/*.c \
  src/structure/*.c \
  src/utils/*.c \
  src/gfx/*.c \
  -o build/deixaeu \
  $(pkg-config --libs raylib) -lm

./build/deixaeu
```

### Opção 3: Manualmente (Windows PowerShell)
```powershell
$files = @(
  "src\main.c",
  "src\menu.c",
  "src\entities\*.c",
  "src\steps\*.c",
  "src\structure\*.c",
  "src\utils\*.c",
  "src\gfx\*.c"
)

gcc -Wall -Wextra -Ibuild $files -o build\deixaeu -lraylib -lm
.\build\deixaeu
```

## Estrutura de Build

```
build/
├── deixaeu (executável)
├── src/
│   ├── main.o
│   ├── menu.o
│   ├── entities/
│   ├── steps/
│   ├── structure/
│   ├── utils/
│   └── gfx/
```

## Flags de Compilação

- `-Wall -Wextra`: Avisos completos
- `-I/usr/local/include`: Includes do Raylib
- `-lraylib`: Link com biblioteca Raylib
- `-lm`: Link com libmath

## Troubleshooting

### Erro: "raylib.h: No such file or directory"
**Solução:** Instale raylib-dev ou adicione `-I/path/to/raylib/include`

### Erro: "cannot find -lraylib"
**Solução:** Adicione `-L/path/to/raylib/lib` antes de `-lraylib`

### Erro: Undefined reference to...
**Solução:** Certifique-se de compilar TODOS os .c em src/ e seus subdiretórios

## Verificação de Compilação

Se compilar com sucesso, você verá:
```
gcc ... -o build/deixaeu
(sem erros)

./build/deixaeu
Fase atual: Recife Chuvoso (numero 1)
```

## Versão do Raylib

Para verificar se Raylib 5.5 está instalado:
```bash
pkg-config --modversion raylib
# Deve retornar: 5.5
```
