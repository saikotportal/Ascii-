#pragma once
#include <string>
#include <vector>

// ─── Vec2 ─────────────────────────────────────────────────────────────────────
struct Vec2 {
    int x = 0, y = 0;
    Vec2() = default;
    Vec2(int x, int y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }
};

// ─── Tile ─────────────────────────────────────────────────────────────────────
enum class TileType {
    WALL, FLOOR, DOOR_CLOSED, DOOR_OPEN,
    STAIRS_DOWN, STAIRS_UP, CHEST, TRAP
};

struct Tile {
    TileType type     = TileType::WALL;
    bool     visible  = false;
    bool     explored = false;
    char     glyph    = '#';
    bool     passable = false;
    bool     hasItem  = false;
};

// ─── Stats ────────────────────────────────────────────────────────────────────
struct Stats {
    int hp    = 30, maxHp   = 30;
    int mana  = 15, maxMana = 15;
    int atk   = 6;
    int def   = 2;
    int spd   = 10;
    int level = 1;
    int xp    = 0, xpNext = 100;
    int gold  = 0;
};

// ─── Item ─────────────────────────────────────────────────────────────────────
enum class ItemType { WEAPON, ARMOR, POTION, SCROLL, FOOD, GOLD_PILE };

struct Item {
    std::string name;
    std::string desc;
    ItemType    type       = ItemType::POTION;
    char        glyph      = '?';
    int         value      = 10;
    int         atkBonus   = 0;
    int         defBonus   = 0;
    int         hpRestore  = 0;
    int         qty        = 1;
    bool        equipped   = false;
};

// ─── Monster ──────────────────────────────────────────────────────────────────
enum class AIType { WANDER, CHASE, RANGED, COWARD };

struct Monster {
    std::string name;
    char        glyph  = 'M';
    Vec2        pos;
    int         hp     = 10, maxHp = 10;
    int         atk    = 3,  def   = 0;
    int         xpDrop = 20, goldDrop = 5;
    AIType      ai     = AIType::CHASE;
    bool        alive  = true;
    int         turnCooldown = 0;
};

// ─── Log Entry ────────────────────────────────────────────────────────────────
struct LogEntry {
    std::string text;
    std::string colorCode; // ANSI escape
};

// ─── Direction helpers ────────────────────────────────────────────────────────
inline Vec2 dirFromChar(char c) {
    switch(c) {
        case 'h': case '4': return {-1,  0};
        case 'l': case '6': return { 1,  0};
        case 'k': case '8': return { 0, -1};
        case 'j': case '2': return { 0,  1};
        case 'y': case '7': return {-1, -1};
        case 'u': case '9': return { 1, -1};
        case 'b': case '1': return {-1,  1};
        case 'n': case '3': return { 1,  1};
        default:            return { 0,  0};
    }
}
