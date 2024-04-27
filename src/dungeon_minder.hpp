#ifndef __DUNGEONMINDER_HPP_
#define __DUNGEONMINDER_HPP_

#include "config.hpp"
#include "draw.hpp"
#include "game_state.hpp"

void presentUpgradeMenu();
bool castSpell(char spellChar, int level, bool isLastLevel);
bool effectSpell(Spell chosenSpell, int level, bool isLastLevel);
bool castEffectSpellAtMonster(Condition curCondition, bool append);

GameState state{};
Draw draw{state};

#endif