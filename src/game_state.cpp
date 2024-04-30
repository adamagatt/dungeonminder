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

void GameState::createLevel(int level, bool isLastLevel) {
   illusion = {-1, -1};

   Position heroPos = map.createLevel(level, isLastLevel);

   hero->resetForNewLevel(heroPos);
   player.resetForNewLevel(heroPos.offset(0, -1));

   if (isLastLevel) {
      generateMonsters(3, 1);
      createEndBoss();
   } else {
      generateMonsters(level, 3);
   }
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

void GameState::addMonster(const MonsterType& type, const Position& pos, bool portalSpawned) {
   monsterList.emplace_back(type, pos, portalSpawned);
   setTile(pos, portalSpawned ? Tile::PORTAL : Tile::MONSTER);
}

void GameState::addSpecifiedMonster(const Position& pos, int number, bool portalSpawned) {
   if (number < 0 || number >= MONSTER_TYPE_COUNT) return;
   const MonsterType& type = MONSTER_TYPES[number];

   addMonster(type, pos, portalSpawned);
}

void GameState::generateMonsters(int level, int monstersPerQuarter) {
   monsterList.clear();

   constexpr int halfMapWidth = (MAP_WIDTH-1)/2;
   constexpr int halfMapHeight = (MAP_HEIGHT-1)/2;

   // Loop for each quarter of the map
   for (int a = 0; a < 4; a++) {
      int minX = (a/2 == 0) ? 0 : halfMapWidth;
      int maxX = (a/2 == 0) ? halfMapWidth : (MAP_WIDTH-1);
      int minY = (a%2 == 0) ? 0 : halfMapHeight;
      int maxY = (a%2 == 0) ? halfMapHeight : (MAP_HEIGHT-1);
      
      for (int i = 0; i < monstersPerQuarter; i++) {
         Position temp {0, 0};
         while (tileAt(temp) != Tile::BLANK) {
            temp = {
               Utils::randGen->getInt(minX, maxX),
               Utils::randGen->getInt(minY, maxY)
            };
         }
         int randomMonster = Utils::randGen->getInt(level-1, level+3);
         addSpecifiedMonster(temp, randomMonster, false);
      }
   }
}

void GameState::createEndBoss() {
   Position bossPos = Utils::randomMapPosWithCondition(
      [this](const auto& pos){return tileAt(pos) == Tile::BLANK && Utils::dist(pos, hero->pos) >= 30;}
   );
   map.exitGoal = bossPos;

   switch (Utils::randGen->getInt(0, 2)) {
      case 0:
         addMonster(MasterSummonerType, bossPos, false); 
         addMessage("Master Summoner: You have come to your grave! I will bury you in monsters!", MessageType::VILLAIN);
         break;
      case 1:
         addMonster(NobleHeroType, bossPos, false); 
         addMessage("Noble Hero: I have made it to the treasure first, my boastful rival.", MessageType::VILLAIN);
         addMessage("Noble Hero: I wish you no harm, do not force me to defend myself.", MessageType::VILLAIN);
         break;
      default:
         addMonster(EvilMageType, bossPos, false); 
         addMessage("Evil Mage: How did you make it this far?! DIE!!!", MessageType::VILLAIN);
         break;
   }
}