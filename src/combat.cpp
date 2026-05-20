#include "../include/combat.h"
#include "../include/color.h"
#include <random>
#include <ctime>
#include <algorithm>
#include <sstream>

static std::mt19937 combatRng(std::random_device{}());

int Combat::rollDice(int sides) const {
    return std::uniform_int_distribution<int>(1, sides)(combatRng);
}

int Combat::applyDamage(int raw, int defense) {
    int dmg = raw - defense;
    return std::max(1, dmg); // always at least 1
}

bool Combat::isCritical(int dexterity) {
    int threshold = 5 + dexterity / 5;  // ~5-10% base crit
    return std::uniform_int_distribution<int>(1, 100)(combatRng) <= threshold;
}

bool Combat::isMiss(int attackerDex, int defenderDex) {
    int missChance = 10 + (defenderDex - attackerDex) * 2;
    missChance = std::clamp(missChance, 5, 40);
    return std::uniform_int_distribution<int>(1, 100)(combatRng) <= missChance;
}

// ─── Player attacks Monster ───────────────────────────────────────────────────
CombatResult Combat::playerAttack(Player& player, Monster& monster) {
    CombatResult result;

    if (isMiss(10, 10)) {
        result.hit     = false;
        result.message = Color::yellow("You swing at the ") +
                         Color::bred(monster.name) +
                         Color::yellow(" and miss!");
        log_(result.message, "\033[33m");
        return result;
    }

    int raw  = player.computeAttack() + rollDice(6);
    bool crit = isCritical();
    if (crit) raw *= 2;

    result.hit    = true;
    result.damage = applyDamage(raw, monster.def);
    monster.hp   -= result.damage;
    result.killed = monster.hp <= 0;

    std::ostringstream msg;
    if (crit)
        msg << Color::BOLD << Color::BYELLOW << "CRITICAL HIT! " << Color::RESET;
    msg << Color::white("You hit the ")
        << Color::bred(monster.name)
        << Color::white(" for ")
        << Color::BOLD << Color::BRED << std::to_string(result.damage) << Color::RESET
        << Color::white(" damage.");

    if (result.killed) {
        msg << " " << Color::bgreen(monster.name + " dies!");
        player.gainXP(monster.xpDrop);
        player.stats.gold += monster.goldDrop;
    }

    result.message = msg.str();
    log_(result.message, "\033[97m");
    return result;
}

// ─── Monster attacks Player ───────────────────────────────────────────────────
CombatResult Combat::monsterAttack(const Monster& monster, Player& player) {
    CombatResult result;

    if (isMiss(8, 10)) {
        result.hit     = false;
        result.message = Color::yellow("The ") + Color::bred(monster.name) +
                         Color::yellow(" swings at you and misses.");
        log_(result.message, "\033[33m");
        return result;
    }

    int raw       = monster.atk + rollDice(4);
    result.hit    = true;
    result.damage = applyDamage(raw, player.computeDefense());
    player.stats.hp -= result.damage;
    result.killed   = player.stats.hp <= 0;

    std::ostringstream msg;
    msg << Color::white("The ")
        << Color::bred(monster.name)
        << Color::white(" hits you for ")
        << Color::BOLD << Color::BRED << std::to_string(result.damage) << Color::RESET
        << Color::white(" damage!");

    if (result.killed)
        msg << " " << Color::BOLD << Color::BRED << "You are slain!" << Color::RESET;

    result.message = msg.str();
    log_(result.message, "\033[91m");
    return result;
}

// ─── Ranged Attack ────────────────────────────────────────────────────────────
CombatResult Combat::rangedAttack(Player& player, Monster& monster, int bonusDmg) {
    CombatResult result;
    int raw       = player.computeAttack() + bonusDmg + rollDice(8);
    result.hit    = true;
    result.damage = applyDamage(raw, monster.def / 2);
    monster.hp   -= result.damage;
    result.killed  = monster.hp <= 0;

    std::ostringstream msg;
    msg << Color::cyan("Your ranged attack hits the ")
        << Color::bred(monster.name)
        << Color::cyan(" for ")
        << Color::BOLD << Color::BRED << std::to_string(result.damage) << Color::RESET
        << Color::cyan("!");
    if (result.killed) {
        msg << " " << Color::bgreen(monster.name + " is destroyed!");
        player.gainXP(monster.xpDrop);
        player.stats.gold += monster.goldDrop;
    }
    result.message = msg.str();
    log_(result.message, "\033[96m");
    return result;
}
