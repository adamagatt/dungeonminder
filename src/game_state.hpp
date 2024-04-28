#ifndef __GAME_STATE_HPP_
#define __GAME_STATE_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "map.hpp"
#include "player.hpp"
#include "position.hpp"

#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Hero;
class Monster;

class GameState {
   public:
   GameState();

   void addMessage(const std::string& message, MessageType type);
   
   void createLevel(int level, bool isLastLevel);
   [[nodiscard]] Tile& tileAt(const Position& pos);
   [[nodiscard]] const Tile& tileAt(const Position& pos) const;
   void setTile(const Position& pos, Tile tile);

   [[nodiscard]] bool isInFov(int x, int y) const;
   [[nodiscard]] bool isInFov(const Position& pos) const;

   [[nodiscard]] Monster* findMonster(const Position& p);
   [[nodiscard]] Monster* findMonster(int x, int y);
   Monster* heroFindMonster();

   void createEndBoss();
   void generateMonsters(int level, int amount);
   void addMonster(const MonsterType& type, const Position& pos, bool portalSpawned);
   void addSpecifiedMonster(const Position& pos, int number, bool portalSpawned);
   
   bool hitMonster(int x, int y, int amount);
   bool hitMonster(const Position& pos, int amount);

   [[nodiscard]] int getHeroSpec() const;
   void setHeroSpec(int heroSpec);
   [[nodiscard]] int getMonsterSpec() const;
   void setMonsterSpec(int monsterSpec);
   [[nodiscard]] int getWorldSpec() const;
   void setWorldSpec(int worldSpec);

   Player player;
   std::vector<Monster> monsterList;
   std::unique_ptr<Hero> hero;
   
   Map map;
   bool bossDead {false};
   Position illusion;

   struct Message {
      std::string text;
      MessageType type;
   };

   std::list<Message> messageList;
   static constexpr int MESSAGE_COUNT = 28;
};

#endif