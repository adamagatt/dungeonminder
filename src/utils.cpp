#include "utils.hpp"

#include <algorithm>
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
   return isEmptyPatch(map, Position(x, y));
}

bool Utils::isEmptyPatch(const Map& map, const Position& pos) {
   if (pos.x <= 0 || pos.x >= MAP_WIDTH || pos.y <= 0 || pos.y >= MAP_HEIGHT)
      return false;

   return std::ranges::all_of(
      offsets,
      [&map, &pos](const auto& offset){return tileAt(map, pos.offset(offset)) == Tile::BLANK;} 
   );
}

constexpr std::array<Position, 9> Utils::offsets {{
    {-1, -1},
    {-1,  0},
    {-1, +1},
    { 0, -1},
    { 0,  0},
    { 0, +1},
    {+1, -1},
    {+1,  0},
    {+1, +1}
}};

TCODRandom* Utils::randGen = TCODRandom::getInstance();