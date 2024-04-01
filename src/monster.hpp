#ifndef __MONSTER_HPP_
#define __MONSTER_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "position.hpp"

#include <string>
#include <unordered_map>

class Monster {
   public:
   Monster();
   bool operator==(const Monster& other) const;
   [[nodiscard]] bool affectedBy(Condition condition) const;

   std::string name;
   char symbol;
   Position pos;
   int health, damage, timer, wait;
   int portalTimer;
   bool angry;
   int maxhealth;
   bool ranged, maimed;
   float range;
   std::string rangedName;
   std::unordered_map<Condition, int> conditionTimers;
};

#endif