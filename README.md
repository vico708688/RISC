# Émulateur RISC-V

## 1. Objectifs du projet

### But principal
Développer un émulateur mini RISC-V 32 bits capable d'exécuter des programmes compilés.

### But final
Faire tourner le jeu Doom via la bibliothèque doomgeneric

### Fonctionnalités

#### Emulateur RISC-V complet

* Support des instructions de base
* Support des extensions de multiplication/division/...

#### Environnement d'exécution

* Implémentation des syscalls POSIX (open/read/write/...)
* 32MB de RAM (64 pour Doom)
* Utilisation d'un fichier .wad Doom stocké en mémoire


## 2. Avancement

### Emulateur RISC-V

* Décodage et exécution de la majorité des instructions
* Instructions arithmétiques et logiques (ADD, SUB, AND, OR, XOR, ...)
* Instructions de multiplication/division
* Branchements conditionnels (BEQ, BNE, BLT, BGE, BLTU, BGEU)
* Sauts (JAL, JALR)
* Lecture/ecriture mémoire (LB, LH, LW, LBU, LHU, SB, SH, SW)
* Instructions de décalage (SLL, SRL, SRA, SLLI, SRLI, SRAI)
* Instructions LUI et AUIPC
* Gestion de 32 MB de RAM avec vérification des limites

### Plateforme d'exécution

* Allocation dynamique de mémoire
* Sortie console via le périphérique charOut à `0x10000000`
* Chargement de binaires ELF
* WAD Doom (doom1.wad) embarqué et accessible via file descriptor

### Syscalls implémentés
* `_write()` : sortie console
* `_read()` : lecture du WAD embarqué
* `_open()` / `_close()` : gestion du fichier doom1.wad
* `_sbrk()` : allocation dynamique avec protection heap/stack
* `_gettimeofday()` : timer basé sur le compteur d'instructions
* `_fstat()`, `_lseek()` : support minimal des opérations fichier

### Problèmes

* **Cause** : Opcode inconnu `0x50` détecté à l'adresse `0x800626a8`

### A implémenter

* Affichage graphique
* Gestion des entrées clavier
* Timer simplifié (basé sur compteur d'instructions, pas du vrai temps réel pour l'instant)

## 3. Compilation et exécution

### Prérequis

* Compilation croisée RISC-V : riscv32-unknown-elf-gcc
* GCC
* Make

### Structure du projet

```
RISC/
|
├─ emulator/
│  ├─ source/
│  │  ├─ main.c
│  │  ├─ minirisc.c
│  │  └─ platform.c
│  ├─ include/
│  │  ├─ minirisc.h
│  │  ├─ platform.h
│  │  └─ types.h
│  └─ build/
|
├─ embedded_software_doom/
│  ├─ doomgeneric/
│  ├─ doomgeneric_minirisc.c
│  ├─ syscalls.c
│  ├─ doom1_wad.c
|  ├─ Makefile
│  ├─ minirisc.ld
│  └─ build/
|
├─ embedded_software/
|  ├─ build/
|  ├─ main.c
|  ├─ Makefile
|  ├─ minirisc_init.S
|  ├─ minirisc.ld
|  └─ syscalls.c
|
└── Makefile
```

### Compilation de l'émulateur

```
# Compiler
make

# Nettoyer
make clean
```

### Compilation du code embarqué Doom (non fonctionnel pour l'instant)

```
# Compiler
make -C embedded_software_doom

# Nettoyer
make -C embedded_software_doom clean
```

### Compilation du code embarqué de test

```
# Compiler
make -C embedded_software

# Nettoyer
make -C embedded_software clean
```

### Configuration du main.c

Changer le chemin d'accès de la ligne 19 :

```platform_load_program(platform, "path/to/esw.bin");```

### Exécution

```
make exec
```

### Sortie attendue du test

```
make exec
./emulator/build/emulator
Creating platform...
Creating minirisc...
Loading program...
Starting VM...
First instruction at RAM_BASE: 02000102
Stack top should be at: 83fffff0
Hello, World!, from put_string()
Hello, World!, we are in main.
PI: 3.14159

---------------------

VM STOPPED
```

## 4. Ressources utilisées

* DoomGeneric : https://github.com/ozkl/doomgeneric
* Make : https://www.gnu.org/software/make/manual/make.html