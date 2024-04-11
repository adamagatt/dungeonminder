#ifndef __HERO_HPP_
#define __HERO_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "game_state.hpp"
#include "monster.hpp"
#include "position.hpp"
#include "utils.hpp"

#include <unordered_set>

class HeroPathCallback : public ITCODPathCallback {
   public :
   HeroPathCallback(const GameState& state) : state(state) { }

   float getWalkCost(int xFrom, int yFrom, int xTo, int yTo, void *_userData ) const {
      const Tile& dest = state.tileAt({xTo, yTo});

      float cost = 0.0f;
      switch(dest) {
         case Tile::STAIRS:
         case Tile::STAIRS_UP:
         case Tile::CHEST:
         case Tile::CHEST_OPEN:
         case Tile::FIELD:
            break;
         case Tile::WALL:
            cost = MAP_WIDTH * MAP_HEIGHT + 2;
            break;
         default:
            cost = abs(xFrom-xTo)+abs(yFrom-yTo);
            break;
      }
      return cost;
   }

   private:
   const GameState& state;
};

class Hero {
   public:
   Hero(GameState& game);

   bool move();
   void giveItem();
   void computePath();
   bool gainHealth(int);
   void die();

   bool checkWin() const;
   bool isAdjacent(int, int) const;
   bool isAdjacent(const Position& pos) const;
   bool inSpellRadius() const;

   Position pos;

   enum class Goal {chest1, chest2, exit} currentGoal;

   
   int wait, timer, health, damage;
   Monster* target;
   int pathstep;
   bool dead, slow, blinking;
   int hasteTimer, pacifismTimer, shieldTimer, regenTimer, meditationTimer, seeInvisibleTimer, summonMonsterTimer;
   std::unordered_set<Item> items;

   static const std::array<std::string, 5> heroEntry;
   static const std::array<std::string, 10> heroKills;
   static const std::array<std::string, 10> heroFight;
   static const std::array<std::string, 5> heroScared;
   static const std::array<std::string, 5> heroExit;
   static const std::array<std::string, 5> heroBump;
   static const std::array<std::string, 10> heroItem;
   static const std::array<std::string, 5> heroCharity;
   static const std::array<std::string, 5> heroBlow;
   static const std::array<std::string, 5> heroIllusion;

   private:
   GameState& game;
   HeroPathCallback heroPathCallback;
   TCODPath path {MAP_WIDTH, MAP_HEIGHT, &heroPathCallback, nullptr, 0.0f};

   static constexpr float SPELL_RADIUS = 8.0f;
};

#endif