#ifndef __GAME_STATE_HPP_
#define __GAME_STATE_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "position.hpp"

#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Hero;
struct Monster;

class GameState {
   public:
   GameState();

   void addMessage(const std::string& message, MessageType type);
   
   [[nodiscard]] Tile& tileAt(const Position& pos);
   [[nodiscard]] const Tile& tileAt(const Position& pos) const;
   void setTile(const Position& pos, Tile tile);

   [[nodiscard]] bool isInFov(int x, int y) const;
   [[nodiscard]] bool isInFov(const Position& pos) const;
   [[nodiscard]] bool isEmptyPatch(int x, int y) const;
   [[nodiscard]] bool isEmptyPatch(const Position& pos) const;

   [[nodiscard]] Monster* findMonster(const Position& p);
   [[nodiscard]] Monster* findMonster(int x, int y);
   Monster* heroFindMonster();
   void addMonster(const std::string&, char, int, int, int, int, bool, const std::string&, float, int, bool);
   void addSpecifiedMonster(int, int, int, bool);
   bool hitMonster(int x, int y, int amount);
   bool hitMonster(const Position& pos, int amount);

   using Tiles = std::array<std::array<Tile, MAP_HEIGHT>, MAP_WIDTH>;
   struct Map {
      Tiles tiles;
      std::unique_ptr<TCODMap> model = std::make_unique<TCODMap>(MAP_WIDTH, MAP_HEIGHT);
   } map;
   
   int level;
   bool bossDead {false};
   
   Position player;
   int heroMana = MANA_BLIP_SIZE*5;
   int monsterMana = MANA_BLIP_SIZE*5;
   int worldMana = MANA_BLIP_SIZE*5;

   std::vector<Monster> monsterList;
   std::unique_ptr<Hero> hero;

   std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH> cloud;
   Position illusion;

   struct Message {
      std::string text;
      MessageType type;
   };

   std::list<Message> messageList;
   static constexpr int MESSAGE_COUNT = 28;

   private:
   static const std::unordered_map<const Tile, const std::pair<bool, bool>> tileProperties;
};

#endif