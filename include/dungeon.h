#pragma once
#include "types.h"
#include <vector>
#include <random>

// ─── Room ─────────────────────────────────────────────────────────────────────
struct Room {
    int x, y, w, h;
    Vec2 center() const { return {x + w / 2, y + h / 2}; }
    bool intersects(const Room& o, int margin = 1) const {
        return x - margin < o.x + o.w &&
               x + w + margin > o.x   &&
               y - margin < o.y + o.h &&
               y + h + margin > o.y;
    }
};

// ─── Map ──────────────────────────────────────────────────────────────────────
class DungeonMap {
public:
    static const int WIDTH  = 80;
    static const int HEIGHT = 40;

    DungeonMap();

    // Generation
    void generate(int floor, unsigned int seed = 0);

    // Tile access
    Tile&       at(int x, int y);
    const Tile& at(int x, int y) const;
    Tile&       at(Vec2 v)       { return at(v.x, v.y); }
    const Tile& at(Vec2 v) const { return at(v.x, v.y); }
    bool        inBounds(int x, int y) const;
    bool        inBounds(Vec2 v) const { return inBounds(v.x, v.y); }

    // Visibility (simple raycasting FOV)
    void computeFOV(Vec2 origin, int radius);
    void resetVisibility();

    // Accessors
    const std::vector<Room>& rooms()      const { return rooms_; }
    Vec2                     stairsDown() const { return stairsDown_; }
    Vec2                     stairsUp()   const { return stairsUp_; }
    int                      floor()      const { return floor_; }

private:
    std::vector<Tile> tiles_;
    std::vector<Room> rooms_;
    Vec2              stairsDown_, stairsUp_;
    int               floor_ = 1;
    std::mt19937      rng_;

    void fillWalls();
    void carveRoom(const Room& r);
    void carveCorridor(Vec2 a, Vec2 b);
    void setTile(int x, int y, TileType t);
    void castRay(Vec2 origin, Vec2 dir, int radius);
    bool lineOfSight(Vec2 a, Vec2 b) const;
};
