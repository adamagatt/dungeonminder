#include "position.hpp"

Position Position::offset(int dx, int dy) const {
    return {x+dx, y+dy};
}

Position Position::offset(const Position& rhs) const {
    return {x+rhs.x, y+rhs.y};
}

bool Position::operator==(const Position& rhs) const {
    return x == rhs.x && y == rhs.y;
}

Position Position::directionOffset(int direction) const {
    int diffX = ((direction-1)%3)-1;
    int diffY = 1-((direction-1)/3);
    return offset(diffX, diffY);
}

bool Position::withinMap() const {
    return x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT;
}