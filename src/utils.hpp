#ifndef __UTILS_HPP_
#define __UTILS_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "position.hpp"

#include <functional>

namespace Utils {
    extern TCODRandom* randGen;

    using PositionPredicate = std::function<bool(const Position& pos)>;

    Position randomMapPosition(int border = 0);
    Position randomMapPosWithCondition(PositionPredicate pred, int border = 0);

    Tile& tileAt(Map& map, const Position& pos);
    const Tile& tileAt(const Map& map, const Position& pos);
    float dist(int x1, int y1, int x2, int y2);
    float dist(Position pos1, Position pos2);
    bool isEmptyPatch(const Map& map, int, int);
    bool isEmptyPatch(const Map& map, const Position& pos);
}

#endif