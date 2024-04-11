#ifndef __DUNGEONMINDER_HPP_
#define __DUNGEONMINDER_HPP_

#include "libtcod.hpp"

#include "config.hpp"
#include "draw.hpp"
#include "game_state.hpp"
#include "hero.hpp"
#include "monster.hpp"

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std::string_literals;

bool fullscreen = false;

/** Displaying */
void displayRangedAttack(int, int, int, int);
void displayRangedAttack(const Position& p1, const Position& p2);

/** Player */
void presentUpgradeMenu();
bool castSpell(char spellChar);
bool effectSpell(Spell);

/** Monster */
void generateMonsters(int, int);
void monsterMove(Monster&);
bool castEffectSpellAtMonster(Condition, bool);
void generateEndBoss();

GameState state{};
Draw draw{state};

#endif