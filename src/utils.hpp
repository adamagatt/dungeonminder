#ifndef __UTILS_HPP_
#define __UTILS_HPP_

#include "libtcod.hpp"

#include "config.hpp"

namespace Utils {
    extern TCODRandom* randGen;

    Tile& tileAt(Map& map, const Position& pos);
    const Tile& tileAt(const Map& map, const Position& pos);
    float dist(int x1, int y1, int x2, int y2);
}

#endif