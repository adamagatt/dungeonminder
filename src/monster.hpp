#ifndef __MONSTER_HPP_
#define __MONSTER_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "entity.hpp"
#include "game_state.hpp"
#include "position.hpp"

#include <cmath>
#include <string>
#include <unordered_map>
#include <variant>

class Monster : public Entity {
   public:
   Monster(const MonsterType& type, const Position& pos, bool portalSpawned);

   bool operator==(const Monster& other) const;
   [[nodiscard]] bool affectedBy(Condition condition) const;
   void takeTurn(GameState& state);

   const MonsterType* type;
   int portalTimer;
   bool angry {false};
   bool maimed {false};
   std::unordered_map<Condition, int> conditionTimers;

   private:
   struct MakeRangedAttack{};
   struct MoveTo{Position pos;};
   using ActionDecision = std::variant<MakeRangedAttack, MoveTo>;

   void takeAction(GameState& state);
   ActionDecision decideOnAction(const GameState& state);
   void rangedAttackHero(GameState& state);
   void performMove(GameState& state, Position diff);
   void decreaseConditionTimers(GameState& state);

   inline int getDamageDealt() {
      return std::max(
         0,
         type->damage - static_cast<int>(std::ceil(static_cast<double>(conditionTimers[Condition::WEAKENED])/CONDITION_TIMES.at(Condition::WEAKENED)))
      );
   }
};

#endif