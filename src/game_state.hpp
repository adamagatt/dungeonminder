#ifndef __GAME_STATE_HPP_
#define __GAME_STATE_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "position.hpp"

#include <memory>
#include <string>
#include <vector>

struct Hero;
struct Monster;

class GameState {
   public:
   GameState();

   void addMessage(const std::string& message, MessageType type);
   
   Monster* findMonster(const Position& p);
   Monster* findMonster(int x, int y);
   Monster* heroFindMonster();
   void addMonster(const std::string&, char, int, int, int, int, bool, const std::string&, float, int, bool);
   void addSpecifiedMonster(int, int, int, bool);
   bool hitMonster(int x, int y, int amount);
   bool hitMonster(const Position& pos, int amount);

   Map map;
   TCODMap *mapModel = new TCODMap(MAP_WIDTH,MAP_HEIGHT);
   
   int level;
   bool bossDead {false};
   
   Position player;
   int heroMana = MANA_BLIP_SIZE*5;
   int monsterMana = MANA_BLIP_SIZE*5;
   int worldMana = MANA_BLIP_SIZE*5;

   std::vector<Monster> monsterList;
   std::unique_ptr<Hero> hero;

   int cloud [MAP_WIDTH][MAP_HEIGHT];
   Position illusion;

   std::array<std::string, MESSAGE_COUNT> messageList;
   std::array<MessageType, MESSAGE_COUNT> messageType;
};

#endif