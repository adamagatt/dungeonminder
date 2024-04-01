#include "game_state.hpp"
#include "config.hpp"
#include "hero.hpp"
#include "monster.hpp"

GameState::GameState() :
    hero{std::make_unique<Hero>(*this)}
{
   // Initialise messageList
   for (int i = 0; i < MESSAGE_COUNT-2; i++) {
      messageList[i] = "";
      messageType[i] = MessageType::NORMAL;
   }
   messageList[MESSAGE_COUNT-2] = "Welcome to the game!";
   messageType[MESSAGE_COUNT-2] = MessageType::IMPORTANT;
   messageList[MESSAGE_COUNT-1] = "";
   messageType[MESSAGE_COUNT-1] = MessageType::NORMAL;
}

void GameState::addMessage(const std::string& message, MessageType type) {
   for (int i = 0; i < MESSAGE_COUNT-1; i++) {
      messageList[i] = messageList[i+1];
      messageType[i] = messageType[i+1];
   }
   messageList[MESSAGE_COUNT-1] = message;
   messageType[MESSAGE_COUNT-1] = type;
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
         if (mapModel->isInFov(i, j) && map[i][j] == Tile::MONSTER) {
            return findMonster(i, j);
         }
      }
   }
   return nullptr;
}

void GameState::hitMonster(int x, int y, int amount) {
   Monster* curMonster = findMonster(x, y);
   if (curMonster == nullptr)
      return;

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
      Utils::tileAt(map, curMonster->pos) = Tile::BLANK;
      bool found = false;

      monsterList.erase(
         std::remove(monsterList.begin(), monsterList.end(), *curMonster),
         monsterList.end()
      );

   } else if (curMonster->conditionTimers[Condition::HALTED] > 0) {
      curMonster->conditionTimers[Condition::HALTED] = 0;
      addMessage("The attack allows the "+curMonster->name + " to move again", MessageType::SPELL); 
   }
}

void GameState::hitMonster(const Position& pos, int amount) {
   hitMonster(pos.x, pos.y, amount);
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
   if (portalSpawned) {
      map[x][y] = Tile::PORTAL;
      mapModel->setProperties(x, y, true, false);
   } else {
      map[x][y] = Tile::MONSTER;
   }
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