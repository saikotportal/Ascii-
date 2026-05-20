#pragma once
#include "dungeon.h"
#include "player.h"
#include "types.h"
#include <vector>
#include <string>

class Renderer {
public:
    static const int LOG_LINES   = 5;
    static const int PANEL_WIDTH = 24;

    Renderer() = default;

    // Main render call — draws everything
    void render(const DungeonMap& map,
                const Player& player,
                const std::vector<Monster>& monsters,
                const std::vector<LogEntry>& log,
                const std::string& statusMsg = "");

    // Sub-screens
    void renderInventory(const Player& player, int selected);
    void renderLevelUp(const Player& player);
    void renderDead(const Player& player, int floorReached);
    void renderWin(const Player& player);
    void renderMainMenu();
    void renderHelp();

private:
    // Helpers
    std::string tileGlyph(const Tile& t, bool isPlayer,
                          const Monster* mon) const;
    std::string hpBar(int hp, int maxHp, int width) const;
    std::string manaBar(int mana, int maxMana, int width) const;
    void        drawBox(int row, int col, int w, int h,
                        const std::string& title = "") const;
    void        printAt(int row, int col, const std::string& text) const;
    std::string itemColor(const Item& item) const;
};
