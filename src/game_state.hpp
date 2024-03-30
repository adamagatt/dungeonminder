#ifndef __GAME_STATE_HPP_
#define __GAME_STATE_HPP_

#include "libtcod.hpp"

#include "config.hpp"

#include <memory>
#include <string>
#include <vector>

struct Hero;
struct Monster;

using RedrawCallback = void(&)();
using Map = int[MAP_WIDTH][MAP_HEIGHT];

class GameState {
   public:
   GameState(const MessageCallback& message, const RedrawCallback& redrawCallback);

   Monster* findMonster(int x, int y);
   Monster* heroFindMonster();
   void addMonster(const std::string&, char, int, int, int, int, bool, const std::string&, float, int, bool);
   void addSpecifiedMonster(int, int, int, bool);
   void hitMonster(int x, int y, int amount);

   Map map;
   TCODMap *mapModel = new TCODMap(MAP_WIDTH,MAP_HEIGHT);
   std::vector<Monster> monsterList;
   std::unique_ptr<Hero> hero;

   int level;
   bool bossDead {false};
   Position player;
   Position illusion;

   private:
   const MessageCallback& message;
};

#endif