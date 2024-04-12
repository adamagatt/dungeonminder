#ifndef __DUNGEONMINDER_HPP_
#define __DUNGEONMINDER_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "draw.hpp"
#include "game_state.hpp"

/** Player */
void presentUpgradeMenu();
bool castSpell(char spellChar, int level, bool isLastLevel);
bool effectSpell(Spell chosenSpell, int level, bool isLastLevel);

/** Monster */
void generateMonsters(int, int);
bool castEffectSpellAtMonster(Condition, bool);
void generateEndBoss();

GameState state{};
Draw draw{state};

#endif