#include "dungeon_minder.hpp"

#include "position.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

// Main Method
int main() {
   // Creates the console
   TCODConsole::initRoot(80,60,"DungeonMinder",false);
   TCODSystem::setFps(25);

   heroSpec = 0;
   monsterSpec = 0;
   worldSpec = 0;

gameLoop:
   for (state.level = 1; state.level <= 10 && !TCODConsole::isWindowClosed(); state.level++) {
      if (state.level%3 == 0) {
         draw.upgradeMenu(heroSpec, monsterSpec, worldSpec);
      }
      bool nextlevel = false;
      state.addMessage("Hero: " + Hero::heroEntry[Utils::randGen->getInt(0, 4)], MessageType::HERO);

      draw.generateMapNoise();

      // Creates the map
      for (int i = 0; i < MAP_WIDTH; i++) {
         for (int j = 0; j < MAP_HEIGHT; j++) {
            Utils::tileAt(state.map, {i, j}) = Tile::WALL;
            state.cloud[i][j] = 0;
            field[i][j] = 0;
         }
      }
      TCODBsp mapBSP (0,0,MAP_WIDTH,MAP_HEIGHT);
      if (state.level < 10) {
         mapBSP.splitRecursive(nullptr, 6, 5, 5, 1.5f, 1.5f);
      } else {
         mapBSP.splitRecursive(nullptr, 2, 5, 5, 1.5f, 1.5f);
      }
      state.mapModel->clear();
      drawBSP(&mapBSP);
      Utils::tileAt(state.map, {0, 0}) = Tile::WALL;

      // Initialise the hero and player
      Position heroPos = Utils::randomMapPosWithCondition(
         [](const auto& pos){return Utils::isEmptyPatch(state.map, pos);},
         2
      );

      Position stairsPos = heroPos.offset(0, 1);
      Utils::tileAt(state.map, stairsPos) = Tile::STAIRS_UP;
      state.mapModel->setProperties(stairsPos.x, stairsPos.y+1, true, false);

      state.player = heroPos.offset(0, -1);
      Hero& hero = *(state.hero);
      Utils::tileAt(state.map, state.player) = Tile::PLAYER;
      hero.pos = heroPos;
      Utils::tileAt(state.map, hero.pos) = Tile::HERO;
      hero.health = 10;
      hero.damage = 5;
      hero.timer = 1;
      hero.wait = 2;
      hero.hasteTimer = 0;
      hero.meditationTimer = 0;
      hero.seeInvisibleTimer = 0;
      hero.slow = false;
      hero.blinking = false;
      hero.regenTimer = 0;
      hero.shieldTimer = 0;
      hero.pacifismTimer = 0;
      hero.target = nullptr;

      hero.pathstep = 0;
      hero.dead = false;
      hero.dest1 = {-1, -1};
      hero.dest2 = {-1, -1};
      hero.items.clear();
      state.heroMana = 5*MANA_BLIP_SIZE;
      state.monsterMana = 5*MANA_BLIP_SIZE;
      state.worldMana = 5*MANA_BLIP_SIZE;
      state.illusion.x = -1; state.illusion.y = -1;

      if (state.level < 10) {
         // Place the stairs
         Position stairsPos = Utils::randomMapPosWithCondition(
            [&hero](const auto& pos){return Utils::isEmptyPatch(state.map, pos) && Utils::dist(hero.pos, pos) >= 20;}
         );
         Utils::tileAt(state.map, stairsPos) = Tile::STAIRS;
         hero.stairs = stairsPos.offset(0, 1);
         state.mapModel->setProperties(stairsPos.x, stairsPos.y, true, false);

         // Place the chests
         Position chest1Pos = Utils::randomMapPosWithCondition([&hero](const auto& pos){
            return Utils::isEmptyPatch(state.map, pos) &&
                   Utils::dist(hero.pos, pos) >= 20 &&
                   Utils::dist(hero.stairs, pos) >= 20;
         });
         Utils::tileAt(state.map, chest1Pos) = Tile::CHEST;
         state.mapModel->setProperties(chest1Pos.x, chest1Pos.y, true, false);
         hero.dest1 = chest1Pos.offset(0, 1);

         Position chest2Pos = Utils::randomMapPosWithCondition([&hero](const auto& pos){
            return Utils::isEmptyPatch(state.map, pos) &&
                   Utils::dist(hero.pos, pos) >= 20 &&
                   Utils::dist(hero.dest1, pos) >= 20 &&
                   Utils::dist(hero.stairs, pos) >= 20;
         });
         Utils::tileAt(state.map, chest2Pos) = Tile::CHEST;
         state.mapModel->setProperties(chest2Pos.x, chest2Pos.y, true, false);
         hero.dest2 = chest2Pos.offset(0, 1);
      }

      // PLACE TRAPS
      for (int i = 0; i < 10; i++) {
         Position trapPos = Utils::randomMapPosWithCondition(
            [](const auto& pos){return Utils::tileAt(state.map, pos) == Tile::BLANK;}
         );
         Utils::tileAt(state.map, trapPos) = Tile::TRAP;
      }

      if (state.level < 10) {
         generateMonsters(state.level, 3);
      } else {
         generateMonsters(3, 1);
         generateEndBoss();
      }

      // Draw the map
      state.mapModel->computeFov(hero.pos.x, hero.pos.y);
      draw.screen();

      // Game loop
      bool turnTaken = false;
      auto& console = *(TCODConsole::root);
      while (!TCODConsole::isWindowClosed() && !nextlevel) {
         turnTaken = false;
         Position dest = state.player;

         TCOD_key_t key = Utils::getKeyPress();

         if (key.vk == TCODK_UP || key.vk == TCODK_KP8 || key.c == 'k') {
            dest = state.player.offset(0, -1);
            turnTaken = true;
         } else if (key.vk == TCODK_DOWN || key.vk == TCODK_KP2 || key.c == 'j') {
            dest = state.player.offset(0, +1);
            turnTaken = true;
         } else if (key.vk == TCODK_LEFT || key.vk == TCODK_KP4 || key.c == 'h') {
            dest = state.player.offset(-1, 0);
            turnTaken = true;
         } else if (key.vk == TCODK_RIGHT || key.vk == TCODK_KP6 || key.c == 'l') {
            dest = state.player.offset(+1, 0);
            turnTaken = true;
         } else if (key.vk == TCODK_KP7 || key.c == 'y') {
            dest = state.player.offset(-1, -1);
            turnTaken = true;
         } else if (key.vk == TCODK_KP9 || key.c == 'u') {
            dest = state.player.offset(+1, -1);
            turnTaken = true;
         } else if (key.vk == TCODK_KP1 || key.c == 'b') {
            dest = state.player.offset(-1, +1);
            turnTaken = true;
         } else if (key.vk == TCODK_KP3 || key.c == 'n') {
            dest = state.player.offset(+1, +1);
            turnTaken = true;
         } else if (key.vk == TCODK_SPACE || key.vk == TCODK_KP5) {
            turnTaken = true;
         } else if (key.vk == TCODK_ESCAPE) {
            exit(0);
         } else if (key.c == '\'') {
            int direction = Utils::getDirection();
            if (direction != 0) {
               Position target = state.player.directionOffset(direction);
               Utils::tileAt(state.map, target) = Tile::WALL;
               state.mapModel->setProperties(target.x, target.y, false, false);
               hero.computePath();
            }
         } else if (key.vk == TCODK_F8) {
            heroSpec = 0;
            monsterSpec = 0;
            worldSpec = 0;
            state.addMessage("", MessageType::NORMAL);
            state.addMessage("", MessageType::NORMAL);
            state.addMessage("You have started a new game", MessageType::IMPORTANT);
            goto gameLoop;
         } else if (key.vk == TCODK_F5) {
            fullscreen = !fullscreen;
            console.setFullscreen(fullscreen);
         } else if (key.vk == TCODK_TAB) {
            turnTaken = castSpell('j');
         } else if (key.c == 'q' || key.c == 'w' || key.c == 'e' || key.c == 'a' || key.c == 's' || key.c == 'd' || key.c == 'z' || key.c == 'x' || key.c == 'c') {
            turnTaken = castSpell(key.c);
         } else if (key.c == 'm') {
            draw.messageHistory();
         } else if (key.c == 'v') {
            if (Utils::randGen->getInt(1, 2) == 1) {
               state.addMessage("You: HEY!", MessageType::SPELL);
            } else {
               state.addMessage("You: LISTEN!", MessageType::SPELL);
            }
         }
         if (dest.withinMap()) {
            Tile& destTile = Utils::tileAt(state.map, dest);
            if (destTile == Tile::BLANK) {
               Utils::tileAt(state.map, state.player) = Tile::BLANK;
               destTile = Tile::PLAYER;
               state.player = dest;
            } else if (destTile == Tile::MONSTER) {
               Monster* curMonster = state.findMonster(dest);
               state.addMessage("You are blocked by the " + curMonster->name, MessageType::NORMAL);
            } else if (destTile == Tile::HERO) {
               if (hero.dead) {
                  state.addMessage("You are blocked by the hero's corpse", MessageType::NORMAL);
               } else {
                  state.addMessage("You are blocked by the hero", MessageType::NORMAL);
               }
            } else if (destTile == Tile::TRAP) {
               state.addMessage("You shy away from the trap", MessageType::NORMAL);
            } else if (destTile == Tile::FIELD) {
               state.addMessage("You are blocked by the forcefield", MessageType::NORMAL);
            } else if (destTile == Tile::ILLUSION) {
               destTile = Tile::BLANK;
               state.illusion.x = -1; state.illusion.y = -1;
               state.addMessage("You disrupt the illusion", MessageType::SPELL);
            } else if (destTile == Tile::PORTAL) {
               state.addMessage("You are blocked by a swirling portal", MessageType::NORMAL);
            } else if (destTile == Tile::CHEST || destTile == Tile::CHEST_OPEN) {
               state.addMessage("You are blocked by the chest", MessageType::NORMAL);
            } else if (destTile == Tile::STAIRS || destTile == Tile::STAIRS_UP) {
               state.addMessage("You are blocked by the stairs", MessageType::NORMAL);
            }
         }
         // Assuming the hero pressed a key that made a turn
         if (turnTaken) {
            // Regenerate mana if the player is close to the hero
            if (hero.inSpellRadius()) {
               // Mana regeneration is faster if the hero is meditating
               if (hero.meditationTimer > 0) {
                  state.heroMana += 2;
                  state.monsterMana += 2;
                  state.worldMana += 2;
               } else {
                  state.heroMana++;
                  state.monsterMana++;
                  state.worldMana++;
               }
               // Ensures the mana generation doesn't go over te limit
               if (state.heroMana > 5*MANA_BLIP_SIZE) state.heroMana = 5*MANA_BLIP_SIZE;
               if (state.monsterMana > 5*MANA_BLIP_SIZE) state.monsterMana = 5*MANA_BLIP_SIZE;
               if (state.worldMana > 5*MANA_BLIP_SIZE) state.worldMana = 5*MANA_BLIP_SIZE;
            }
            // If the hero isn't dead, make him move
            if (hero.dead == false) {
               nextlevel = hero.move();
               // If the hero finished the level, go to the next level
               if (state.level == 10 && state.bossDead) {
                  nextlevel = true;
               }
            }
            // Move monsters
            for (Monster& monster : state.monsterList) {
               monsterMove(monster);
            }

            // Lowers the amount of clouds and forcefields
            for (int i = 0; i < MAP_WIDTH; i++) {
               for (int j = 0; j < MAP_HEIGHT; j++) {
                  if (state.cloud[i][j] > 0) {
                     state.cloud[i][j]--;
                     if (state.cloud[i][j] == 0) {
                        state.mapModel->setProperties(i, j, true, true);
                     }
                  }
                  if (field[i][j] > 0) {
                     field[i][j]--;
                     if (field[i][j] == 0) {
                        state.mapModel->setProperties(i, j, true, true);
                        Utils::tileAt(state.map, {i, j}) = Tile::BLANK;
                     }
                  }
               }
            }
         }
         if (nextlevel && state.level < 10) {
            // Display the next state.level
            state.addMessage("Hero: " + Hero::heroExit[Utils::randGen->getInt(0, 4)], MessageType::HERO);
            state.addMessage("The hero descends to the next state.level of the dungeon!", MessageType::IMPORTANT);
         } else {
            state.mapModel->computeFov(hero.pos.x, hero.pos.y);
         }
      }
   }
   draw.screen();
   if (state.bossDead) {
      draw.victoryScreen();
      for (TCOD_key_t key = Utils::getKeyPress(); key.vk != TCODK_ESCAPE; key = Utils::getKeyPress()) { }
   }
}

void monsterMove(Monster& curMonster) {
   // If the monster is still spawning, resolve it
   if (curMonster.portalTimer > 0) {
      curMonster.portalTimer--;
      // Once the monster has spawned
      if (curMonster.portalTimer == 0) {
         state.mapModel->setProperties(curMonster.pos.x, curMonster.pos.y, true, true);
         Utils::tileAt(state.map, curMonster.pos) = Tile::MONSTER;
      }
   } else {
      Hero& hero = *(state.hero);
      // Otherwise, the monster is not spawning
      if (curMonster.conditionTimers[Condition::SLEEPING] == 0) {
         if (curMonster.timer == 0) {
            if (curMonster.symbol != '*') {
               bool rangedAttack = false;
               Utils::tileAt(state.map, curMonster.pos) = Tile::BLANK;
               int diffx = 0, diffy = 0;

               if (curMonster.conditionTimers[Condition::HALTED] > 0) {
                  if (Utils::dist(hero.pos, curMonster.pos) == 1.0) {
                     diffx = Utils::signum(hero.pos.x - curMonster.pos.x);
                     diffy = Utils::signum(hero.pos.y - curMonster.pos.y);
                  }
               } else if (curMonster.conditionTimers[Condition::FLEEING] > 0 && state.mapModel->isInFov(curMonster.pos.x, curMonster.pos.y)) {
                     diffx = Utils::signum(curMonster.pos.x - hero.pos.x);
                     diffy = Utils::signum(curMonster.pos.y - hero.pos.y);
               } else if (curMonster.conditionTimers[Condition::RAGED] > 0 || curMonster.conditionTimers[Condition::ALLIED] > 0) {
                  float heroDist = Utils::dist(hero.pos, curMonster.pos);
                  float nearestMonsterDist = MAP_WIDTH + MAP_HEIGHT + 2;
                  Monster* nearestMonster;
                  state.mapModel->computeFov(curMonster.pos.x, curMonster.pos.y);
                  for (Monster& otherMon : state.monsterList) {
                     if ((&otherMon != &curMonster) && state.mapModel->isInFov(otherMon.pos.x, otherMon.pos.y)) {
                        float tempdist = Utils::dist(curMonster.pos, otherMon.pos);
                        if (tempdist < nearestMonsterDist) {
                           nearestMonsterDist = tempdist;
                           nearestMonster = &otherMon;
                        }
                     }
                  }
                  state.mapModel->computeFov(hero.pos.x, hero.pos.y);
                  if (heroDist < nearestMonsterDist && curMonster.conditionTimers[Condition::ALLIED] == 0) {
                     if (heroDist == 1.0 || curMonster.conditionTimers[Condition::BLINDED] == 0) {
                        diffx = Utils::signum(hero.pos.x - curMonster.pos.x);
                        diffy = Utils::signum(hero.pos.y - curMonster.pos.y);
                     } else {
                        diffx = Utils::randGen->getInt(-1, 1);
                        diffy = Utils::randGen->getInt(-1, 1);
                     }
                  } else {
                     if (nearestMonsterDist == 1.0 || curMonster.conditionTimers[Condition::BLINDED] == 0) {
                        diffx = Utils::signum(nearestMonster->pos.x - curMonster.pos.x);
                        diffy = Utils::signum(nearestMonster->pos.y - curMonster.pos.y);
                     } else {
                        diffx = Utils::randGen->getInt(-1, 1);
                        diffy = Utils::randGen->getInt(-1, 1);
                     }
                  }
               } else if (curMonster.angry && !hero.dead && (curMonster.conditionTimers[Condition::BLINDED] == 0 || hero.isAdjacent(curMonster.pos))) {
                  if (curMonster.ranged && !hero.isAdjacent(curMonster.pos) && Utils::dist(curMonster.pos, hero.pos) <= curMonster.range && state.mapModel->isInFov(curMonster.pos.x, curMonster.pos.y)) {
                     rangedAttack = true;
                  }
                  diffx = Utils::signum(hero.pos.x - curMonster.pos.x);
                  diffy = Utils::signum(hero.pos.y - curMonster.pos.y);
               } else {
                  if (state.mapModel->isInFov(curMonster.pos.x, curMonster.pos.y) && (curMonster.conditionTimers[Condition::BLINDED] == 0 || hero.isAdjacent(curMonster.pos))) {
                     if (curMonster.symbol != '@' || curMonster.health != curMonster.maxhealth) {
                        curMonster.angry = true;
                     }
                  }
                  diffx = Utils::randGen->getInt(-1, 1);
                  diffy = Utils::randGen->getInt(-1, 1);
               }
               if (diffx != 0 && diffy != 0) {
                  if (Utils::randGen->getInt(0, 1) == 0) {
                     diffx = 0;
                  } else {
                     diffy = 0;
                  }
               }
               if (rangedAttack) {
                  displayRangedAttack(hero.pos, curMonster.pos);
                  int damage = curMonster.damage - static_cast<int>(ceil((double)curMonster.conditionTimers[Condition::WEAKENED]/CONDITION_TIMES.at(Condition::WEAKENED)));
                  if (hero.shieldTimer == 0) {
                     if (damage < 0) damage = 0;
                     hero.health -= damage;
                     char buffer[20];
                     sprintf(buffer, "%d", damage);
                     state.addMessage("The " + curMonster.name + " " + curMonster.rangedName + " at the hero for " + buffer + " damage", MessageType::NORMAL);
                     if (hero.health <= 0) {
                        hero.dead = true;
                        state.addMessage("The hero has died!", MessageType::IMPORTANT);
                     } else if (hero.meditationTimer > 0) {
                        hero.meditationTimer = 0;
                        state.addMessage("The hero's meditation is interrupted!", MessageType::SPELL);
                     }
                  } else {
                     hero.shieldTimer -= curMonster.damage;
                     if (hero.shieldTimer <= 0) {
                        hero.shieldTimer = 0;
                        state.addMessage("The shield is shattered by the " + curMonster.name + "'s attack!", MessageType::SPELL);
                     } else {
                        state.addMessage("The " + curMonster.name + "'s attack is deflected by the shield", MessageType::SPELL);
                     }
                  }
                  if (curMonster.maimed) {
                     state.addMessage("The "+curMonster.name+" suffers from the exertion!", MessageType::NORMAL);
                     state.hitMonster(curMonster.pos, damage);
                  }
               } else {
                  Position diffPos = curMonster.pos.offset(diffx, diffy);
                  Tile& diffTile = Utils::tileAt(state.map, diffPos);
                  if (diffPos == hero.pos && curMonster.conditionTimers[Condition::ALLIED] == 0) {
                     if (hero.dead) {
                        hero.health -= curMonster.damage;
                        if (curMonster.symbol != '@') {
                           state.addMessage("The " + curMonster.name + " savages the hero's corpse", MessageType::NORMAL);
                        }
                     } else {
                        int damage = curMonster.damage - static_cast<int>(ceil((double)curMonster.conditionTimers[Condition::WEAKENED]/CONDITION_TIMES.at(Condition::WEAKENED)));
                        if (damage < 0) damage = 0;
                        hero.health -= damage;
                        char buffer[20];
                        sprintf(buffer, "%d", damage);
                        state.addMessage("The " + curMonster.name + " hits the hero for " + buffer + " damage", MessageType::NORMAL);
                        if (hero.health <= 0) {
                           hero.dead = true;
                           state.addMessage("The hero has died!", MessageType::IMPORTANT);
                        } else if (hero.meditationTimer > 0) {
                           hero.meditationTimer = 0;
                           state.addMessage("The hero's meditation is interrupted!", MessageType::SPELL);
                        } else if (&curMonster != hero.target && hero.target != nullptr) {
                           if (Utils::dist(hero.pos, curMonster.pos) < Utils::dist(hero.pos, hero.target->pos)) {
                              hero.target = &curMonster;
                           }
                        }
                        if (curMonster.maimed) {
                           state.addMessage("The "+curMonster.name+" suffers from the exertion!", MessageType::NORMAL);
                           state.hitMonster(curMonster.pos, damage);
                        }
                     }
                  } else if (diffTile == Tile::MONSTER && (diffx != 0 || diffy != 0)) {
                     Monster* otherMonster = state.findMonster(diffPos);
                     if (curMonster.conditionTimers[Condition::RAGED] > 0 || curMonster.conditionTimers[Condition::ALLIED] > 0) {
                        char buffer[20];
                        sprintf(buffer, "%d", curMonster.damage);
                        state.addMessage("The " + curMonster.name + " hits the " + otherMonster->name + " for " +buffer + " damage", MessageType::NORMAL);
                        state.hitMonster(diffPos, curMonster.damage);
                     } else {
                        state.addMessage("The " + curMonster.name + " bumps into the " + otherMonster->name, MessageType::NORMAL);
                     }
                  } else if (diffTile == Tile::PLAYER) {
                     std::swap(state.player, curMonster.pos);
                     state.addMessage("The " + curMonster.name + " passes through you", MessageType::NORMAL);
                     Utils::tileAt(state.map, state.player) = Tile::PLAYER;
                  } else if (diffTile == Tile::TRAP) {
                     state.addMessage("The " + curMonster.name+ " falls into the trap!", MessageType::NORMAL);
                     diffTile = Tile::BLANK;
                     curMonster.pos = diffPos;
                     state.hitMonster(curMonster.pos, 4);
                  } else if (diffTile == Tile::ILLUSION) {
                     diffTile = Tile::BLANK;
                     state.illusion = {1, -1};
                     state.addMessage("The "+curMonster.name+" disrupts the illusion", MessageType::SPELL);
                  } else if (diffTile == Tile::BLANK) {
                     curMonster.pos = diffPos;
                  }
               }
               Utils::tileAt(state.map, curMonster.pos) = Tile::MONSTER;
            } else {
               if (state.monsterList.size() < MAX_MONSTERS) {
                  Position l = Utils::randomMapPosWithCondition(
                     [](const auto& pos){return Utils::tileAt(state.map, pos) == Tile::BLANK;}
                  );
                  state.addSpecifiedMonster(l.x, l.y, Utils::randGen->getInt(0, 12), true);
               }
            }
            curMonster.timer = curMonster.wait;

            for (auto& [condition, timer]: curMonster.conditionTimers) {
               if (timer > 0) {
                  timer--;
                  if (timer == 0) {
                     const auto& text = CONDITION_END.at(condition);
                     state.addMessage(text.first + curMonster.name + text.second, MessageType::SPELL);
                  }
               }
            }

         }
         curMonster.timer -= 1;
      } else {
         curMonster.conditionTimers[Condition::SLEEPING]--;
         if (curMonster.conditionTimers[Condition::SLEEPING] == 0) {
            auto& text = CONDITION_END.at(Condition::SLEEPING);
            state.addMessage(text.first + curMonster.name + text.second, MessageType::SPELL);
         }
      }
   }
}

void drawBSP(TCODBsp* curBSP) {
   if (curBSP != nullptr) {
      if (curBSP->isLeaf()) {
         int x1 = curBSP->x;
         int x2 = curBSP->x+curBSP->w-1;
         int y1 = curBSP->y;
         int y2 = curBSP->y+curBSP->h-1;
         for (int i = x1+2; i <= x2-2; i++) {
            for (int j = y1+Utils::randGen->getInt(1, 2); j <= y2-Utils::randGen->getInt(1, 2); j++) {
               Utils::tileAt(state.map, {i, j}) = Tile::BLANK;
               state.mapModel->setProperties(i, j, true, true);
            }
         }
         for (int j = y1+2; j <= y2-2; j++) {
            if (Utils::randGen->getInt(1, 2) == 1) {
               Utils::tileAt(state.map, {x1+1, j}) = Tile::BLANK;
               state.mapModel->setProperties(x1+1, j, true, true);
            }
            if (Utils::randGen->getInt(1, 2) == 1) {
               Utils::tileAt(state.map, {x2-1, j}) = Tile::BLANK;
               state.mapModel->setProperties(x2-1, j, true, true);
            }
         }
      } else {
         drawBSP(curBSP->getLeft());
         drawBSP(curBSP->getRight());
         if (!curBSP->horizontal) {
            int x1 = curBSP->getLeft()->x + curBSP->getLeft()->w/2;
            int x2 = curBSP->getRight()->x + curBSP->getRight()->w/2;
            int y = curBSP->y + curBSP->h/2;
            if (x1 > x2) {
               int temp = x1;
               x1 = x2;
               x2 = temp;
            }
            for (int i = x1; i <= x2; i++) {
               Utils::tileAt(state.map, {i, y}) = Tile::BLANK;
               state.mapModel->setProperties(i, y, true, true);
            }
         } else {
            int y1 = curBSP->getLeft()->y + curBSP->getLeft()->h/2;
            int y2 = curBSP->getRight()->y + curBSP->getRight()->h/2;
            int x = curBSP->x + curBSP->w/2;
            if (y1 > y2) {
               int temp = y1;
               y1 = y2;
               y2 = temp;
            }
            for (int j = y1; j <= y2; j++) {
               Utils::tileAt(state.map, {x, j}) = Tile::BLANK;
               state.mapModel->setProperties(x, j, true, true);
            }
         }
      }
   }
}

void displayMessageHistory() {
   draw.messageHistory();

   // Wait for keyboard response
   TCOD_key_t key = Utils::getKeyPress();
}

void displayUpgradeMenu() {
   state.addMessage("You are now experienced enough to specialise your magic!", MessageType::IMPORTANT);

   draw.upgradeMenu(heroSpec, monsterSpec, worldSpec);

   // Get keyboard input
   TCOD_key_t key;
   char spell = '\0';
   while (spell == '\0') {
      key = Utils::getKeyPress();
      if (key.vk == TCODK_BACKSPACE) {
         spell = ' ';
      } else if (((key.c == 'q' || key.c == 'a' || key.c == 'z') && heroSpec == 0) || ((key.c == 'w' || key.c == 's' || key.c == 'x') && monsterSpec == 0) || ((key.c == 'e' || key.c == 'd' || key.c == 'c') && worldSpec == 0)) {
         spell = key.c;
      }
   }
   switch (spell) {
      case 'q':
         state.addMessage("You specialise towards Pacifism! Check your spell menu!", MessageType::IMPORTANT);
         heroSpec = 1;
         break;
      case 'a':
         state.addMessage("You specialise towards Speed! Check your spell menu!", MessageType::IMPORTANT);
         heroSpec = 2;
         break;
      case 'z':
         state.addMessage("You specialise towards Heal! Check your spell menu!", MessageType::IMPORTANT);
         heroSpec = 3;
         break;
      case 'w':
         state.addMessage("You specialise towards Blind! Check your spell menu!", MessageType::IMPORTANT);
         monsterSpec = 1;
         break;
      case 's':
         state.addMessage("You specialise towards Rage! Check your spell menu!", MessageType::IMPORTANT);
         monsterSpec = 2;
         break;
      case 'x':
         state.addMessage("You specialise towards Sleep! Check your spell menu!", MessageType::IMPORTANT);
         monsterSpec = 3;
         break;
      case 'e':
         state.addMessage("You specialise towards Clear! Check your spell menu!", MessageType::IMPORTANT);
         worldSpec = 1;
         break;
      case 'd':
         state.addMessage("You specialise towards Cloud! Check your spell menu!", MessageType::IMPORTANT);
         worldSpec = 2;
         break;
      case 'c':
         state.addMessage("You specialise towards Trap! Check your spell menu!", MessageType::IMPORTANT);
         worldSpec = 3;
         break;
      default:
         state.addMessage("You choose not to specialise your spells", MessageType::IMPORTANT);
         break;
   }
}

bool castSpell(char spellChar) {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   if (spellChar == 'j') {
      draw.spellMenu(heroSpec, monsterSpec, worldSpec);

      // Get keyboard input
      TCOD_key_t key = Utils::getKeyPress();
      spellChar = key.c;
   }

   // DETERMINE THE SPELL TO CAST BASED ON THE KEY AND SPELL SPECIALISATION
   bool spellCast = false;
   switch (spellChar) {
      case 'q':
         if (state.heroMana >= MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[0][heroSpec][0]);
            if (spellCast) state.heroMana -= MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'a':
         if (state.heroMana >= 3*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[0][heroSpec][1]);
            if (spellCast) state.heroMana -= 3*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'z':
         if (state.heroMana >= 5*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[0][heroSpec][2]);
            if (spellCast) state.heroMana -= 5*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'w':
         if (state.monsterMana >= MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[1][monsterSpec][0]);
            if (spellCast) state.monsterMana -= MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 's':
         if (state.monsterMana >= 3*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[1][monsterSpec][1]);
            if (spellCast) state.monsterMana -= 3*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'x':
         if (state.monsterMana >= 5*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[1][monsterSpec][2]);
            if (spellCast) state.monsterMana -= 5*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'e':
         if (state.worldMana >= MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[2][worldSpec][0]);
            if (spellCast) state.worldMana -= MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'd':
         if (state.worldMana >= 3*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[2][worldSpec][1]);
            if (spellCast) state.worldMana -= 3*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'c':
         if (state.worldMana >= 5*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[2][worldSpec][2]);
            if (spellCast) state.worldMana -= 5*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      default:
         break;
   }
   // CAST THE CHOSEN SPELL
   return spellCast;
}

bool effectSpell(Spell chosenSpell) {
   Hero& hero = *(state.hero);

   int spellCast = false;
   int direction;
   bool itemDropped = false;
   bool minePlaced = false;
   
   draw.screen();
   switch (chosenSpell) {
      case Spell::PACIFISM:
         if (state.level < 10) {
            if (hero.inSpellRadius()) {
               if (hero.dead) {
                  state.addMessage("The hero is dead!", MessageType::SPELL);
               } else {
                  hero.pacifismTimer = hero.items.contains(Item::magicResist)
                     ? (PACIFISM_TIME/2)
                     : PACIFISM_TIME;
                  hero.target = nullptr;
                  state.addMessage("The hero appears calmer!", MessageType::SPELL);
                  hero.computePath();
               }
               spellCast = true;
            } else {
               state.addMessage("You are too far from the hero!", MessageType::SPELL);
            }
         } else {
            state.addMessage("The hero is too enCondition::RAGED to be pacified!", MessageType::SPELL);
         }
         break;
      case Spell::SPEED:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.hasteTimer = hero.items.contains(Item::magicResist)
                  ? (SPEED_TIME/2)
                  : SPEED_TIME;
               state.addMessage("The hero becomes a blur!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::HEAL:
         if (hero.inSpellRadius()) {
            if (hero.dead){
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               bool gained = false;
               gained = hero.gainHealth(hero.items.contains(Item::magicResist) ? 1 : 3);
               if (gained) {
                  state.addMessage("The hero looks healthier!", MessageType::SPELL);
               } else {
                  state.addMessage("The spell has no effect!", MessageType::SPELL);
               }
               spellCast = true;
            }
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::BLIND:
         spellCast = applyMonsterCondition(Condition::BLINDED, false);
         break;
      case Spell::RAGE:
         spellCast = applyMonsterCondition(Condition::RAGED, false);
         break;
      case Spell::SLEEP:
         spellCast = applyMonsterCondition(Condition::SLEEPING, false);
         break;
      case Spell::CLEAR:
         for (int i = state.player.x-1; i <= state.player.x+1; i++) {
            for (int j = state.player.y-1; j <= state.player.y+1; j++) {
               if (i>=0 && j>=0 && i<MAP_WIDTH && j <MAP_HEIGHT) {
                  if (Tile& curTile = Utils::tileAt(state.map, {i, j}); curTile == Tile::WALL || curTile == Tile::TRAP) {
                     curTile = Tile::BLANK;
                     state.mapModel->setProperties(i, j, true, true);
                     if (hero.target == nullptr) {
                        hero.computePath();
                     }
                  }
               }
            }
         }
         state.addMessage("The area around you clears!", MessageType::SPELL);
         spellCast = true;
         break;
      case Spell::CLOUD:
         for (int i = state.player.x-2; i <= state.player.x+2; i++) {
            for (int j = state.player.y-2; j <= state.player.y+2; j++) {
               int cloudDist = abs(state.player.x-i)+abs(state.player.y-j);
               if ((cloudDist < 4) && i>=0 && i<MAP_WIDTH && j>=0 && j <MAP_HEIGHT) {
                  if (Utils::tileAt(state.map, {i, j}) != Tile::WALL) {
                     int newCloudLevel = CLOUD_TIME*(8-cloudDist)/8;
                     if (newCloudLevel > state.cloud[i][j]) {
                        state.cloud[i][j] = newCloudLevel;
                     }
                     state.mapModel->setProperties(i, j, false, true);
                  }
               }
            }
         }
         state.addMessage("A thick cloud of smoke appears around you!", MessageType::SPELL);
         spellCast = true;

         break;
      case Spell::MTRAP:
         direction = Utils::getDirection();
         if (direction != 0) {
            Position target = state.player.directionOffset(direction);
            if (target.withinMap() && Utils::tileAt(state.map,target) == Tile::BLANK) {
               state.addMessage("You create a trap in the ground", MessageType::SPELL);
               Utils::tileAt(state.map,target) = Tile::TRAP;
            } else {
               state.addMessage("Without empty ground, the spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::MEDITATION:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.meditationTimer = hero.items.contains(Item::magicResist)
                  ? (MEDITATION_TIME / 2)
                  : MEDITATION_TIME;
               state.addMessage("The hero begins quiet introspection", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::CHARITY:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               // Check for each item the hero might have
               for (int i = 0; i < ITEM_COUNT; i++) {
                  const Item curItem = static_cast<Item>(i);
                  if (hero.items.contains(curItem)) {
                     // Take off the item
                     hero.items.erase(curItem);
                     state.addMessage("The hero takes off "+ITEM_NAME.at(curItem), MessageType::SPELL);
                     itemDropped = true;
                     // Specific effects of taking off each item
                     switch(curItem) {
                        case Item::monsterHelm:
                           for (auto& monster : state.monsterList) {
                              monster.angry = false;
                           }
                           break;
                        case Item::rustedSword:
                           hero.damage = 5;
                        default:
                           break;
                     };
                  }
               }
               // If an item was dropped, the hero is healed
               if (itemDropped) {
                  state.addMessage("Hero: " + Hero::heroCharity[Utils::randGen->getInt(0, 4)], MessageType::HERO);
                  hero.gainHealth(10);
                  spellCast = true;
               } else {
                  state.addMessage("The hero is not wearing any treasure!", MessageType::SPELL);
               }
            }
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::SLOW:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               if (hero.slow) {
                  hero.slow = false;
                  state.addMessage("The hero's actions speed up!", MessageType::SPELL);
               } else {
                  hero.slow = true;
                  state.addMessage("The hero's actions slow down!", MessageType::SPELL);
               }
            }
            spellCast = true;
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::SHIELD:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.shieldTimer = hero.items.contains(Item::magicResist)
                  ? (SHIELD_TIME / 2)
                  : SHIELD_TIME;
               state.addMessage("A transclucent shield surrounds the hero!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::REGENERATE:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.regenTimer = hero.items.contains(Item::magicResist)
                  ? (REGEN_TIME / 2)
                  : REGEN_TIME;
               state.addMessage("You project some healing magic onto the hero!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::BLINK:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               state.addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.blinking = true;
               const auto limit = hero.items.contains(Item::magicResist)
                  ? (BLINK_MOVES / 2)
                  : BLINK_MOVES;
               for (int i = 0; i < limit; ++i) {
                  hero.move();
                  draw.screen();
                  TCODSystem::sleepMilli(10);
               }
               hero.blinking = false;
               state.addMessage("The hero's actions are almost instant!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            state.addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::TUNNEL:
         direction = Utils::getDirection();
         if (direction != 0) {
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            for (int i = 1; i <= 3; i++) {
               Position stepPos = state.player.offset(diffX*i, diffY*i);
               if (stepPos.withinMap()) {
                  if (Tile& stepTile = Utils::tileAt(state.map, stepPos); stepTile == Tile::WALL || stepTile == Tile::TRAP) {
                     stepTile = Tile::BLANK;
                     state.mapModel->setProperties(stepPos.x, stepPos.y, true, true);
                  }
               }
            }
            state.addMessage("A path is cleared for you!", MessageType::SPELL);
            if (hero.target == nullptr) {
               hero.computePath();
            }
            spellCast = true;
         }
         break;
      case Spell::MINEFIELD:
         for (int i = 0; i < 5; i++) {
            Position temp = state.player.offset(
               Utils::randGen->getInt(-2, 2),
               Utils::randGen->getInt(-2, 2)
            );
            if (temp.withinMap()) {
               if (Tile& tempTile = Utils::tileAt(state.map, temp); tempTile == Tile::BLANK) {
                  tempTile = Tile::TRAP;
                  minePlaced = true;
               }
            }
         }
         if (minePlaced) {
            state.addMessage("Traps materialise in the surrounding area!", MessageType::SPELL);
         } else {
            state.addMessage("You cast the spell, but no traps materialise!", MessageType::SPELL);
         }
         spellCast = true;
         break;
      case Spell::MAIM:
         direction = Utils::getDirection();
         if (direction != 0) {
            Position target = state.player.directionOffset(direction);
            if (Utils::tileAt(state.map, target) == Tile::MONSTER) {
               Monster* targetMonster = state.findMonster(target);
               state.addMessage("The " + targetMonster->name + " looks pained!", MessageType::SPELL);
               targetMonster->maimed = true;
            } else {
               state.addMessage("The spell fizzles in empty air", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::CRIPPLE:
         direction = Utils::getDirection();
         if (direction != 0) {
            Position target = state.player.directionOffset(direction);
            if (Utils::tileAt(state.map, target)  == Tile::MONSTER) {
               Monster* targetMonster = state.findMonster(target);
               state.addMessage("A terrible snap comes from inside the " + targetMonster->name + "!", MessageType::SPELL);
               targetMonster->health = (targetMonster->health+1)/2;
            } else {
               state.addMessage("The spell fizzles in empty air", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::MILLUSION:
         direction = Utils::getDirection();
         if (direction != 0) {
            Position target = state.player.directionOffset(direction);
            if (target.withinMap() && Utils::tileAt(state.map, target) == Tile::BLANK ) {
               if (state.illusion.x != -1) {
                  Utils::tileAt(state.map, state.illusion)  = Tile::BLANK;
               }
               state.illusion = target;
               Utils::tileAt(state.map, target) = Tile::ILLUSION;
            } else {
               state.addMessage("Without empty ground, the spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::WEAKEN:
         spellCast = applyMonsterCondition(Condition::WEAKENED, true);
         break;
      case Spell::ALLY:
         spellCast = applyMonsterCondition(Condition::ALLIED, false);
         break;
      case Spell::HALT:
         spellCast = applyMonsterCondition(Condition::HALTED, false);
         break;
      case Spell::FLEE:
         spellCast = applyMonsterCondition(Condition::FLEEING, false);
         break;
      case Spell::SCREEN:
         direction = Utils::getDirection();
         if (direction != 0) {
            bool screenMade = false;
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            for (int i = -1; i < 2; i++) {
               for (int j = -1; j < 2; j++) {
                  int diff = abs(diffX-i)+abs(diffY-j);
                  if ((i != 0 || j != 0) && diff < 2) {
                     if (Position curr = state.player.offset(i, j); curr.withinMap()) {
                        Tile& currTile = Utils::tileAt(state.map, curr);
                        if (currTile == Tile::TRAP || currTile == Tile::WALL) {
                           currTile = Tile::BLANK;
                        }
                        state.cloud[curr.x][curr.y] = (diff == 0 ? CLOUD_TIME : (CLOUD_TIME*7/8));
                        state.mapModel->setProperties(curr.x, curr.y, false, true);
                        screenMade = true;
                     }
                  }
               }
            }
            if (screenMade) {
               state.addMessage("A wall of cloud appears before you", MessageType::SPELL);
               hero.computePath();
            } else {
               state.addMessage("The spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::MFIELD:
         direction = Utils::getDirection();
         if (direction != 0) {
            bool fieldMade = false;
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            Position curr{0, 0};
            if (diffX == 0 || diffY == 0) {
               for (int i = -2; i <= 2; i++) {
                  curr = state.player.offset(diffX+(i*(1-abs(diffX))), diffY+(i*(1-abs(diffY))));
                  if (Tile& currTile = Utils::tileAt(state.map, curr); currTile == Tile::BLANK) {
                     field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     currTile = Tile::FIELD;
                     state.mapModel->setProperties(curr.x, curr.y, false, false);
                     fieldMade = true;
                  }
               }
               for (int i = -1; i <= 1; i++) {
                  curr = state.player.offset(diffX*2+(i*(1-abs(diffX))), diffY*2+(i*(1-abs(diffY))));
                  if (Tile& currTile = Utils::tileAt(state.map, curr); currTile == Tile::BLANK) {
                     field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     currTile = Tile::FIELD;
                     state.mapModel->setProperties(curr.x, curr.y, false, false);
                     fieldMade = true;
                  }
               }
            } else {
               for (int i = -1; i <= 1; i++) {
                  curr = state.player.offset(diffX-(i*diffX), diffY+(i*diffY));
                  if (Tile& currTile = Utils::tileAt(state.map, curr); currTile == Tile::BLANK) {
                     field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     currTile = Tile::FIELD;
                     state.mapModel->setProperties(curr.x, curr.y, false, false);
                     fieldMade = true;
                  }
               }
               for (int i = 0; i <= 3; i++) {
                  curr = state.player.offset(
                     static_cast<int>(diffX/2.0f-((i-1.5)*diffX)),
                     static_cast<int>(diffY/2.0f+((i-1.5)*diffY))
                  );
                  if (Tile& currTile = Utils::tileAt(state.map, curr); currTile == Tile::BLANK) {
                     field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     currTile = Tile::FIELD;
                     state.mapModel->setProperties(curr.x, curr.y, false, false);
                     fieldMade = true;
                  }
               }
            }
            if (fieldMade) {
               hero.computePath();
               state.addMessage("An impassable field appears before you!", MessageType::SPELL);
            } else {
               state.addMessage("The spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::BLOW:
         spellCast = true;
         state.addMessage("Magical force bursts out from you!", MessageType::SPELL);
         for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
               if (i != 0 || j != 0) {
                  Position step1 = state.player.offset(i, j);
                  Tile& step1Tile = Utils::tileAt(state.map, step1);
                  Position step2 = state.player.offset(2*i, 2*j);
                  Tile& step2Tile = Utils::tileAt(state.map, step2);
                  Position step3 = state.player.offset(3*i, 3*j);
                  Tile& step3Tile = Utils::tileAt(state.map, step3);
                  Position temp{step1.x, step1.y};
                  bool trapSprung = false;
                  if (step1.withinMap() && step2.withinMap()) {
                     if (step1Tile == Tile::MONSTER) {
                        if (step2Tile == Tile::BLANK) {
                           if (step3.withinMap() && (step3Tile == Tile::BLANK || step3Tile == Tile::TRAP)) {
                              temp = step3;
                              if (step3Tile == Tile::TRAP) {
                                 trapSprung = true;
                              }
                           } else {
                              temp = step2;
                           }
                        } else if (step2Tile == Tile::TRAP) {
                           trapSprung = true;
                           temp = step2;
                        }
                        Monster* target = state.findMonster(step1);
                        target->timer = target->wait;
                        step1Tile = Tile::BLANK;
                        Utils::tileAt(state.map, temp) = Tile::MONSTER;
                        target->pos = temp;
                        if (trapSprung) {
                           state.addMessage("The " + target->name+ " is blown into the trap!", MessageType::SPELL);
                           state.hitMonster(target->pos, 4);
                        }
                     } else if (step1Tile == Tile::HERO) {
                        if (step2Tile == Tile::BLANK) {
                           if (step3.withinMap() && (step3Tile == Tile::BLANK || step3Tile == Tile::TRAP)) {
                              temp = step3;
                              if (step3Tile == Tile::TRAP) {
                                 trapSprung = true;
                              }
                           } else {
                              temp = step2;
                           }
                        } else if (step2Tile == Tile::TRAP) {
                           trapSprung = true;
                           temp = step2;
                        }
                        step1Tile = Tile::BLANK;
                        Utils::tileAt(state.map, temp) = Tile::HERO;
                        hero.pos = temp;
                        hero.timer = hero.wait;
                        if (trapSprung) {
                           if (!hero.dead) {
                              state.addMessage("The hero is blown into the trap!", MessageType::SPELL);
                              hero.health -= 4;
                              step1Tile = Tile::BLANK;
                              Utils::tileAt(state.map, temp) = Tile::HERO;
                              hero.pos.x = temp.x; hero.pos.y = temp.y;
                              if (hero.health <= 0) {
                                 hero.dead = true;
                                 state.addMessage("The hero has died!", MessageType::IMPORTANT);
                              } else {
                                 state.addMessage("Hero: " + Hero::heroBlow[Utils::randGen->getInt(0, 4)], MessageType::HERO);
                              }
                           } else {
                              step1Tile = Tile::BLANK;
                              Utils::tileAt(state.map, temp) = Tile::HERO;
                              hero.pos = temp;
                              state.addMessage("The hero's corpse is blown into the trap!", MessageType::SPELL);
                           }
                        } else {
                           if (!hero.dead) {
                              state.addMessage("Hero: " + Hero::heroBlow[Utils::randGen->getInt(0, 4)], MessageType::HERO);
                           }
                        }
                     } else if (step1Tile == Tile::TRAP && (step2Tile == Tile::BLANK || step2Tile == Tile::HERO || step2Tile == Tile::MONSTER)) {
                        if (step2Tile == Tile::BLANK) {
                           if (step3.withinMap() && (step3Tile == Tile::BLANK || step3Tile == Tile::HERO || step3Tile == Tile::MONSTER)) {
                              temp = step3;
                           } else {
                              temp = step2;
                           }
                        } else {
                           temp = step2;
                        }
                        step1Tile = Tile::BLANK;
                        Tile& tempTile = Utils::tileAt(state.map, temp);
                        if (tempTile == Tile::BLANK) {
                           tempTile = Tile::TRAP;
                        } else if (tempTile == Tile::HERO) {
                           if (!hero.dead) {
                              state.addMessage("You blow a trap into the hero!", MessageType::SPELL);
                              hero.health -= 4;
                              if (hero.health <= 0) {
                                 hero.dead = true;
                                 state.addMessage("The hero has died!", MessageType::IMPORTANT);
                              } else {
                                 state.addMessage("Hero: " + Hero::heroBlow[Utils::randGen->getInt(0, 4)], MessageType::HERO);
                              }
                           } else {
                              state.addMessage("You blow a trap into the hero's corpse!", MessageType::SPELL);
                           }
                        } else if (tempTile == Tile::MONSTER) {
                           Monster* target = state.findMonster(temp);
                           state.addMessage("You blow a trap into the " + target->name+ "!", MessageType::SPELL);
                           state.hitMonster(target->pos, 4);
                        }
                     }
                  }
               }
            }
         }
         break;
      default:
         break;
   }

   draw.screen();
   return spellCast;
}

void generateMonsters(int level, int amount) {
   // Clear all existing monsters
   state.monsterList.clear();
   Position temp {0, 0};
   for (int a = 0; a < 4; a++) {
      for (int i = 0; i < amount; i++) {
         temp = {0, 0};
         while (Utils::tileAt(state.map, temp) != Tile::BLANK) {
            int x = (a/2 == 0)
               ? Utils::randGen->getInt(0, (MAP_WIDTH-1)/2)
               : Utils::randGen->getInt((MAP_WIDTH-1)/2, MAP_WIDTH-1);
            int y = (a%2 == 0)
               ? Utils::randGen->getInt(0, (MAP_HEIGHT-1)/2)
               : Utils::randGen->getInt((MAP_HEIGHT-1)/2, MAP_HEIGHT-1);
            temp = {x, y};
         }
         int randomMonster = Utils::randGen->getInt(level-1, level+3);
         state.addSpecifiedMonster(temp.x, temp.y, randomMonster, false);
      }
   }
}

void displayRangedAttack(int x1, int y1, int x2, int y2) {
   auto& console = *(TCODConsole::root);

   TCODLine::init(x1, y1, x2, y2);
   int xStep = x1, yStep = y1;
   do {
      console.setDefaultForeground(TCODColor::red);
      console.putChar(LEFT+xStep, TOP+yStep, '*', TCOD_BKGND_NONE);
   } while (! TCODLine::step(&xStep, &yStep) );
   console.flush();
   TCODSystem::sleepMilli(200);
}

void displayRangedAttack(const Position& p1, const Position& p2) {
   displayRangedAttack(p1.x, p1.y, p2.x, p2.y);
}

void generateEndBoss() {
   Hero& hero = *(state.hero);
   Position bossPos = Utils::randomMapPosWithCondition(
      [&hero](const auto& pos){return Utils::tileAt(state.map, pos) == Tile::BLANK && Utils::dist(pos, hero.pos) >= 30;}
   );
   int boss = Utils::randGen->getInt(0, 2);
   if (boss == 0) {
      state.addMonster("Master Summoner", '*', bossPos.x, bossPos.y, 20, 2, false, " ", 0.0f, 15, false); 
      state.addMessage("Master Summoner: You have come to your grave! I will bury you in monsters!", MessageType::VILLAIN);
   } else if (boss == 1) {
      state.addMonster("Noble Hero", '@', bossPos.x, bossPos.y, 30, 3, false, " ", 0.0f, 2, false); 
      state.addMessage("Noble Hero: I have made it to the treasure first, my boastful rival.", MessageType::VILLAIN);
      state.addMessage("Noble Hero: I wish you no harm, do not force me to defend myself.", MessageType::VILLAIN);
   } else {
      state.addMonster("Evil Mage", 'M', bossPos.x, bossPos.y, 15, 4, true, "shoots lightning", 20.0f, 5, false); 
      state.addMessage("Evil Mage: How did you make it this far?! DIE!!!", MessageType::VILLAIN);
   }
   hero.stairs = bossPos;
}

bool applyMonsterCondition(Condition curCondition, bool append) {
   int direction = Utils::getDirection();
   if (direction == 0)
      return false;

   Position target = state.player.directionOffset(direction);
   if (Utils::tileAt(state.map, target) == Tile::MONSTER) {
      Monster* targetMonster = state.findMonster(target);
      const auto& text = CONDITION_START.at(curCondition);
      state.addMessage(text.first + targetMonster->name + text.second, MessageType::SPELL);
      
      int& timer = targetMonster->conditionTimers.at(curCondition);
      const int amount = CONDITION_TIMES.at(curCondition);
      if (append) {
         timer += amount;
      } else {
         timer = amount;
      }
   } else {
      state.addMessage("The spell fizzles in empty air", MessageType::SPELL);
   }
   return true;
}