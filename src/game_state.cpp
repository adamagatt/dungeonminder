#include "game_state.hpp"

#include "config.hpp"
#include "hero.hpp"
#include "monster.hpp"

#include <algorithm>

GameState::GameState() : hero{std::make_unique<Hero>(*this)} {
   using namespace std::string_literals;

   addMessage("Welcome to the game!"s, MessageType::IMPORTANT);
   addMessage(""s, MessageType::NORMAL);
}

void GameState::addMessage(const std::string& message, MessageType type) {
   if (messageList.size() == MESSAGE_COUNT) {
      messageList.pop_front();
   }

   messageList.push_back({message, type});
}


const Tile& GameState::tileAt(const Position& pos) const {
    return map.tiles[pos.x][pos.y];
}

Tile& GameState::tileAt(const Position& pos) {
    return map.tiles[pos.x][pos.y];
}

void GameState::setTile(const Position& pos, Tile tile) {
   tileAt(pos) = tile;
   const auto& [visible, walkable] = tileProperties.at(tile);
   map.model->setProperties(pos.x, pos.y, visible, walkable);
}

bool GameState::isInFov(int x, int y) const {
   return map.model->isInFov(x, y);
}

bool GameState::isInFov(const Position& pos) const {
   return isInFov(pos.x, pos.y);
}

bool GameState::isEmptyPatch(int x, int y) const {
   return isEmptyPatch(Position(x, y));
}

bool GameState::isEmptyPatch(const Position& pos) const {
   if (pos.x <= 0 || pos.x >= MAP_WIDTH || pos.y <= 0 || pos.y >= MAP_HEIGHT)
      return false;

   return std::ranges::all_of(
      Utils::offsets,
      [this, &pos](const auto& offset){return tileAt(pos.offset(offset)) == Tile::BLANK;} 
   );
}

Monster* GameState::findMonster(const Position& p) {
   return findMonster(p.x, p.y);
}

Monster* GameState::findMonster(int x, int y) {
   auto found = std::ranges::find_if(
      monsterList,
      [x, y](const auto& monster) {return monster.pos.x == x && monster.pos.y == y;}
   );

   return (found != monsterList.end())
    ? &(*found)
    : nullptr;
}

Monster* GameState::heroFindMonster() {
   for (int i = 0; i < MAP_WIDTH; i++) {
      for (int j = 0; j < MAP_HEIGHT; j++) {
         if (map.model->isInFov(i, j) && map.tiles[i][j] == Tile::MONSTER) {
            return findMonster(i, j);
         }
      }
   }
   return nullptr;
}

bool GameState::hitMonster(int x, int y, int amount) {
   Monster* curMonster = findMonster(x, y);
   if (curMonster == nullptr)
      return false;

   curMonster->health -= amount;
   if (curMonster->health <= 0) {
      if (curMonster->symbol == '*' || curMonster->symbol=='M' || curMonster->symbol=='@') {
         bossDead = true;
      }
      addMessage("The " + curMonster->name + " dies!", MessageType::NORMAL);
      if (hero->target == curMonster) {
         hero->target = nullptr;
         hero->computePath();
      }
      tileAt(curMonster->pos) = Tile::BLANK;
      bool found = false;

      monsterList.erase(
         std::remove(monsterList.begin(), monsterList.end(), *curMonster),
         monsterList.end()
      );

      return true;

   } else if (curMonster->affectedBy(Condition::HALTED)) {
      curMonster->conditionTimers[Condition::HALTED] = 0;
      addMessage("The attack allows the "+curMonster->name + " to move again", MessageType::SPELL); 
   }
   return false;
}

bool GameState::hitMonster(const Position& pos, int amount) {
   return hitMonster(pos.x, pos.y, amount);
}

void GameState::addMonster(const std::string& name, char symbol, int x, int y, int health, int damage, bool ranged, const std::string& rangedName, float range, int wait, bool portalSpawned) {
   auto& curMonster = monsterList.emplace_back();
   curMonster.name = name;
   curMonster.symbol = symbol;
   curMonster.pos = {x, y};
   curMonster.health = health;
   curMonster.maxhealth = health;
   curMonster.range = range;
   curMonster.damage = damage;
   curMonster.wait = wait;
   curMonster.ranged = ranged;
   curMonster.rangedName = rangedName;
   curMonster.timer = 0;
   curMonster.angry = false;
   curMonster.maimed = false;
   curMonster.portalTimer = portalSpawned?PORTAL_TIME:0;
   for (auto& [condition, timer] : curMonster.conditionTimers) {
      timer = 0;
   }

   setTile({x, y}, portalSpawned ? Tile::PORTAL : Tile::MONSTER);
}

void GameState::addSpecifiedMonster(int tempx, int tempy, int number, bool portalSpawned) {
   switch (number){
      case 0:
         addMonster("rat", 'r', tempx, tempy, 3, 1, false, " ", 0.0f, 2, portalSpawned);
         break;
      case 1:
         addMonster("kobold", 'k', tempx, tempy, 4, 2, false, " ", 0.0f, 2, portalSpawned);
         break;
      case 2:
         addMonster("sprite", 's', tempx, tempy, 5, 1, false, " ", 0.0f, 1, portalSpawned);
         break;
      case 3:
         addMonster("dwarf", 'd', tempx, tempy, 9, 2, false, " ", 0.0f, 2, portalSpawned);
         break;
      case 4:
         addMonster("skeleton archer", 'a', tempx, tempy, 5, 1, true, "shoots an arrow", 5.0f, 3, portalSpawned);
         break;
      case 5:
         addMonster("ghost", 'g', tempx, tempy, 7, 2, false, "", 0.0f, 1, portalSpawned);
         break;
      case 6:
         addMonster("orc", 'o', tempx, tempy, 10, 3, true, "throws an axe", 6.0f, 3, portalSpawned);
         break;
      case 7:
         addMonster("ogre", 'O', tempx, tempy, 15, 4, false, "", 0.0f, 2, portalSpawned);
         break;
      case 8:
         addMonster("dragon", 'D', tempx, tempy, 20, 6, true, "breathes fire", 3.0f, 4, portalSpawned);
         break;
      case 9:
         addMonster("troll", 'T', tempx, tempy, 30, 2, false, " ", 0.0f, 2, portalSpawned);
         break;
      case 10:
         addMonster("wraith", 'W', tempx, tempy, 5, 1, true, "gazes", 12.0f, 2, portalSpawned);
         break;
      case 11:
         addMonster("golem", 'G', tempx, tempy, 40, 8, false, " ", 0.0f, 6, portalSpawned);
         break;
      case 12:
         addMonster("hunter", 'H', tempx, tempy, 5, 5, false, " ", 0.0f, 1, portalSpawned);
         break;
   }
}

const std::unordered_map<const Tile, const std::pair<bool, bool>> GameState::tileProperties {
   {Tile::BLANK, {true, true}},
   {Tile::WALL, {false, false}},
   {Tile::MONSTER, {true, true}},
   {Tile::STAIRS, {true, false}},
   {Tile::STAIRS_UP, {true, false}},
   {Tile::CHEST, {true, false}},
   {Tile::CHEST_OPEN, {true, false}},
   {Tile::FIELD, {false, false}},
   {Tile::PORTAL, {true, false}},
   {Tile::ILLUSION, {true, true}}
};