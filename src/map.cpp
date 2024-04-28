#include "map.hpp"

#include "utils.hpp"

void Map::setTile(const Position& pos, Tile tile) {
   tileAt(pos) = tile;
   const auto& [visible, walkable] = tileProperties.at(tile);
   model->setProperties(pos.x, pos.y, visible, walkable);
}

Position Map::createLevel(int level, bool isLastLevel) {
   createMapLayout(level);

   Position heroPos = Utils::randomMapPosWithCondition(
      [this](const auto& pos){return isEmptyPatch(pos);},
      2
   );

   // Stairs is above hero and player is below
   setTile(heroPos.offset(0, 1), Tile::STAIRS_UP);
   setTile(heroPos, Tile::HERO);
   setTile(heroPos.offset(0, -1), Tile::PLAYER);

   if (!isLastLevel) {
      // Place the stairs
      Position stairsPos = Utils::randomMapPosWithCondition(
         [this, &heroPos](const auto& pos){return isEmptyPatch(pos) && Utils::dist(heroPos, pos) >= 20;}
      );
      setTile(stairsPos, Tile::STAIRS);
      exitGoal = stairsPos.offset(0, 1);
      
      // Place the chests
      Position chest1Pos = Utils::randomMapPosWithCondition([this, &heroPos](const auto& pos){
         return isEmptyPatch(pos) &&
            Utils::dist(heroPos, pos) >= 20 &&
            Utils::dist(exitGoal, pos) >= 20;
      });
      setTile(chest1Pos, Tile::CHEST);
      chest1Goal = chest1Pos.offset(0, 1);

      Position chest2Pos = Utils::randomMapPosWithCondition([this, &heroPos](const auto& pos){
         return isEmptyPatch(pos) &&
            Utils::dist(heroPos, pos) >= 20 &&
            Utils::dist(chest1Goal, pos) >= 20 &&
            Utils::dist(exitGoal, pos) >= 20;
      });
      setTile(chest2Pos, Tile::CHEST);
      chest2Goal = chest2Pos.offset(0, 1);
   }

   // PLACE TRAPS
   for (int i = 0; i < 10; i++) {
      Position trapPos = Utils::randomMapPosWithCondition(
         [this](const auto& pos){return tileAt(pos) == Tile::BLANK;}
      );
      tileAt(trapPos) = Tile::TRAP;
   }

   return heroPos;
}

void Map::createMapLayout(int level) {
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

bool Map::isEmptyPatch(int x, int y) const {
   return isEmptyPatch(Position(x, y));
}

bool Map::isEmptyPatch(const Position& pos) const {
   if (pos.x <= 0 || pos.x >= MAP_WIDTH || pos.y <= 0 || pos.y >= MAP_HEIGHT)
      return false;

   return std::ranges::all_of(
      Utils::offsets,
      [this, &pos](const auto& offset){return tileAt(pos.offset(offset)) == Tile::BLANK;} 
   );
}

void Map::openChest1() {
   tileAt(chest1Goal.offset(0, -1)) = Tile::CHEST_OPEN;
}

void Map::openChest2() {
   tileAt(chest2Goal.offset(0, -1)) = Tile::CHEST_OPEN;
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
   {Tile::ILLUSION, {true, true}},
   {Tile::HERO, {true, true}},
   {Tile::PLAYER, {true, true}}
};