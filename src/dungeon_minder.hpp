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

// Relating to spell specialisation
int heroSpec;
int monsterSpec;
int worldSpec;

// Relating to the map 
bool fullscreen = false;
int field [MAP_WIDTH][MAP_HEIGHT];

// Relating to the different game modes
bool mode_specialisation = false;
bool mode_hero_levels = false;

/** Displaying */
void displayRangedAttack(int, int, int, int);
void displayRangedAttack(const Position& p1, const Position& p2);

/** Initialisation */
void drawBSP(TCODBsp*);

/** Player */
bool castSpell(char spellChar);
bool effectSpell(Spell);

/** Monster */
void generateMonsters(int, int);
void monsterMove(Monster&);
bool applyMonsterCondition(Condition, bool);
void generateEndBoss();

GameState state{};
Draw draw{state};

#endif