#pragma once
#include "types.h"
#include "player.h"
#include <string>
#include <vector>
#include <functional>

struct CombatResult {
    bool      hit       = false;
    int       damage    = 0;
    bool      killed    = false;
    std::string message;
};

// Callback type for logging
using LogFn = std::function<void(const std::string&, const std::string&)>;

class Combat {
public:
    explicit Combat(LogFn logFn) : log_(logFn) {}

    // Player attacks monster
    CombatResult playerAttack(Player& player, Monster& monster);

    // Monster attacks player
    CombatResult monsterAttack(const Monster& monster, Player& player);

    // Ranged attack (placeholder for scrolls / bows)
    CombatResult rangedAttack(Player& player, Monster& monster, int bonusDmg = 0);

    // Apply damage with armor reduction
    static int applyDamage(int raw, int defense);

    // Critical hit check
    static bool isCritical(int dexterity = 10);

    // Miss check
    static bool isMiss(int attackerDex, int defenderDex);

private:
    LogFn log_;
    int rollDice(int sides) const;
};
