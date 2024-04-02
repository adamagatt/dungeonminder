#include "map.hpp"

#include "utils.hpp"

void Map::setTile(const Position& pos, Tile tile) {
   tileAt(pos) = tile;
   const auto& [visible, walkable] = tileProperties.at(tile);
   model->setProperties(pos.x, pos.y, visible, walkable);
}

void Map::createMap(int level) {
    // Creates the map
    for (int i = 0; i < MAP_WIDTH; i++) {
        cloud[i].fill(0);
        field[i].fill(0);
        tiles[i].fill(Tile::WALL);
    }
    model->clear();

    TCODBsp mapBSP (0,0,MAP_WIDTH,MAP_HEIGHT);
    if (level < 10) {
        mapBSP.splitRecursive(nullptr, 6, 5, 5, 1.5f, 1.5f);
    } else {
        mapBSP.splitRecursive(nullptr, 2, 5, 5, 1.5f, 1.5f);
    }
    drawBSP(&mapBSP);
    tileAt({0, 0}) = Tile::WALL;
}

const Tile& Map::tileAt(const Position& pos) const {
    return tiles[pos.x][pos.y];
}

Tile& Map::tileAt(const Position& pos) {
    return tiles[pos.x][pos.y];
}

void Map::drawBSP(TCODBsp* curBSP) {
   if (curBSP != nullptr) {
      if (curBSP->isLeaf()) {
         int x1 = curBSP->x;
         int x2 = curBSP->x+curBSP->w-1;
         int y1 = curBSP->y;
         int y2 = curBSP->y+curBSP->h-1;
         for (int i = x1+2; i <= x2-2; i++) {
            for (int j = y1+Utils::randGen->getInt(1, 2); j <= y2-Utils::randGen->getInt(1, 2); j++) {
               setTile({i, j}, Tile::BLANK);
            }
         }
         for (int j = y1+2; j <= y2-2; j++) {
            if (Utils::randGen->getInt(1, 2) == 1) {
               setTile({x1+1, j}, Tile::BLANK);
            }
            if (Utils::randGen->getInt(1, 2) == 1) {
               setTile({x2-1, j}, Tile::BLANK);
            }
         }
      } else {
         drawBSP(curBSP->getLeft());
         drawBSP(curBSP->getRight());
         if (!curBSP->horizontal) {
            int x1 = curBSP->getLeft()->x + curBSP->getLeft()->w/2;
            int x2 = curBSP->getRight()->x + curBSP->getRight()->w/2;
            int y = curBSP->y + curBSP->h/2;
            if (x1 > x2) {
               int temp = x1;
               x1 = x2;
               x2 = temp;
            }
            for (int i = x1; i <= x2; i++) {
               setTile({i, y},  Tile::BLANK);
            }
         } else {
            int y1 = curBSP->getLeft()->y + curBSP->getLeft()->h/2;
            int y2 = curBSP->getRight()->y + curBSP->getRight()->h/2;
            int x = curBSP->x + curBSP->w/2;
            if (y1 > y2) {
               int temp = y1;
               y1 = y2;
               y2 = temp;
            }
            for (int j = y1; j <= y2; j++) {
               setTile({x, j}, Tile::BLANK);
            }
         }
      }
   }
}

const std::unordered_map<const Tile, const std::pair<bool, bool>> Map::tileProperties {
   {Tile::BLANK, {true, true}},
   {Tile::WALL, {false, false}},
   {Tile::MONSTER, {true, true}},
   {Tile::STAIRS, {true, false}},
   {Tile::STAIRS_UP, {true, false}},
   {Tile::CHEST, {true, false}},
   {Tile::CHEST_OPEN, {true, false}},
   {Tile::FIELD, {false, false}},
   {Tile::PORTAL, {true, false}},
   {Tile::ILLUSION, {true, true}}
};