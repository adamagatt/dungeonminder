#include "game_state.hpp"
#include "config.hpp"
#include "hero.hpp"
#include "monster.hpp"

GameState::GameState(const MessageCallback& message) :
    message(message),
    hero{std::make_unique<Hero>(*this, message)}
{ }

Monster* GameState::findMonster(int x, int y) {
   auto found = std::ranges::find_if(
      monsterList,
      [x, y](const auto& monster) {return monster.x == x && monster.y == y;}
   );

   return (found != monsterList.end())
    ? &(*found)
    : nullptr;
}

Monster* GameState::heroFindMonster() {
   for (int i = 0; i < MAP_WIDTH; i++) {
      for (int j = 0; j < MAP_HEIGHT; j++) {
         if (mapModel->isInFov(i, j) && map[i][j] == MONSTER) {
            return findMonster(i, j);
         }
      }
   }
   return nullptr;
}

void GameState::hitMonster(int x, int y, int amount) {
   if (Monster* curMonster = findMonster(x, y); curMonster != nullptr) {
      curMonster->health -= amount;
      if (curMonster->health <= 0) {
         if (curMonster->symbol == '*' || curMonster->symbol=='M' || curMonster->symbol=='@') {
            bossDead = true;
         }
         message("The " + curMonster->name + " dies!", MessageType::NORMAL);
         if (hero->target == curMonster) {
            hero->target = nullptr;
            hero->computePath();
         }
         map[curMonster->x][curMonster->y] = BLANK;
         bool found = false;

         for (auto it = monsterList.begin(); it != monsterList.end(); ++it) {
            if (&(*it) == curMonster) {
               curMonster == nullptr;
               monsterList.erase(it);
               break;
            }
         }
      } else if (curMonster->conditionTimers[Condition::HALTED] > 0) {
         curMonster->conditionTimers[Condition::HALTED] = 0;
         message("The attack allows the "+curMonster->name + " to move again", MessageType::SPELL); 
      }
   }
}

void GameState::addMonster(const std::string& name, char symbol, int x, int y, int health, int damage, bool ranged, const std::string& rangedName, float range, int wait, bool portalSpawned) {
   auto& curMonster = monsterList.emplace_back();
   curMonster.name = name;
   curMonster.symbol = symbol;
   curMonster.x = x;
   curMonster.y = y;
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
   if (portalSpawned) {
      map[x][y] = PORTAL;
      mapModel->setProperties(x, y, true, false);
   } else {
      map[x][y] = MONSTER;
   }
}