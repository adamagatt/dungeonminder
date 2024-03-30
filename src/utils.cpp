#include "utils.hpp"

#include <cmath>

float Utils::dist(int x1, int y1, int x2, int y2) {
    return std::sqrt(std::pow((x1-x2),2)+std::pow((y1-y2),2));
};

const Tile& Utils::tileAt(const Map& map, const Position& pos) {
    return map[pos.x][pos.y];
}

Tile& Utils::tileAt(Map& map, const Position& pos) {
    return map[pos.x][pos.y];
}

TCODRandom* Utils::randGen = TCODRandom::getInstance();