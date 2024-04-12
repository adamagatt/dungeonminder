#include "game_state.hpp"

#include "config.hpp"
#include "hero.hpp"
#include "monster.hpp"

#include <algorithm>
#include <string>

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

void GameState::createMap() {
   map.createMap(level);
}

const Tile& GameState::tileAt(const Position& pos) const {
    return map.tileAt(pos);
}

Tile& GameState::tileAt(const Position& pos) {
    return map.tileAt(pos);
}

void GameState::setTile(const Position& pos, Tile tile) {
   map.setTile(pos, tile);
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


   const MonsterType* type = curMonster->type;
   curMonster->health -= amount;
   if (curMonster->health <= 0) {
      if (type->symbol == '*' || type->symbol=='M' || type->symbol=='@') {
         bossDead = true;
      }
      addMessage("The " + type->name + " dies!", MessageType::NORMAL);
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
      addMessage("The attack allows the " + type->name + " to move again", MessageType::SPELL); 
   }
   return false;
}

bool GameState::hitMonster(const Position& pos, int amount) {
   return hitMonster(pos.x, pos.y, amount);
}

void GameState::addMonster(const MonsterType& type, const Position& pos, bool portalSpawned) {
   monsterList.emplace_back(type, pos, portalSpawned);
   setTile(pos, portalSpawned ? Tile::PORTAL : Tile::MONSTER);
}

void GameState::addSpecifiedMonster(const Position& pos, int number, bool portalSpawned) {
   if (number < 0 || number >= MONSTER_TYPE_COUNT) return;
   const MonsterType& type = MONSTER_TYPES[number];

   addMonster(type, pos, portalSpawned);
}

int GameState::getHeroSpec() const {
   return player.heroSpec;
}
   
void GameState::setHeroSpec(int heroSpec) {
   player.heroSpec = heroSpec;
}

int GameState::getMonsterSpec() const {
   return player.monsterSpec;
}

void GameState::setMonsterSpec(int monsterSpec) {
   player.monsterSpec = monsterSpec;
}

int GameState::getWorldSpec() const {
   return player.worldSpec;
}

void GameState::setWorldSpec(int worldSpec) {
   player.worldSpec = worldSpec;
}