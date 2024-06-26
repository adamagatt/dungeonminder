#ifndef __MAP_HPP_
#define __MAP_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "position.hpp"

#include <array>
#include <memory>
#include <unordered_map>
#include <utility>

class Map {
   public:
   using Tiles = std::array<std::array<Tile, MAP_HEIGHT>, MAP_WIDTH>;


   Position createLevel(int level, bool isLastLevel);
   void createMapLayout(int level);
   void setTile(const Position& pos, Tile tile);
   [[nodiscard]] Tile& tileAt(const Position& pos);
   [[nodiscard]] const Tile& tileAt(const Position& pos) const;
   [[nodiscard]] bool isEmptyPatch(int x, int y) const;
   [[nodiscard]] bool isEmptyPatch(const Position& pos) const;

   void openChest1();
   void openChest2();

   Tiles tiles;
   std::unique_ptr<TCODMap> model = std::make_unique<TCODMap>(MAP_WIDTH, MAP_HEIGHT);
   std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH> field;
   std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH> cloud;

   Position chest1Goal;
   Position chest2Goal;
   Position exitGoal;

   private:
   void drawBSP(TCODBsp*);

   static const std::unordered_map<const Tile, const std::pair<bool, bool>> tileProperties;
};

#endif