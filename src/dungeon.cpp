#include "../include/dungeon.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// ─── Constructor ──────────────────────────────────────────────────────────────
DungeonMap::DungeonMap() : tiles_(WIDTH * HEIGHT) {}

// ─── Tile Access ──────────────────────────────────────────────────────────────
bool DungeonMap::inBounds(int x, int y) const {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}
Tile& DungeonMap::at(int x, int y) {
    return tiles_[y * WIDTH + x];
}
const Tile& DungeonMap::at(int x, int y) const {
    return tiles_[y * WIDTH + x];
}

// ─── Set a tile's properties ──────────────────────────────────────────────────
void DungeonMap::setTile(int x, int y, TileType t) {
    if (!inBounds(x, y)) return;
    auto& tile = at(x, y);
    tile.type = t;
    switch (t) {
        case TileType::WALL:
            tile.glyph = '#'; tile.passable = false; tile.transparent = false; break;
        case TileType::FLOOR:
            tile.glyph = '.'; tile.passable = true;  tile.transparent = true;  break;
        case TileType::DOOR_CLOSED:
            tile.glyph = '+'; tile.passable = false; tile.transparent = false; break;
        case TileType::DOOR_OPEN:
            tile.glyph = '/'; tile.passable = true;  tile.transparent = true;  break;
        case TileType::STAIRS_DOWN:
            tile.glyph = '>'; tile.passable = true;  tile.transparent = true;  break;
        case TileType::STAIRS_UP:
            tile.glyph = '<'; tile.passable = true;  tile.transparent = true;  break;
        case TileType::CHEST:
            tile.glyph = '$'; tile.passable = true;  tile.transparent = true;  break;
        case TileType::TRAP:
            tile.glyph = '^'; tile.passable = true;  tile.transparent = true;  break;
        default:
            tile.glyph = ' '; tile.passable = false; tile.transparent = false; break;
    }
}

// ─── Fill all with walls ──────────────────────────────────────────────────────
void DungeonMap::fillWalls() {
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            setTile(x, y, TileType::WALL);
}

// ─── Carve a rectangular room ─────────────────────────────────────────────────
void DungeonMap::carveRoom(const Room& r) {
    for (int y = r.y; y < r.y + r.h; ++y)
        for (int x = r.x; x < r.x + r.w; ++x)
            setTile(x, y, TileType::FLOOR);
}

// ─── Carve L-shaped corridor ──────────────────────────────────────────────────
void DungeonMap::carveCorridor(Vec2 a, Vec2 b) {
    // horizontal then vertical
    int xDir = (b.x > a.x) ? 1 : -1;
    int yDir = (b.y > a.y) ? 1 : -1;
    for (int x = a.x; x != b.x; x += xDir)
        setTile(x, a.y, TileType::FLOOR);
    for (int y = a.y; y != b.y + yDir; y += yDir)
        setTile(b.x, y, TileType::FLOOR);
}

// ─── Main Generator ───────────────────────────────────────────────────────────
void DungeonMap::generate(int floor, unsigned int seed) {
    floor_ = floor;
    if (seed == 0) {
        std::random_device rd;
        rng_.seed(rd());
    } else {
        rng_.seed(seed);
    }

    fillWalls();
    rooms_.clear();

    auto randInt = [&](int lo, int hi) -> int {
        return std::uniform_int_distribution<int>(lo, hi)(rng_);
    };

    const int MAX_ROOMS  = 12 + floor * 2;
    const int MIN_SIZE   = 4;
    const int MAX_SIZE   = 10;
    const int ATTEMPTS   = 200;

    for (int attempt = 0; attempt < ATTEMPTS && (int)rooms_.size() < MAX_ROOMS; ++attempt) {
        int rw = randInt(MIN_SIZE, MAX_SIZE);
        int rh = randInt(MIN_SIZE, MAX_SIZE / 2 + 2);
        int rx = randInt(1, WIDTH  - rw - 2);
        int ry = randInt(1, HEIGHT - rh - 2);
        Room nr{rx, ry, rw, rh};

        bool overlap = false;
        for (auto& existing : rooms_)
            if (nr.intersects(existing, 2)) { overlap = true; break; }

        if (!overlap) {
            carveRoom(nr);
            if (!rooms_.empty())
                carveCorridor(rooms_.back().center(), nr.center());
            rooms_.push_back(nr);
        }
    }

    // Place doors on corridor-room borders
    for (auto& room : rooms_) {
        for (int x = room.x - 1; x <= room.x + room.w; ++x) {
            for (int y : {room.y - 1, room.y + room.h}) {
                if (inBounds(x, y) && at(x, y).type == TileType::FLOOR) {
                    if (randInt(0, 3) == 0) setTile(x, y, TileType::DOOR_CLOSED);
                }
            }
        }
    }

    // Stairs down in last room, stairs up in first
    if (rooms_.size() >= 2) {
        stairsDown_ = rooms_.back().center();
        stairsUp_   = rooms_.front().center();
        setTile(stairsDown_.x, stairsDown_.y, TileType::STAIRS_DOWN);
        if (floor > 1)
            setTile(stairsUp_.x, stairsUp_.y, TileType::STAIRS_UP);
    }

    // Occasional traps
    int traps = randInt(2, 4 + floor);
    for (int i = 0; i < traps; ++i) {
        auto& r = rooms_[randInt(0, (int)rooms_.size() - 1)];
        int tx = randInt(r.x, r.x + r.w - 1);
        int ty = randInt(r.y, r.y + r.h - 1);
        if (at(tx, ty).type == TileType::FLOOR)
            setTile(tx, ty, TileType::TRAP);
    }
}

// ─── FOV: simple shadow casting ───────────────────────────────────────────────
void DungeonMap::resetVisibility() {
    for (auto& t : tiles_) t.visible = false;
}

void DungeonMap::computeFOV(Vec2 origin, int radius) {
    resetVisibility();
    if (inBounds(origin)) {
        at(origin).visible = true;
        at(origin).explored = true;
    }
    // Cast rays in 360 degrees
    const int RAYS = 360;
    for (int i = 0; i < RAYS; ++i) {
        double angle = i * 2.0 * M_PI / RAYS;
        double dx = std::cos(angle);
        double dy = std::sin(angle);
        double cx = origin.x + 0.5, cy = origin.y + 0.5;
        for (int step = 0; step < radius; ++step) {
            cx += dx; cy += dy;
            int ix = (int)cx, iy = (int)cy;
            if (!inBounds(ix, iy)) break;
            at(ix, iy).visible  = true;
            at(ix, iy).explored = true;
            if (!at(ix, iy).transparent) break;
        }
    }
}
