#pragma once
#include "dungeon.h"
#include "player.h"
#include "combat.h"
#include "renderer.h"
#include "types.h"
#include <vector>
#include <deque>
#include <string>
#include <random>

enum class GameState { MENU, PLAYING, INVENTORY, DEAD, WIN };

class Game {
public:
    Game();
    ~Game();

    void run();   // main loop

private:
    // ── Core objects ──────────────────────────────────────────────────────────
    DungeonMap            map_;
    Player                player_;
    std::vector<Monster>  monsters_;
    std::vector<Item>     floorItems_;   // items lying on ground
    Renderer              renderer_;
    std::unique_ptr<Combat> combat_;
    std::deque<LogEntry>  log_;
    GameState             state_    = GameState::MENU;
    int                   floor_    = 1;
    int                   turnCount_= 0;
    int                   invSel_   = 0;   // inventory cursor
    std::mt19937          rng_;

    // ── Initialization ────────────────────────────────────────────────────────
    void newGame();
    void descend();           // go to next floor
    void ascend();            // go to previous floor (if stairs up)

    // ── Per-turn logic ────────────────────────────────────────────────────────
    void handleInput(char c);
    void updateMonsters();    // AI step for all monsters
    void handlePlayerMove(Vec2 delta);
    void handleStairs(char c);
    void handlePickup();
    void handleInventoryInput(char c);
    void handleRestCommand();

    // ── Monster helpers ───────────────────────────────────────────────────────
    void       spawnMonsters();
    void       spawnItems();
    Monster*   monsterAt(Vec2 pos);
    void       removeDeadMonsters();

    // ── Item helpers ──────────────────────────────────────────────────────────
    Item       randomItem(int floorLevel);
    Monster    randomMonster(int floorLevel);

    // ── Logging ───────────────────────────────────────────────────────────────
    void addLog(const std::string& text, const std::string& color = "\033[37m");

    // ── Save / Load ───────────────────────────────────────────────────────────
    bool saveGame(const std::string& filename = "save.dat");
    bool loadGame(const std::string& filename = "save.dat");

    // ── Helpers ───────────────────────────────────────────────────────────────
    bool isOccupied(Vec2 pos) const;
    bool hasLineOfSight(Vec2 a, Vec2 b) const;
};
