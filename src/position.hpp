#ifndef __POSITION_HPP_
#define __POSITION_HPP_

#include "config.hpp"

struct Position {
   int x;
   int y;

   Position offset(int dx, int dy) const;

   Position offset(const Position& rhs) const;

   bool operator==(const Position& rhs) const;

   Position directionOffset(int direction) const;

   bool withinMap() const;
};

#endif