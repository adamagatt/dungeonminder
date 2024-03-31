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
   HeroPathCallback(const Map& map) : map(map) { }

   float getWalkCost(int xFrom, int yFrom, int xTo, int yTo, void *_userData ) const {
      const Tile& dest = Utils::tileAt(map, {xTo, yTo});

      float cost = 0.0f;
      if (dest == Tile::WALL) {
         cost = MAP_WIDTH * MAP_HEIGHT + 2;
      } else if (
         dest != Tile::STAIRS &&
         dest != Tile::STAIRS_UP &&
         dest != Tile::CHEST &&
         dest != Tile::CHEST_OPEN &&
         dest != Tile::FIELD
      ) {
         cost = abs(xFrom-xTo)+abs(yFrom-yTo);
      }
      return cost;
   }

   private:
   const Map& map;
};

class Hero {
   public:
   Hero(GameState& game, const MessageCallback& message, const RedrawCallback& redrawCallback);

   bool move();
   void giveItem();
   void computePath();
   bool gainHealth(int);
   bool checkWin() const;
   bool isAdjacent(int, int) const;
   bool inSpellRadius() const;

   Position pos;
   Position dest1;
   Position dest2;
   Position stairs;
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
   const MessageCallback& message;
   const RedrawCallback& redrawCallback;
   HeroPathCallback heroPathCallback;
   TCODPath path {MAP_WIDTH, MAP_HEIGHT, &heroPathCallback, nullptr, 0.0f};

   static constexpr float SPELL_RADIUS = 8.0f;
};

#endif