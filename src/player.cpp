#include "../include/player.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

Player::Player() {
    pos = {1, 1};
}

// ─── Pickup ───────────────────────────────────────────────────────────────────
void Player::pickupItem(const Item& item) {
    if (item.type == ItemType::GOLD_PILE) {
        stats.gold += item.value * item.qty;
        return;
    }
    if ((int)inventory.size() < maxItems())
        inventory.push_back(item);
}

bool Player::dropItem(int idx) {
    if (idx < 0 || idx >= (int)inventory.size()) return false;
    inventory.erase(inventory.begin() + idx);
    return true;
}

// ─── Use ──────────────────────────────────────────────────────────────────────
bool Player::useItem(int idx) {
    if (idx < 0 || idx >= (int)inventory.size()) return false;
    auto& item = inventory[idx];
    bool consumed = false;

    if (item.type == ItemType::POTION) {
        stats.hp   = std::min(stats.maxHp,   stats.hp   + item.hpRestore);
        stats.mana = std::min(stats.maxMana,  stats.mana + item.manaRestore);
        consumed = true;
    } else if (item.type == ItemType::FOOD) {
        stats.hp = std::min(stats.maxHp, stats.hp + item.hpRestore);
        consumed = true;
    } else if (item.type == ItemType::WEAPON || item.type == ItemType::ARMOR) {
        equipItem(idx);
        return false; // equip doesn't remove item
    }

    if (consumed) {
        item.qty--;
        if (item.qty <= 0)
            inventory.erase(inventory.begin() + idx);
    }
    return consumed;
}

// ─── Equip ────────────────────────────────────────────────────────────────────
bool Player::equipItem(int idx) {
    if (idx < 0 || idx >= (int)inventory.size()) return false;
    auto& item = inventory[idx];

    if (item.type == ItemType::WEAPON) {
        if (equippedWeapon >= 0 && equippedWeapon < (int)inventory.size())
            inventory[equippedWeapon].equipped = false;
        equippedWeapon = idx;
        item.equipped  = true;
        return true;
    }
    if (item.type == ItemType::ARMOR) {
        if (equippedArmor >= 0 && equippedArmor < (int)inventory.size())
            inventory[equippedArmor].equipped = false;
        equippedArmor = idx;
        item.equipped = true;
        return true;
    }
    return false;
}

bool Player::unequipItem(int idx) {
    if (idx < 0 || idx >= (int)inventory.size()) return false;
    auto& item = inventory[idx];
    if (equippedWeapon == idx) equippedWeapon = -1;
    if (equippedArmor  == idx) equippedArmor  = -1;
    item.equipped = false;
    return true;
}

// ─── Combat stats ─────────────────────────────────────────────────────────────
int Player::computeAttack() const {
    int base = stats.atk;
    if (equippedWeapon >= 0 && equippedWeapon < (int)inventory.size())
        base += inventory[equippedWeapon].atkBonus;
    return base;
}

int Player::computeDefense() const {
    int base = stats.def;
    if (equippedArmor >= 0 && equippedArmor < (int)inventory.size())
        base += inventory[equippedArmor].defBonus;
    return base;
}

// ─── XP / Level ──────────────────────────────────────────────────────────────
void Player::gainXP(int amount) {
    stats.xp += amount;
    while (stats.xp >= stats.xpNext) {
        stats.xp -= stats.xpNext;
        levelUp();
    }
}

void Player::levelUp() {
    stats.level++;
    stats.xpNext  = stats.level * 100 + (stats.level - 1) * 50;
    stats.maxHp  += 8;
    stats.hp      = stats.maxHp;          // full heal on level up
    stats.maxMana += 4;
    stats.mana    = stats.maxMana;
    stats.atk    += 2;
    stats.def    += 1;
}

// ─── Serialization ────────────────────────────────────────────────────────────
std::string Player::serialize() const {
    std::ostringstream ss;
    ss << name << "\n"
       << stats.hp    << " " << stats.maxHp   << "\n"
       << stats.mana  << " " << stats.maxMana  << "\n"
       << stats.atk   << " " << stats.def      << "\n"
       << stats.level << " " << stats.xp << " " << stats.xpNext << "\n"
       << stats.gold  << "\n"
       << pos.x       << " " << pos.y           << "\n"
       << inventory.size() << "\n";
    for (auto& item : inventory) {
        ss << item.name  << "|"
           << (int)item.type << "|"
           << item.atkBonus  << "|"
           << item.defBonus  << "|"
           << item.hpRestore << "|"
           << item.value     << "|"
           << item.qty       << "|"
           << item.equipped  << "\n";
    }
    return ss.str();
}

bool Player::deserialize(const std::string& data) {
    std::istringstream ss(data);
    int invSize = 0;
    ss >> name
       >> stats.hp   >> stats.maxHp
       >> stats.mana >> stats.maxMana
       >> stats.atk  >> stats.def
       >> stats.level >> stats.xp >> stats.xpNext
       >> stats.gold
       >> pos.x >> pos.y
       >> invSize;
    ss.ignore();
    inventory.clear();
    equippedWeapon = -1;
    equippedArmor  = -1;
    for (int i = 0; i < invSize; ++i) {
        std::string line;
        std::getline(ss, line);
        std::istringstream ls(line);
        Item item;
        int typeInt, equippedInt;
        std::getline(ls, item.name, '|');
        ls >> typeInt; ls.ignore();
        item.type = (ItemType)typeInt;
        ls >> item.atkBonus; ls.ignore();
        ls >> item.defBonus; ls.ignore();
        ls >> item.hpRestore; ls.ignore();
        ls >> item.value; ls.ignore();
        ls >> item.qty; ls.ignore();
        ls >> equippedInt;
        item.equipped = equippedInt != 0;
        if (item.equipped) {
            if (item.type == ItemType::WEAPON) equippedWeapon = i;
            if (item.type == ItemType::ARMOR)  equippedArmor  = i;
        }
        inventory.push_back(item);
    }
    return true;
}
