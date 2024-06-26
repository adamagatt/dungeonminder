#ifndef __UTILS_HPP_
#define __UTILS_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "position.hpp"

#include <functional>

namespace Utils {
    extern TCODRandom* randGen;

    using PositionPredicate = std::function<bool(const Position& pos)>;

    int getDirection();
    TCOD_key_t getKeyPress();

    int signum(int num);

    Position randomMapPosition(int border = 0);
    Position randomMapPosWithCondition(PositionPredicate pred, int border = 0);
    
    float dist(int x1, int y1, int x2, int y2);
    float dist(Position pos1, Position pos2);

    extern const std::array<Position, 9> offsets;

    class WithBackgroundSet {
        public:
        WithBackgroundSet(TCODConsole& console);
        ~WithBackgroundSet();

        private:
        TCODConsole& console;
    };

    template <typename T, typename U, U N>
    const T& randomFrom(const std::array<T, N>&  input) {
        return input[Utils::randGen->getInt(0, N-1)];
    };    
}

#endif