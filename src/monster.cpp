#include "monster.hpp"

Monster::Monster() {
    for(int i = 0; i < CONDITION_COUNT; ++i) {
        conditionTimers[static_cast<Condition>(i)] = 0;
    }
}

bool Monster::operator==(const Monster& other) const {
    return name == other.name && pos == other.pos;
}

bool Monster::affectedBy(Condition condition) const {
    auto it = conditionTimers.find(condition);
    return it != conditionTimers.end() && it->second != 0;
}