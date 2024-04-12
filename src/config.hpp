#ifndef __CONFIG_HPP_
#define __CONFIG_HPP_

#include "libtcod.hpp"

#include <array>
#include <optional>
#include <string>
#include <unordered_map>

using namespace std::string_literals;

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
constexpr int CONDITION_COUNT = 7;
enum class Condition {
    SLEEPING,
    BLINDED,
    RAGED,
    ALLIED,
    WEAKENED,
    HALTED,
    FLEEING
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
    PORTAL
};

struct RangedAttack {float distance; std::string description;};

struct MonsterType {
   std::string name;
   char symbol;
   int maxHealth;
   int damage;
   int wait;
   std::optional<RangedAttack> rangedAttack;

   inline bool operator==(const MonsterType& other) {
      return name == other.name;
   }
};

constexpr int MONSTER_TYPE_COUNT = 13;

const std::array<MonsterType, MONSTER_TYPE_COUNT> MONSTER_TYPES {{
   {"rat",             'r',  3, 1, 2, std::nullopt},
   {"kobold",          'k',  4, 2, 2, std::nullopt},
   {"sprite",          's',  5, 1, 1, std::nullopt},
   {"dwarf",           'd',  9, 2, 2, std::nullopt},
   {"skeleton archer", 'a',  5, 1, 3, RangedAttack{5.0f, "shoots an arrow"s}},
   {"ghost",           'g',  7, 2, 1, std::nullopt},
   {"orc",             'o', 10, 3, 3, RangedAttack{6.0f, "throws an axe"s}},
   {"ogre",            'O', 15, 4, 2, std::nullopt},
   {"dragon",          'D', 20, 6, 4, RangedAttack{3.0f, "breathes fire"s}},
   {"troll",           'T', 30, 2, 2, std::nullopt},
   {"wraith",          'W',  5, 1, 2, RangedAttack{12.0f, "gazes"s}},
   {"golem",           'G', 40, 8, 6, std::nullopt},
   {"hunter",          'H',  5, 5, 1, std::nullopt}
}};

const MonsterType MasterSummonerType {"Master Summoner", '*', 20, 2, 15, std::nullopt};
const MonsterType NobleHeroType {"Noble Hero", '@', 30, 3, 2, std::nullopt};
const MonsterType EvilMageType {"Evil Mage", 'M', 15, 4, 5, RangedAttack{20.0f, "shoots lightning"s}};

#endif