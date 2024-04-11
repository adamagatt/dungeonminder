#ifndef __DUNGEONMINDER_HPP_
#define __DUNGEONMINDER_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "draw.hpp"
#include "game_state.hpp"

using namespace std::string_literals;

bool fullscreen = false;

/** Player */
void presentUpgradeMenu();
bool castSpell(char spellChar);
bool effectSpell(Spell);

/** Monster */
void generateMonsters(int, int);
bool castEffectSpellAtMonster(Condition, bool);
void generateEndBoss();

GameState state{};
Draw draw{state};

#endif