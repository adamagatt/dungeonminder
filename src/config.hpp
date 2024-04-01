#ifndef __CONFIG_HPP_
#define __CONFIG_HPP_

#include "libtcod.hpp"

#include <array>
#include <unordered_map>
#include <string>

constexpr int MENU_X = 7;
constexpr int MENU_Y = 7;
constexpr int TOP = 2;
constexpr int LEFT = 4;
constexpr int BOTTOM = 46;
constexpr int RIGHT = 75;
constexpr int MAP_WIDTH = RIGHT-LEFT+1;
constexpr int MAP_HEIGHT = BOTTOM-TOP+1;

constexpr int MAX_MONSTERS = 20;
constexpr int PORTAL_TIME = 20;

constexpr int MANA_BLIP_SIZE = 10;

// Conditions
const int CONDITION_COUNT = 7;
enum class Condition {
    SLEEPING = 0,
    BLINDED = 1,
    RAGED = 2,
    ALLIED = 3,
    WEAKENED = 4,
    HALTED = 5,
    FLEEING = 6
};

const std::unordered_map<Condition, int> CONDITION_TIMES {
   {Condition::SLEEPING, 15},
   {Condition::BLINDED, 15},
   {Condition::RAGED, 10},
   {Condition::ALLIED, 12},
   {Condition::WEAKENED, 10},
   {Condition::HALTED, 15},
   {Condition::FLEEING, 18}
};

const std::unordered_map<Condition, std::pair<std::string, std::string>> CONDITION_START {
   {Condition::SLEEPING, {"The "," becomes soundly asleep!"}},
   {Condition::BLINDED, {"The "," stumbles around blindly!"}},
   {Condition::RAGED, {"The "," starts foaming at the mouth!"}},
   {Condition::ALLIED, {"A strange look comes over the ","!"}},
   {Condition::WEAKENED, {"The "," looks slightly frailer!"}},
   {Condition::HALTED, {"The "," appears to be stuck to the ground!"}},
   {Condition::FLEEING, {"The "," trembles in fear!"}}
};

const std::unordered_map<Condition, std::pair<std::string, std::string>> CONDITION_END {
   {Condition::SLEEPING, {"The ", " awakens from its slumber"}},
   {Condition::BLINDED, {"The ", " rubs its eyes and looks around"}},
   {Condition::RAGED, {"The ", " stops foaming at the mouth"}},
   {Condition::ALLIED, {"The strange look leaves the ", ""}},
   {Condition::WEAKENED, {"The ", " appears to be stronger now"}},
   {Condition::HALTED, {"The ", " is no longer stuck to the ground"}},
   {Condition::FLEEING, {"The ", " gathers up its resolve"}}
};

constexpr int ITEM_COUNT = 10;
enum class Item {
    magicResist,
    healthCap,
    slowBoots,
    monsterHelm,
    rustedSword,
    scrollEarthquake,
    scrollSeeInvisible,
    scrollSummonMonsters,
    beltTrapAttraction,
    carelessGauntlets
};

const std::unordered_map<Item, std::string> ITEM_NAME {
   {Item::magicResist, "the Amulet of Magic Resistance"},
   {Item::healthCap, "the Health-Capping Armour"},
   {Item::slowBoots, "the Boots of Slowness"},
   {Item::monsterHelm, "the Helm of Monster Attraction"},
   {Item::rustedSword, "a blunt, rusted sword"},
   {Item::scrollEarthquake, "a scroll: Cast Earthquake"},
   {Item::scrollSeeInvisible, "a scroll: See Invisible"},
   {Item::scrollSummonMonsters, "a scroll: Summon Monsters"},
   {Item::beltTrapAttraction, "the Belt of Trap Attraction"},
   {Item::carelessGauntlets, "the Gauntlets of Over-Exertion"}
};

// Spell times
constexpr int MEDITATION_TIME = 10;
constexpr int PACIFISM_TIME = 25;
constexpr int SPEED_TIME = 10;
constexpr int BLINK_MOVES = 8;
constexpr int SHIELD_TIME = 10;
constexpr int REGEN_TIME = 6;
constexpr int CLOUD_TIME = 18;
constexpr int FIELD_TIME = 32;

enum class Spell {
   NONE,
   PACIFISM,
   SPEED,
   HEAL,
   BLIND,
   RAGE,
   SLEEP,
   CLEAR,
   CLOUD,
   MTRAP,
   MEDITATION,
   CHARITY,
   SLOW,
   BLINK,
   SHIELD,
   REGENERATE,
   MAIM,
   CRIPPLE,
   WEAKEN,
   ALLY,
   HALT,
   FLEE,
   BLOW,
   MILLUSION,
   SCREEN,
   MFIELD,
   TUNNEL,
   MINEFIELD
};

constexpr std::array<std::array<std::array<Spell, 3>, 4>, 3> spellLists {{
   {{
      {Spell::PACIFISM, Spell::SPEED, Spell::HEAL},
      {Spell::PACIFISM, Spell::MEDITATION, Spell::CHARITY},
      {Spell::SLOW, Spell::SPEED, Spell::BLINK},
      {Spell::SHIELD, Spell::REGENERATE, Spell::HEAL}
   }}, {{
      {Spell::BLIND, Spell::RAGE, Spell::SLEEP},
      {Spell::BLIND, Spell::MAIM, Spell::CRIPPLE},
      {Spell::WEAKEN, Spell::RAGE, Spell::ALLY},
      {Spell::HALT, Spell::FLEE, Spell::SLEEP}
   }}, {{
      {Spell::CLEAR, Spell::CLOUD, Spell::MTRAP},
      {Spell::CLEAR, Spell::BLOW, Spell::MILLUSION},
      {Spell::SCREEN, Spell::CLOUD, Spell::MFIELD},
      {Spell::TUNNEL, Spell::MINEFIELD, Spell::MTRAP}
   }}
}};

constexpr int MESSAGE_COUNT = 28;
enum class MessageType {
   NORMAL,
   HERO,
   SPELL,
   VILLAIN,
   IMPORTANT
};

const std::unordered_map<MessageType, TCODColor> MESSAGE_COLOR {
   {MessageType::NORMAL, TCODColor::white},
   {MessageType::HERO, TCODColor::lightBlue},
   {MessageType::SPELL, TCODColor::yellow},
   {MessageType::VILLAIN, TCODColor::red},
   {MessageType::IMPORTANT, TCODColor(255, 100, 230)}
};

enum class Tile {
    BLANK,
    PLAYER,
    HERO,
    MONSTER,
    WALL,
    STAIRS,
    STAIRS_UP,
    TRAP,
    CHEST,
    CHEST_OPEN,
    ILLUSION,
    FIELD,
    PORTAL,
    MARKER1,
    MARKER2
};

using Map = std::array<std::array<Tile, MAP_HEIGHT>, MAP_WIDTH>;

#endif