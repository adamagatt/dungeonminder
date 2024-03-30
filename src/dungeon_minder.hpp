#ifndef __DUNGEONMINDER_HPP_
#define __DUNGEONMINDER_HPP_

#include "libtcod.hpp"

#include "config.hpp"
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

/* GLOBAL VARIABLE DECLARATIONS */

// Relating to the player
int destx = 0, desty = 0;

// Relating to messages and the message list
std::array<std::string, MESSAGE_COUNT> messageList;
std::array<MessageType, MESSAGE_COUNT> messageType;

char charBuffer[20];

// Relating to spells
constexpr int manaBlipSize = 10;
int heroMana = manaBlipSize*5, monsterMana = manaBlipSize*5, worldMana = manaBlipSize*5;

// Relating to spell specialisation
int heroSpec;
int monsterSpec;
int worldSpec;

// Relating to the map 
bool fullscreen = false;
int cloud [MAP_WIDTH][MAP_HEIGHT];
int field [MAP_WIDTH][MAP_HEIGHT];
float floorNoise [MAP_WIDTH][MAP_HEIGHT];
float wallNoise [MAP_WIDTH][MAP_HEIGHT];

TCODConsole * spellMenu;

// Relating to the different game modes
bool mode_specialisation = false;
bool mode_hero_levels = false;

/** Displaying */
void drawScreen();
void drawCommonGUI();
bool displaySpellMenu(char);
void displayUpgradeMenu();
void addMessage(const std::string&, MessageType);
void displayMessageHistory();
void displayStatLine(int);
void displayRangedAttack(int, int, int, int);
void showVictoryScreen();

/** Initialisation */
void drawBSP(TCODBsp*);
void createSpellMenu();

/** Player */
bool castSpell(Spell);

/** Monster */
void generateMonsters(int, int);
void monsterMove(Monster&);
bool applyMonsterCondition(Condition, bool);
void generateEndBoss();

/** Utility */
bool isEmptyPatch(int, int);
int getDirection();
TCOD_key_t getKeyPress();

GameState state{addMessage, drawScreen};

class WithBackgroundSet {
   public:
   WithBackgroundSet(TCODConsole& console) : console(console) {
      console.setBackgroundFlag(TCOD_BKGND_SET);
   }

   ~WithBackgroundSet() {
      console.setBackgroundFlag(TCOD_BKGND_NONE);
   }

   private:
   TCODConsole& console;
};

#endif