# DungeonRPG — A Roguelike

A terminal-based ASCII dungeon crawler written in modern C++17.
No external dependencies — just a POSIX terminal.

 
## Features

- **Procedural dungeon generation** — every run is different
- **Turn-based combat** with crits, misses, and armor reduction
- **8-directional movement** (hjkl vi-keys or numpad) 
- **Inventory system** — weapons, armor, potions, scrolls, gold
- **Monster AI** — chase, wander, ranged, coward behaviours
- **Field of view** — shadow-casting, fog of war
- **10 dungeon floors** with scaling difficulty
- **Save / Load** system
- **XP and leveling** — gain power as you descend
- **Full ANSI color** rendering

## Build

```bash
# Requires g++ with C++17 support (Linux/macOS)
make
./dungeon
```
  
## Controls

| Key          | Action              |
|--------------|---------------------|
| `h j k l`   | Move (←↓↑→)        |
| `y u b n`   | Move diagonally     |
| `1-9` numpad| Move (8-dir)        |
| `g`          | Pick up item        |
| `i`          | Open inventory      |
| `>`          | Descend stairs      |
| `<`          | Ascend stairs       |
| `.`          | Wait one turn       |
| `s`          | Save game           |
| `?`          | Help screen         |
| `q`          | Quit                |

### In Inventory
| Key  | Action         |
|------|----------------|
| `a-z`| Select item    |
| `u`  | Use / drink    |
| `e`  | Equip item     |
| `d`  | Drop item      |
| ESC  | Close          |

## Goal

Descend through **10 floors** and retrieve the **Amulet of Yendor**!

## Project Structure

```
DungeonRPG/
├── main.cpp              — Entry point
├── Makefile
├── include/
│   ├── types.h           — Core data structures (Vec2, Tile, Item, Monster…)
│   ├── color.h           — ANSI color utilities + terminal raw mode
│   ├── dungeon.h         — Map / room / dungeon generator
│   ├── player.h          — Player, inventory, leveling
│   ├── combat.h          — Combat resolution
│   ├── renderer.h        — All rendering logic
│   └── game.h            — Main game loop / state machine
└── src/
    ├── terminal.cpp      — Raw terminal I/O (POSIX termios)
    ├── dungeon.cpp       — BSP room generation + FOV
    ├── player.cpp        — Player actions + serialization
    ├── combat.cpp        — Hit/miss/crit/damage math
    ├── renderer.cpp      — Map draw, panels, screens
    └── game.cpp          — Game loop, AI, spawning, save/load
```
