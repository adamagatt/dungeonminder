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

   void createMap(int level);
   void setTile(const Position& pos, Tile tile);
   [[nodiscard]] Tile& tileAt(const Position& pos);
   [[nodiscard]] const Tile& tileAt(const Position& pos) const;

   Tiles tiles;
   std::unique_ptr<TCODMap> model = std::make_unique<TCODMap>(MAP_WIDTH, MAP_HEIGHT);
   std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH> field;
   std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH> cloud;

   private:
   void drawBSP(TCODBsp*);

   static const std::unordered_map<const Tile, const std::pair<bool, bool>> tileProperties;
};

#endif