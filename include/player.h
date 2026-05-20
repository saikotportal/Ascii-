#pragma once
#include "types.h"
#include <vector>
#include <string>

class Player {
public:
    Player();

    // Movement / position
    Vec2        pos;
    std::string name = "Hero";
    Stats       stats;

    // Inventory
    std::vector<Item> inventory;
    int               equippedWeapon = -1;  // index, -1 = none
    int               equippedArmor  = -1;

    // Actions
    void      pickupItem(const Item& item);
    bool      dropItem(int idx);
    bool      useItem(int idx);          // returns true if item consumed
    bool      equipItem(int idx);
    bool      unequipItem(int idx);
    int       computeAttack()  const;
    int       computeDefense() const;

    // XP / leveling
    void      gainXP(int amount);        // may trigger levelUp
    void      levelUp();

    // Serialization (simple text)
    std::string serialize()   const;
    bool        deserialize(const std::string& data);

    // Convenience
    bool        isDead()  const { return stats.hp <= 0; }
    int         maxItems() const { return 26; }   // a-z slots
};
