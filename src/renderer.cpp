#include "../include/renderer.h"
#include "../include/color.h"
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <cstring>

// ─── helpers ──────────────────────────────────────────────────────────────────
void Renderer::printAt(int row, int col, const std::string& text) const {
    Term::moveTo(row, col);
    std::printf("%s", text.c_str());
}

std::string Renderer::hpBar(int hp, int maxHp, int width) const {
    int filled = std::max(0, (int)((double)hp / maxHp * width));
    std::string bar = "[";
    for (int i = 0; i < width; ++i)
        bar += (i < filled) ? '|' : ' ';
    bar += "]";
    if ((double)hp / maxHp > 0.5)       return Color::paint(Color::BGREEN,  bar);
    else if ((double)hp / maxHp > 0.25) return Color::paint(Color::BYELLOW, bar);
    else                                 return Color::paint(Color::BRED,    bar);
}

std::string Renderer::manaBar(int mana, int maxMana, int width) const {
    int filled = std::max(0, (int)((double)mana / maxMana * width));
    std::string bar = "[";
    for (int i = 0; i < width; ++i)
        bar += (i < filled) ? '~' : ' ';
    bar += "]";
    return Color::paint(Color::BCYAN, bar);
}

void Renderer::drawBox(int row, int col, int w, int h, const std::string& title) const {
    // top
    printAt(row, col, Color::DIM + "+" + std::string(w - 2, '-') + "+" + Color::RESET);
    // sides
    for (int r = 1; r < h - 1; ++r) {
        printAt(row + r, col,         Color::DIM + "|" + Color::RESET);
        printAt(row + r, col + w - 1, Color::DIM + "|" + Color::RESET);
    }
    // bottom
    printAt(row + h - 1, col, Color::DIM + "+" + std::string(w - 2, '-') + "+" + Color::RESET);
    // title
    if (!title.empty())
        printAt(row, col + 2, Color::BYELLOW + "[ " + title + " ]" + Color::RESET);
}

std::string Renderer::itemColor(const Item& item) const {
    switch (item.type) {
        case ItemType::WEAPON:    return Color::BYELLOW;
        case ItemType::ARMOR:     return Color::BCYAN;
        case ItemType::POTION:    return Color::BGREEN;
        case ItemType::SCROLL:    return Color::BMAGENTA;
        case ItemType::FOOD:      return Color::YELLOW;
        case ItemType::GOLD_PILE: return Color::BYELLOW;
        default:                  return Color::WHITE;
    }
}

// ─── Tile color + glyph ───────────────────────────────────────────────────────
std::string Renderer::tileGlyph(const Tile& t, bool isPlayer, const Monster* mon) const {
    if (isPlayer) return Color::BOLD + Color::BWHITE + "@" + Color::RESET;
    if (mon && mon->alive) {
        std::string c = Color::BRED;
        char g = mon->glyph;
        // color by monster type
        if (g == 'g') c = Color::GREEN;
        else if (g == 'D') c = Color::BRED;
        else if (g == 'S') c = Color::CYAN;
        else if (g == 'Z') c = Color::DIM + Color::WHITE;
        return Color::BOLD + c + std::string(1, g) + Color::RESET;
    }
    if (!t.visible && !t.explored) return " ";

    std::string col = t.visible ? Color::WHITE : Color::DIM;

    switch (t.type) {
        case TileType::WALL:        return col + "#" + Color::RESET;
        case TileType::FLOOR:       return Color::DIM + "." + Color::RESET;
        case TileType::DOOR_CLOSED: return Color::YELLOW + "+" + Color::RESET;
        case TileType::DOOR_OPEN:   return Color::YELLOW + "/" + Color::RESET;
        case TileType::STAIRS_DOWN: return Color::BGREEN + ">" + Color::RESET;
        case TileType::STAIRS_UP:   return Color::BGREEN + "<" + Color::RESET;
        case TileType::CHEST:       return Color::BYELLOW + "$" + Color::RESET;
        case TileType::TRAP:        return t.visible
                                        ? Color::RED + "^" + Color::RESET
                                        : Color::DIM + "." + Color::RESET;
        default:                    return " ";
    }
}

// ─── Main Render ──────────────────────────────────────────────────────────────
void Renderer::render(const DungeonMap& map,
                      const Player& player,
                      const std::vector<Monster>& monsters,
                      const std::vector<LogEntry>& log,
                      const std::string& statusMsg)
{
    Term::clear();

    // Build a lookup for monster positions
    std::map<int, const Monster*> monMap;
    for (auto& m : monsters)
        if (m.alive)
            monMap[m.pos.y * DungeonMap::WIDTH + m.pos.x] = &m;

    // Draw map
    for (int y = 0; y < DungeonMap::HEIGHT; ++y) {
        Term::moveTo(y + 1, 1);
        for (int x = 0; x < DungeonMap::WIDTH; ++x) {
            const Tile& t = map.at(x, y);
            bool isPlayer = (player.pos.x == x && player.pos.y == y);
            int key = y * DungeonMap::WIDTH + x;
            const Monster* mon = (monMap.count(key) && t.visible) ? monMap[key] : nullptr;

            if (!t.visible && !t.explored) {
                std::printf(" ");
            } else {
                std::printf("%s", tileGlyph(t, isPlayer, mon).c_str());
            }
        }
    }

    // ── Side panel ────────────────────────────────────────────────────────────
    const int PX = DungeonMap::WIDTH + 2;
    drawBox(1, PX, PANEL_WIDTH, DungeonMap::HEIGHT, "");

    int row = 2;
    // Title
    printAt(row++, PX + 2, Color::BOLD + Color::BYELLOW + "  DUNGEON CRAWLER" + Color::RESET);
    printAt(row++, PX + 2, Color::DIM  + std::string(PANEL_WIDTH - 4, '~') + Color::RESET);

    // Player name + floor
    printAt(row++, PX + 2, Color::BWHITE + player.name + Color::RESET);
    printAt(row++, PX + 2, Color::CYAN + "Floor: " + Color::BWHITE + std::to_string(map.floor()) + Color::RESET);
    printAt(row++, PX + 2, Color::CYAN + "Turn:  " + Color::WHITE  + "...");
    row++;

    // HP / Mana
    printAt(row++, PX + 2, Color::WHITE + "HP   " +
            Color::BRED + std::to_string(player.stats.hp) + "/" + std::to_string(player.stats.maxHp) + Color::RESET);
    printAt(row++, PX + 2, hpBar(player.stats.hp, player.stats.maxHp, PANEL_WIDTH - 6));
    row++;
    printAt(row++, PX + 2, Color::WHITE + "Mana " +
            Color::BCYAN + std::to_string(player.stats.mana) + "/" + std::to_string(player.stats.maxMana) + Color::RESET);
    printAt(row++, PX + 2, manaBar(player.stats.mana, player.stats.maxMana, PANEL_WIDTH - 6));
    row++;

    // Stats
    printAt(row++, PX + 2, Color::BYELLOW + "Level " + std::to_string(player.stats.level) + Color::RESET);
    printAt(row++, PX + 2, Color::WHITE + "XP    " + std::to_string(player.stats.xp) + "/" + std::to_string(player.stats.xpNext));
    printAt(row++, PX + 2, Color::WHITE + "ATK   " + std::to_string(player.computeAttack()));
    printAt(row++, PX + 2, Color::WHITE + "DEF   " + std::to_string(player.computeDefense()));
    printAt(row++, PX + 2, Color::BYELLOW + "Gold  " + std::to_string(player.stats.gold) + "g" + Color::RESET);
    row++;

    // Equipment
    printAt(row++, PX + 2, Color::DIM + "Equipment:" + Color::RESET);
    std::string wpnName = player.equippedWeapon >= 0
                          ? player.inventory[player.equippedWeapon].name : "None";
    std::string armName = player.equippedArmor  >= 0
                          ? player.inventory[player.equippedArmor].name  : "None";
    printAt(row++, PX + 2, Color::BYELLOW + "W: " + Color::WHITE + wpnName.substr(0, PANEL_WIDTH - 6) + Color::RESET);
    printAt(row++, PX + 2, Color::BCYAN   + "A: " + Color::WHITE + armName.substr(0, PANEL_WIDTH - 6) + Color::RESET);
    row++;

    // Controls hint
    printAt(row++, PX + 2, Color::DIM + "hjklyubn:move" + Color::RESET);
    printAt(row++, PX + 2, Color::DIM + "i:inv  g:get" + Color::RESET);
    printAt(row++, PX + 2, Color::DIM + ">/<:stairs s:save" + Color::RESET);

    // ── Status message ────────────────────────────────────────────────────────
    if (!statusMsg.empty()) {
        Term::moveTo(DungeonMap::HEIGHT + 1, 1);
        std::printf("%s", (Color::BYELLOW + ">> " + statusMsg + Color::RESET).c_str());
    }

    // ── Message log ───────────────────────────────────────────────────────────
    int logRow = DungeonMap::HEIGHT + 2;
    int shown  = 0;
    for (int i = (int)log.size() - 1; i >= 0 && shown < LOG_LINES; --i, ++shown) {
        Term::moveTo(logRow + shown, 1);
        std::string prefix = shown == 0 ? Color::BWHITE : Color::DIM;
        std::printf("%s%s%s", prefix.c_str(), log[i].text.c_str(), Color::RESET.c_str());
    }

    std::fflush(stdout);
}

// ─── Inventory Screen ─────────────────────────────────────────────────────────
void Renderer::renderInventory(const Player& player, int selected) {
    Term::clear();
    const int W = 60, H = 30, START_R = 2, START_C = 10;
    drawBox(START_R, START_C, W, H, "INVENTORY");

    printAt(START_R + 2, START_C + 2,
            Color::WHITE + "Gold: " + Color::BYELLOW + std::to_string(player.stats.gold) + "g" + Color::RESET);
    printAt(START_R + 3, START_C + 2,
            Color::DIM + "[e]quip  [u]se  [d]rop  [ESC]back" + Color::RESET);
    printAt(START_R + 4, START_C + 2, Color::DIM + std::string(W - 4, '-') + Color::RESET);

    for (int i = 0; i < (int)player.inventory.size(); ++i) {
        const auto& item = player.inventory[i];
        std::string prefix = std::string(1, 'a' + i) + ") ";
        std::string equip  = item.equipped ? Color::BYELLOW + "*" + Color::RESET : " ";
        std::string col    = itemColor(item);
        std::string line   = prefix + equip + " " + col + item.name + Color::RESET;
        if (item.qty > 1)
            line += Color::DIM + " x" + std::to_string(item.qty) + Color::RESET;

        if (i == selected)
            printAt(START_R + 5 + i, START_C + 2, Color::BG_BLUE + line + Color::RESET);
        else
            printAt(START_R + 5 + i, START_C + 2, line);
    }

    if (player.inventory.empty())
        printAt(START_R + 6, START_C + 2, Color::DIM + "(empty)" + Color::RESET);

    // Item detail
    if (selected >= 0 && selected < (int)player.inventory.size()) {
        const auto& it = player.inventory[selected];
        printAt(START_R + H - 5, START_C + 2, Color::DIM + std::string(W - 4, '-') + Color::RESET);
        printAt(START_R + H - 4, START_C + 2, itemColor(it) + it.name + Color::RESET);
        printAt(START_R + H - 3, START_C + 2, Color::WHITE + it.desc + Color::RESET);
        std::string stats = "";
        if (it.atkBonus  > 0) stats += "ATK+" + std::to_string(it.atkBonus) + " ";
        if (it.defBonus  > 0) stats += "DEF+" + std::to_string(it.defBonus) + " ";
        if (it.hpRestore > 0) stats += "HP+"  + std::to_string(it.hpRestore) + " ";
        if (!stats.empty())
            printAt(START_R + H - 2, START_C + 2, Color::BGREEN + stats + Color::RESET);
    }

    std::fflush(stdout);
}

// ─── Level Up Screen ──────────────────────────────────────────────────────────
void Renderer::renderLevelUp(const Player& player) {
    Term::clear();
    printAt(10, 20, Color::BOLD + Color::BYELLOW + "  *** LEVEL UP! ***" + Color::RESET);
    printAt(12, 20, Color::WHITE + "You are now level " + std::to_string(player.stats.level) + "!");
    printAt(13, 20, Color::BGREEN + "HP fully restored.");
    printAt(14, 20, Color::BCYAN  + "ATK and DEF increased.");
    printAt(16, 20, Color::DIM    + "Press any key to continue..." + Color::RESET);
    std::fflush(stdout);
}

// ─── Dead Screen ──────────────────────────────────────────────────────────────
void Renderer::renderDead(const Player& player, int floorReached) {
    Term::clear();
    printAt( 5, 20, Color::BOLD + Color::BRED + "  YOU HAVE DIED" + Color::RESET);
    printAt( 7, 20, Color::WHITE + "Name:         " + player.name);
    printAt( 8, 20, Color::WHITE + "Level reached: " + std::to_string(player.stats.level));
    printAt( 9, 20, Color::WHITE + "Floor reached: " + std::to_string(floorReached));
    printAt(10, 20, Color::WHITE + "Gold earned:   " + std::to_string(player.stats.gold) + "g");
    printAt(12, 20, Color::DIM   + "Press any key to quit..." + Color::RESET);
    std::fflush(stdout);
}

// ─── Win Screen ───────────────────────────────────────────────────────────────
void Renderer::renderWin(const Player& player) {
    Term::clear();
    printAt( 5, 20, Color::BOLD + Color::BYELLOW + "  VICTORY!" + Color::RESET);
    printAt( 7, 20, Color::BGREEN + "You retrieved the Amulet of Yendor!");
    printAt( 9, 20, Color::WHITE  + "Final level: " + std::to_string(player.stats.level));
    printAt(10, 20, Color::WHITE  + "Gold:        " + std::to_string(player.stats.gold) + "g");
    printAt(12, 20, Color::DIM    + "Press any key to quit..." + Color::RESET);
    std::fflush(stdout);
}

// ─── Main Menu ────────────────────────────────────────────────────────────────
void Renderer::renderMainMenu() {
    Term::clear();
    const std::string banner[] = {
        R"(  ____  _   _ _   _  ____ _____ ___  _   _   ____  ____  ____  )",
        R"( |  _ \| | | | \ | |/ ___| ____/ _ \| \ | | |  _ \|  _ \/ ___| )",
        R"( | | | | | | |  \| | |  _|  _|| | | |  \| | | |_) | |_) | |  _ )",
        R"( | |_| | |_| | |\  | |_| | |__| |_| | |\  | |  _ <|  __/| |_| |)",
        R"( |____/ \___/|_| \_|\____|_____\___/|_| \_| |_| \_\_|    \____|)",
    };
    int r = 3;
    for (auto& line : banner)
        printAt(r++, 5, Color::BYELLOW + line + Color::RESET);

    printAt(10, 25, Color::BCYAN + "A Roguelike Adventure" + Color::RESET);
    printAt(12, 25, Color::WHITE + "[n] New Game");
    printAt(13, 25, Color::WHITE + "[l] Load Game");
    printAt(14, 25, Color::WHITE + "[q] Quit");
    printAt(16, 25, Color::DIM   + "Use hjkl or numpad to move" + Color::RESET);
    printAt(17, 25, Color::DIM   + "Reach floor 10 to win!" + Color::RESET);
    std::fflush(stdout);
}

// ─── Help Screen ─────────────────────────────────────────────────────────────
void Renderer::renderHelp() {
    Term::clear();
    drawBox(1, 5, 60, 30, "HELP");
    int r = 3;
    auto line = [&](const std::string& key, const std::string& desc) {
        printAt(r++, 7, Color::BYELLOW + key + Color::RESET +
                        Color::WHITE   + "  " + desc + Color::RESET);
    };
    line("hjkl / 1-9", "Move (8-directional)");
    line("g",          "Pick up item");
    line("i",          "Open inventory");
    line(">",          "Descend stairs");
    line("<",          "Ascend stairs");
    line("s",          "Save game");
    line(".",          "Wait a turn");
    line("?",          "Show this help");
    line("q",          "Quit game");
    r++;
    line("In inventory:", "");
    line("a-z",        "Select item");
    line("u",          "Use/drink/eat selected");
    line("e",          "Equip selected");
    line("d",          "Drop selected");
    line("ESC",        "Close inventory");
    printAt(r + 2, 7, Color::DIM + "Press any key to continue..." + Color::RESET);
    std::fflush(stdout);
}
