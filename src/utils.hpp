#ifndef __UTILS_HPP_
#define __UTILS_HPP_

#include "libtcod.hpp"

namespace Utils {
    extern TCODRandom* randGen;

    float dist(int x1, int y1, int x2, int y2);
}

#endif