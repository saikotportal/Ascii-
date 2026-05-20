#include "../include/game.h"
#include "../include/color.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>

// ─── Monster templates ────────────────────────────────────────────────────────
static std::vector<MonsterDef> MONSTER_DEFS = {
    {"Rat",         'r',  8,  3, 0,  10,  10,  2, AIType::WANDER},
    {"Goblin",      'g', 14,  5, 1,  10,  20,  5, AIType::CHASE },
    {"Skeleton",    'S', 18,  6, 2,   8,  30,  8, AIType::CHASE },
    {"Zombie",      'Z', 22,  7, 3,   6,  40, 10, AIType::CHASE },
    {"Orc",         'o', 28,  9, 4,  10,  55, 15, AIType::CHASE },
    {"Troll",       'T', 40, 12, 5,   8,  80, 25, AIType::CHASE },
    {"Dark Mage",   'M', 24, 11, 2,  10, 100, 30, AIType::RANGED},
    {"Dragon",      'D', 60, 18, 8,   9, 200, 80, AIType::CHASE },
    {"Shadow",      'X', 30, 10, 6,  12,  90, 20, AIType::COWARD},
};

// ─── Item templates ───────────────────────────────────────────────────────────
static Item makePotion(const std::string& name, int hp, int mana, int val) {
    Item i; i.name=name; i.type=ItemType::POTION; i.glyph='!';
    i.hpRestore=hp; i.manaRestore=mana; i.value=val;
    i.desc = "Restores " + std::to_string(hp) + " HP.";
    return i;
}
static Item makeWeapon(const std::string& name, int atk, int val) {
    Item i; i.name=name; i.type=ItemType::WEAPON; i.glyph=')';
    i.atkBonus=atk; i.value=val;
    i.desc = "Attack +" + std::to_string(atk) + ".";
    return i;
}
static Item makeArmor(const std::string& name, int def, int val) {
    Item i; i.name=name; i.type=ItemType::ARMOR; i.glyph='[';
    i.defBonus=def; i.value=val;
    i.desc = "Defense +" + std::to_string(def) + ".";
    return i;
}
static Item makeGold(int amount) {
    Item i; i.name="Gold"; i.type=ItemType::GOLD_PILE; i.glyph='$';
    i.value=amount; i.qty=amount;
    i.desc = std::to_string(amount) + " gold coins.";
    return i;
}

// ─── Constructor ──────────────────────────────────────────────────────────────
Game::Game() : rng_(std::random_device{}()) {
    combat_ = std::make_unique<Combat>([this](const std::string& msg, const std::string& col) {
        addLog(msg, col);
    });
}
Game::~Game() { Term::showCursor(); Term::disableRaw(); }

// ─── Logging ──────────────────────────────────────────────────────────────────
void Game::addLog(const std::string& text, const std::string& col) {
    log_.push_front({text, col});
    if (log_.size() > 50) log_.pop_back();
}

// ─── New Game ─────────────────────────────────────────────────────────────────
void Game::newGame() {
    floor_ = 1;
    turnCount_ = 0;
    player_ = Player();
    player_.name = "Hero";
    player_.stats.hp = player_.stats.maxHp = 30;
    player_.stats.mana = player_.stats.maxMana = 15;
    // Starting gear
    player_.pickupItem(makeWeapon("Rusty Dagger", 2, 5));
    player_.pickupItem(makePotion("Healing Potion", 15, 0, 20));
    player_.equipItem(0);  // equip dagger

    map_.generate(floor_);
    player_.pos = map_.rooms()[0].center();
    spawnMonsters();
    spawnItems();
    map_.computeFOV(player_.pos, 8);
    addLog(Color::BGREEN + "Welcome to the Dungeon!" + Color::RESET, "\033[92m");
    addLog(Color::WHITE + "Reach floor 10 to find the Amulet!" + Color::RESET, "\033[97m");
    state_ = GameState::PLAYING;
}

// ─── Descend / Ascend ─────────────────────────────────────────────────────────
void Game::descend() {
    if (map_.at(player_.pos).type != TileType::STAIRS_DOWN) {
        addLog(Color::YELLOW + "No stairs down here." + Color::RESET); return;
    }
    floor_++;
    if (floor_ > 10) { state_ = GameState::WIN; return; }
    map_.generate(floor_);
    player_.pos = map_.rooms()[0].center();
    monsters_.clear();
    floorItems_.clear();
    spawnMonsters();
    spawnItems();
    map_.computeFOV(player_.pos, 8);
    addLog(Color::BCYAN + "You descend to floor " + std::to_string(floor_) + "." + Color::RESET);
}

void Game::ascend() {
    if (map_.at(player_.pos).type != TileType::STAIRS_UP) {
        addLog(Color::YELLOW + "No stairs up here." + Color::RESET); return;
    }
    if (floor_ <= 1) { addLog(Color::YELLOW + "You're already on the first floor."); return; }
    floor_--;
    map_.generate(floor_);
    player_.pos = map_.stairsDown();
    monsters_.clear();
    floorItems_.clear();
    spawnMonsters();
    spawnItems();
    map_.computeFOV(player_.pos, 8);
    addLog(Color::BCYAN + "You ascend to floor " + std::to_string(floor_) + "." + Color::RESET);
}

// ─── Spawn ────────────────────────────────────────────────────────────────────
Monster Game::randomMonster(int floorLevel) {
    std::uniform_int_distribution<int> dist(0, std::min((int)MONSTER_DEFS.size()-1, floorLevel));
    auto& def = MONSTER_DEFS[dist(rng_)];
    Monster m;
    m.name    = def.name; m.glyph = def.glyph;
    m.hp      = def.hp + floorLevel * 2;  m.maxHp = m.hp;
    m.atk     = def.atk + floorLevel;
    m.def     = def.def;
    m.xpDrop  = def.xpDrop + floorLevel * 5;
    m.goldDrop= def.goldDrop + floorLevel * 2;
    m.ai      = def.ai;
    m.alive   = true;
    return m;
}

void Game::spawnMonsters() {
    int count = 3 + floor_ * 2;
    auto& rooms = map_.rooms();
    for (int i = 0; i < count; ++i) {
        std::uniform_int_distribution<int> roomDist(1, (int)rooms.size() - 1);
        auto& room = rooms[roomDist(rng_)];
        std::uniform_int_distribution<int> rx(room.x, room.x + room.w - 1);
        std::uniform_int_distribution<int> ry(room.y, room.y + room.h - 1);
        Vec2 pos{rx(rng_), ry(rng_)};
        if (!map_.at(pos).passable || pos == player_.pos) continue;
        auto m = randomMonster(floor_ - 1);
        m.pos = pos;
        monsters_.push_back(m);
    }
}

Item Game::randomItem(int floorLevel) {
    std::vector<Item> pool = {
        makePotion("Healing Potion", 15 + floorLevel*2, 0, 20),
        makePotion("Mana Potion", 0, 10 + floorLevel, 15),
        makePotion("Elixir", 25 + floorLevel*3, 8, 40),
        makeWeapon("Short Sword",   4 + floorLevel,     30),
        makeWeapon("Battleaxe",     7 + floorLevel*2,   60),
        makeWeapon("Magic Staff",   6 + floorLevel,     55),
        makeArmor("Leather Armor",  3 + floorLevel/2,   25),
        makeArmor("Chain Mail",     6 + floorLevel,     50),
        makeArmor("Plate Armor",   10 + floorLevel*2,  100),
        makeGold(10 + floorLevel * 5),
    };
    std::uniform_int_distribution<int> d(0, (int)pool.size() - 1);
    return pool[d(rng_)];
}

void Game::spawnItems() {
    int count = 2 + floor_;
    auto& rooms = map_.rooms();
    for (int i = 0; i < count; ++i) {
        std::uniform_int_distribution<int> roomDist(0, (int)rooms.size() - 1);
        auto& room = rooms[roomDist(rng_)];
        std::uniform_int_distribution<int> rx(room.x, room.x + room.w - 1);
        std::uniform_int_distribution<int> ry(room.y, room.y + room.h - 1);
        Vec2 pos{rx(rng_), ry(rng_)};
        if (!map_.at(pos).passable) continue;
        auto item = randomItem(floor_);
        item.atkBonus = item.atkBonus;  // already set
        // store position in item (hack: embed in desc temporarily)
        floorItems_.push_back(item);
        // Tag tile
        // We need item positions — store them separately:
        // (We'll use a simple parallel vector of Vec2)
        // For simplicity encode pos into item.value temporarily
        // Better: store as pair
        // Rebuild: use a struct
        // Actually, just set glyph on tile
        map_.at(pos).hasItem = true;
    }
}

// ─── Helpers ──────────────────────────────────────────────────────────────────
Monster* Game::monsterAt(Vec2 pos) {
    for (auto& m : monsters_)
        if (m.alive && m.pos == pos) return &m;
    return nullptr;
}

bool Game::isOccupied(Vec2 pos) const {
    for (auto& m : monsters_)
        if (m.alive && m.pos == pos) return true;
    return false;
}

void Game::removeDeadMonsters() {
    monsters_.erase(std::remove_if(monsters_.begin(), monsters_.end(),
        [](const Monster& m) { return !m.alive; }), monsters_.end());
}

// ─── Player Move ──────────────────────────────────────────────────────────────
void Game::handlePlayerMove(Vec2 delta) {
    Vec2 newPos = player_.pos + delta;
    if (!map_.inBounds(newPos)) return;
    Tile& tile = map_.at(newPos);

    // Attack monster if present
    if (Monster* m = monsterAt(newPos)) {
        combat_->playerAttack(player_, *m);
        if (!m->alive) removeDeadMonsters();
        // Check level up
        if (player_.stats.xp >= player_.stats.xpNext) {
            // level up handled in gainXP
            addLog(Color::BOLD + Color::BYELLOW + "You leveled up to " +
                   std::to_string(player_.stats.level) + "!" + Color::RESET);
        }
        return;
    }

    if (!tile.passable) {
        // Try to open door
        if (tile.type == TileType::DOOR_CLOSED) {
            tile.type = TileType::DOOR_OPEN;
            tile.glyph = '/'; tile.passable = true; tile.transparent = true;
            addLog(Color::YELLOW + "You open the door." + Color::RESET);
        }
        return;
    }

    player_.pos = newPos;

    // Trap trigger
    if (tile.type == TileType::TRAP) {
        int dmg = 3 + floor_ * 2;
        player_.stats.hp -= dmg;
        addLog(Color::BRED + "You triggered a trap! -" + std::to_string(dmg) + " HP!" + Color::RESET);
    }

    // Stairs message
    if (tile.type == TileType::STAIRS_DOWN)
        addLog(Color::DIM + "You see stairs going down. Press > to descend." + Color::RESET);
    if (tile.type == TileType::STAIRS_UP)
        addLog(Color::DIM + "You see stairs going up. Press < to ascend." + Color::RESET);
}

// ─── Pickup ───────────────────────────────────────────────────────────────────
void Game::handlePickup() {
    if (!floorItems_.empty()) {
        Item it = floorItems_.back();
        floorItems_.pop_back();
        if (it.type == ItemType::GOLD_PILE) {
            player_.stats.gold += it.value;
            addLog(Color::BYELLOW + "You pick up " + std::to_string(it.value) + " gold." + Color::RESET);
        } else {
            player_.pickupItem(it);
            addLog(Color::BGREEN + "You pick up: " + it.name + Color::RESET);
        }
    } else {
        addLog(Color::DIM + "Nothing to pick up here." + Color::RESET);
    }
}

// ─── Monster AI ───────────────────────────────────────────────────────────────
void Game::updateMonsters() {
    for (auto& m : monsters_) {
        if (!m.alive) continue;
        if (m.turnCooldown > 0) { m.turnCooldown--; continue; }

        Vec2 dp = {player_.pos.x - m.pos.x, player_.pos.y - m.pos.y};
        int dist = std::abs(dp.x) + std::abs(dp.y);

        if (dist == 1) {
            // Adjacent: attack
            combat_->monsterAttack(m, player_);
            continue;
        }

        Vec2 move = {0, 0};
        switch (m.ai) {
            case AIType::CHASE:
                if (map_.at(m.pos).visible) {
                    move.x = (dp.x > 0) ? 1 : (dp.x < 0) ? -1 : 0;
                    move.y = (dp.y > 0) ? 1 : (dp.y < 0) ? -1 : 0;
                }
                break;
            case AIType::WANDER: {
                std::uniform_int_distribution<int> d(-1, 1);
                move = {d(rng_), d(rng_)};
                break;
            }
            case AIType::COWARD:
                if (dist < 6) {
                    move.x = (dp.x > 0) ? -1 : 1;
                    move.y = (dp.y > 0) ? -1 : 1;
                }
                break;
            case AIType::RANGED:
                if (dist <= 6 && dist > 1 && map_.at(m.pos).visible) {
                    // Ranged attack
                    int dmg = std::max(1, m.atk - player_.computeDefense() / 2);
                    player_.stats.hp -= dmg;
                    addLog(Color::BRED + m.name + " fires a bolt at you for " +
                           std::to_string(dmg) + " damage!" + Color::RESET);
                }
                break;
        }

        Vec2 np = m.pos + move;
        if (move.x != 0 || move.y != 0) {
            if (map_.inBounds(np) && map_.at(np).passable &&
                np != player_.pos && !monsterAt(np)) {
                m.pos = np;
            }
        }
        m.turnCooldown = std::max(0, 10 - m.atk / 2);
    }
}

// ─── Inventory Input ──────────────────────────────────────────────────────────
void Game::handleInventoryInput(char c) {
    if (c == 27 || c == 'i') { state_ = GameState::PLAYING; return; }
    if (c >= 'a' && c < 'a' + (int)player_.inventory.size()) {
        invSel_ = c - 'a';
    }
    if (c == 'u' && invSel_ < (int)player_.inventory.size()) {
        auto name = player_.inventory[invSel_].name;
        bool used = player_.useItem(invSel_);
        if (used) addLog(Color::BGREEN + "You use the " + name + "." + Color::RESET);
        invSel_ = std::min(invSel_, (int)player_.inventory.size() - 1);
    }
    if (c == 'e' && invSel_ < (int)player_.inventory.size()) {
        auto name = player_.inventory[invSel_].name;
        player_.equipItem(invSel_);
        addLog(Color::BCYAN + "You equip the " + name + "." + Color::RESET);
    }
    if (c == 'd' && invSel_ < (int)player_.inventory.size()) {
        auto name = player_.inventory[invSel_].name;
        player_.dropItem(invSel_);
        addLog(Color::YELLOW + "You drop the " + name + "." + Color::RESET);
        invSel_ = std::max(0, invSel_ - 1);
    }
}

// ─── Save / Load ──────────────────────────────────────────────────────────────
bool Game::saveGame(const std::string& filename) {
    std::ofstream f(filename);
    if (!f) return false;
    f << floor_ << "\n" << turnCount_ << "\n";
    f << player_.serialize();
    addLog(Color::BGREEN + "Game saved." + Color::RESET);
    return true;
}

bool Game::loadGame(const std::string& filename) {
    std::ifstream f(filename);
    if (!f) { addLog(Color::RED + "No save found." + Color::RESET); return false; }
    f >> floor_ >> turnCount_;
    f.ignore();
    std::string rest((std::istreambuf_iterator<char>(f)), {});
    player_.deserialize(rest);
    map_.generate(floor_);
    spawnMonsters();
    spawnItems();
    map_.computeFOV(player_.pos, 8);
    addLog(Color::BGREEN + "Game loaded (floor " + std::to_string(floor_) + ")." + Color::RESET);
    state_ = GameState::PLAYING;
    return true;
}

// ─── Handle Input ─────────────────────────────────────────────────────────────
void Game::handleInput(char c) {
    if (state_ == GameState::INVENTORY) {
        handleInventoryInput(c);
        return;
    }

    Vec2 delta = dirFromChar(c);
    if (delta.x != 0 || delta.y != 0) {
        handlePlayerMove(delta);
        updateMonsters();
        map_.computeFOV(player_.pos, 8);
        turnCount_++;
        // Regen
        if (turnCount_ % 10 == 0) {
            player_.stats.hp   = std::min(player_.stats.maxHp,   player_.stats.hp   + 1);
            player_.stats.mana = std::min(player_.stats.maxMana,  player_.stats.mana + 1);
        }
        if (player_.isDead()) state_ = GameState::DEAD;
        return;
    }

    switch (c) {
        case 'i': state_ = GameState::INVENTORY; invSel_ = 0; break;
        case 'g': handlePickup();
                  updateMonsters();
                  map_.computeFOV(player_.pos, 8);
                  turnCount_++;
                  break;
        case '>': descend(); break;
        case '<': ascend();  break;
        case '.': updateMonsters();
                  map_.computeFOV(player_.pos, 8);
                  turnCount_++;
                  addLog(Color::DIM + "You wait..." + Color::RESET);
                  break;
        case 's': saveGame(); break;
        case '?': renderer_.renderHelp(); Term::getChar(); break;
        case 'q': state_ = GameState::DEAD; break;
    }
    if (player_.isDead()) state_ = GameState::DEAD;
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void Game::run() {
    Term::enableRaw();
    Term::hideCursor();

    renderer_.renderMainMenu();
    char c = Term::getChar();

    if (c == 'q') { Term::disableRaw(); Term::showCursor(); return; }
    if (c == 'l') { loadGame(); }
    else           { newGame(); }

    while (true) {
        // Draw
        if (state_ == GameState::INVENTORY) {
            renderer_.renderInventory(player_, invSel_);
        } else if (state_ == GameState::PLAYING) {
            std::vector<LogEntry> logVec(log_.begin(), log_.end());
            renderer_.render(map_, player_, monsters_, logVec);
        } else if (state_ == GameState::DEAD) {
            renderer_.renderDead(player_, floor_);
            Term::getChar();
            break;
        } else if (state_ == GameState::WIN) {
            renderer_.renderWin(player_);
            Term::getChar();
            break;
        }

        char input = Term::getChar();
        handleInput(input);
    }

    Term::showCursor();
    Term::disableRaw();
    Term::moveTo(35, 1);
}
