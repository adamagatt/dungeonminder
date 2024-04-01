#include "utils.hpp"

#include <SDL2/SDL.h>

#include "draw.hpp"

#include <algorithm>
#include <cmath>

TCOD_key_t Utils::getKeyPress() {
   TCOD_key_t key;
   SDL_Event event;

   for (; event.type != SDL_KEYDOWN; SDL_WaitEvent(&event)) {}
   tcod::sdl2::process_event(event, key);  // Convert a SDL key to a libtcod key event, to help port older code.

   return key;
}

int Utils::getDirection() {
   Draw::directionScreen();

   // Get user response
   TCOD_key_t key = getKeyPress();
   if (key.vk == TCODK_UP || key.vk == TCODK_KP8 || key.c == 'k') {
      return 8;
   } else if (key.vk == TCODK_DOWN || key.vk == TCODK_KP2 || key.c == 'j') {
      return 2;
   } else if (key.vk == TCODK_LEFT || key.vk == TCODK_KP4 || key.c == 'h') {
      return 4;
   } else if (key.vk == TCODK_RIGHT || key.vk == TCODK_KP6 || key.c == 'l') {
      return 6;
   } else if (key.vk == TCODK_KP7 || key.c == 'y') {
      return 7;
   } else if (key.vk == TCODK_KP9 || key.c == 'u') {
      return 9;
   } else if (key.vk == TCODK_KP3 || key.c == 'n') {
      return 3;
   } else if (key.vk == TCODK_KP1 || key.c == 'b') {
      return 1;
   }

   return 0;
}

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

Utils::WithBackgroundSet::WithBackgroundSet(TCODConsole& console) : console(console) {
   console.setBackgroundFlag(TCOD_BKGND_SET);
}

Utils::WithBackgroundSet::~WithBackgroundSet() {
   console.setBackgroundFlag(TCOD_BKGND_NONE);
}

TCODRandom* Utils::randGen = TCODRandom::getInstance();