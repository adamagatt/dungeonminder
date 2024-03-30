#ifndef __MONSTER_HPP_
#define __MONSTER_HPP_

#include "libtcod.hpp"

#include "config.hpp"

#include <string>
#include <unordered_map>

class Monster {
   public:
   Monster();

   std::string name;
   char symbol;
   int x, y, health, damage, timer, wait;
   int portalTimer;
   bool angry;
   int maxhealth;
   bool ranged, maimed;
   float range;
   std::string rangedName;
   std::unordered_map<Condition, int> conditionTimers;
};

#endif