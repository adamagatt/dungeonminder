#include "utils.hpp"

#include <cmath>

int Utils::signum(int num) {
    return (num > 0) ? 1 :
           (num < 0) ? -1 :
           0;
}

Position Utils::randomMapPosition(int border) {
    return {
        randGen->getInt(border, MAP_WIDTH-1-border),
        randGen->getInt(border, MAP_HEIGHT-1-border)
    };
}

Position Utils::randomMapPosWithCondition(PositionPredicate pred, int border) {
    Position temp = randomMapPosition(border);
    for (; !pred(temp); temp = randomMapPosition(border)) { }
    return temp;   
}

float Utils::dist(int x1, int y1, int x2, int y2) {
    return std::sqrt(std::pow((x1-x2),2)+std::pow((y1-y2),2));
};

float Utils::dist(Position pos1, Position pos2) {
    return std::sqrt(std::pow((pos1.x-pos2.x),2)+std::pow((pos1.y-pos2.y),2));
};

const Tile& Utils::tileAt(const Map& map, const Position& pos) {
    return map[pos.x][pos.y];
}

Tile& Utils::tileAt(Map& map, const Position& pos) {
    return map[pos.x][pos.y];
}

bool Utils::isEmptyPatch(const Map& map, int x, int y) {
   if (x <= 0 || x >= MAP_WIDTH || y <= 0 || y >= MAP_HEIGHT)
      return false;

   return map[x-1][y-1] == Tile::BLANK &&
          map[x-1][y] == Tile::BLANK &&
          map[x-1][y+1] == Tile::BLANK &&
          map[x][y-1] == Tile::BLANK &&
          map[x][y] == Tile::BLANK &&
          map[x][y+1] == Tile::BLANK &&
          map[x+1][y-1] == Tile::BLANK &&
          map[x+1][y] == Tile::BLANK &&
          map[x+1][y+1] == Tile::BLANK;
}

bool Utils::isEmptyPatch(const Map& map, const Position& pos) {
   return isEmptyPatch(map, pos.x, pos.y);
}

TCODRandom* Utils::randGen = TCODRandom::getInstance();