#include "monster.hpp"

Monster::Monster() {
    for(int i = 0; i < CONDITION_COUNT; ++i) {
        conditionTimers[static_cast<Condition>(i)] = 0;
    }
}