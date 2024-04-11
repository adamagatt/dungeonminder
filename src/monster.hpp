#ifndef __MONSTER_HPP_
#define __MONSTER_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "game_state.hpp"
#include "position.hpp"

#include <cmath>
#include <string>
#include <unordered_map>
#include <variant>

class Monster {
   public:
   Monster();
   bool operator==(const Monster& other) const;
   [[nodiscard]] bool affectedBy(Condition condition) const;
   void takeTurn(GameState& state);

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

   private:
   struct RangedAttack{};
   struct MoveTo{Position pos;};
   using ActionDecision = std::variant<RangedAttack, MoveTo>;

   void takeAction(GameState& state);
   ActionDecision decideOnAction(const GameState& state);
   void rangedAttackHero(GameState& state);
   void performMove(GameState& state, Position diff);
   void decreaseConditionTimers(GameState& state);

   inline int getDamageDealt() {
      return std::max(
         0,
         this->damage - static_cast<int>(std::ceil(static_cast<double>(conditionTimers[Condition::WEAKENED])/CONDITION_TIMES.at(Condition::WEAKENED)))
      );
   }
};

#endif