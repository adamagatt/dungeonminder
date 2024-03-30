#include <SDL2/SDL.h>
#include "DungeonMinder.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

// Main Method
int main() {

   // Initialise the spell menu
   createSpellMenu();

   // Initialise messageList
   for (int i = 0; i < MESSAGE_COUNT-2; i++) {
      messageList[i] = "";
      messageType[i] = MessageType::NORMAL;
   }
   messageList[MESSAGE_COUNT-2] = "Welcome to the game!";
   messageType[MESSAGE_COUNT-2] = MessageType::IMPORTANT;
   messageList[MESSAGE_COUNT-1] = "";
   messageType[MESSAGE_COUNT-1] = MessageType::NORMAL;

   // Creates the console
   TCODConsole::initRoot(80,60,"DungeonMinder",false);
   TCODSystem::setFps(25);

   heroSpec = 0;
   monsterSpec = 0;
   worldSpec = 0;

gameLoop:
   for (level = 1; level <= 10 && !TCODConsole::isWindowClosed(); level++) {
      if (level%3 == 0) {
         displayUpgradeMenu();
      }
      bool nextLevel = false;
      addMessage("Hero: " + Hero::heroEntry[randGen->getInt(0, 4)], MessageType::HERO);

      // Generate Map Noise
      TCODNoise wallNoiseGen = TCODNoise(2, randGen);
      TCODNoise floorNoiseGen = TCODNoise(2, randGen);
      for (int j = 0; j < MAP_HEIGHT; j++) {
         for (int i = 0; i < MAP_WIDTH; i++) {
            float location[2] = {((float)i*2)/MAP_WIDTH-1, ((float)j*2)/MAP_HEIGHT-1};
            wallNoise[i][j] = (1+wallNoiseGen.get(location)/2);
            floorNoise[i][j] = (1+floorNoiseGen.get(location)/2);
         }
      }

      // Creates the map
      for (int i = 0; i < MAP_WIDTH; i++) {
         for (int j = 0; j < MAP_HEIGHT; j++) {
            map[i][j] = WALL;
            cloud[i][j] = 0;
            field[i][j] = 0;
         }
      }
      TCODBsp mapBSP (0,0,MAP_WIDTH,MAP_HEIGHT);
      if (level < 10) {
         mapBSP.splitRecursive(nullptr, 6, 5, 5, 1.5f, 1.5f);
      } else {
         mapBSP.splitRecursive(nullptr, 2, 5, 5, 1.5f, 1.5f);
      }
      mapModel->clear();
      drawBSP(&mapBSP);
      map[0][0] = WALL;

      // Initialise the hero and player
      int x=-1, y=-1;
      while (!isEmptyPatch(x, y)) {
         x = randGen->getInt(2, MAP_WIDTH-2);
         y = randGen->getInt(2, MAP_HEIGHT-2);
      }
      map[x][y+1] = STAIRS_UP;
      mapModel->setProperties(x, y+1, true, false);

      playerX = x;
      playerY = y-1;
      map[playerX][playerY] = PLAYER;
      hero.x = x;
      hero.y = y;
      map[hero.x][hero.y] = HERO;
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
      hero.dest1x = -1;
      hero.dest1y = -1;
      hero.dest2x = -1;
      hero.dest2y = -1;
      hero.items.clear();
      y--;
      heroMana = 5*manaBlipSize;
      monsterMana = 5*manaBlipSize;
      worldMana = 5*manaBlipSize;
      illusionX = -1; illusionY = -1;

      int tempx = 0, tempy = 0;
      if (level < 10) {
         // Place the stairs
         while (!isEmptyPatch(tempx, tempy) || dist(hero.x, hero.y, tempx, tempy) < 20)  {
            tempx = randGen->getInt(0, MAP_WIDTH-1);
            tempy = randGen->getInt(0, MAP_HEIGHT-1);
         }
         map[tempx][tempy] = STAIRS;
         hero.stairsx = tempx; hero.stairsy = tempy+1;
         mapModel->setProperties(tempx, tempy, true, false);

         // Place the chests
         tempx = 0; tempy = 0;
         while (!isEmptyPatch(tempx, tempy) || dist(hero.x, hero.y, tempx, tempy) < 20 || dist(hero.stairsx, hero.stairsy, tempx, tempy) < 20) {
            tempx = randGen->getInt(0, MAP_WIDTH-1);
            tempy = randGen->getInt(0, MAP_HEIGHT-1);
         }
         map[tempx][tempy] = CHEST;
         mapModel->setProperties(tempx, tempy, true, false);
         hero.dest1x = tempx; hero.dest1y = tempy+1;
         tempx = 0; tempy = 0;
         while (!isEmptyPatch(tempx, tempy) || dist(hero.x, hero.y, tempx, tempy) < 20 || dist(hero.dest1x, hero.dest1y, tempx, tempy) < 20 || dist(hero.stairsx, hero.stairsy, tempx, tempy) < 20) {
            tempx = randGen->getInt(0, MAP_WIDTH-1);
            tempy = randGen->getInt(0, MAP_HEIGHT-1);
         }
         map[tempx][tempy] = CHEST;
         mapModel->setProperties(tempx, tempy, true, false);
         hero.dest2x = tempx; hero.dest2y = tempy+1;
      } else {
      }

      // PLACE TRAPS
      for (int i = 0; i < 10; i++) {
         tempx = 0; tempy = 0;
         while (map[tempx][tempy] != BLANK) {
            tempx = randGen->getInt(0, MAP_WIDTH-1);
            tempy = randGen->getInt(0, MAP_HEIGHT-1);
         }
         map[tempx][tempy] = TRAP;
      }

      if (level < 10) {
         generateMonsters(level, 3);
      } else {
         generateMonsters(3, 1);
         generateEndBoss();
      }

      // Draw the map
      mapModel->computeFov(hero.x, hero.y);
      drawScreen();

      // Game loop
      bool turnTaken = false;
      auto& console = *(TCODConsole::root);
      while (!TCODConsole::isWindowClosed() && !nextLevel) {
         turnTaken = false;
         destx = x; desty = y;


         TCOD_key_t key = getKeyPress();

         if (key.vk == TCODK_UP || key.vk == TCODK_KP8 || key.c == 'k') {
            destx = x;
            desty = y-1;
            turnTaken = true;
         } else if (key.vk == TCODK_DOWN || key.vk == TCODK_KP2 || key.c == 'j') {
            destx = x;
            desty = y+1;
            turnTaken = true;
         } else if (key.vk == TCODK_LEFT || key.vk == TCODK_KP4 || key.c == 'h') {
            destx = x-1;
            desty = y;
            turnTaken = true;
         } else if (key.vk == TCODK_RIGHT || key.vk == TCODK_KP6 || key.c == 'l') {
            destx = x+1;
            desty = y;
            turnTaken = true;
         } else if (key.vk == TCODK_KP7 || key.c == 'y') {
            destx = x-1;
            desty = y-1;
            turnTaken = true;
         } else if (key.vk == TCODK_KP9 || key.c == 'u') {
            destx = x+1;
            desty = y-1;
            turnTaken = true;
         } else if (key.vk == TCODK_KP3 || key.c == 'n') {
            destx = x+1;
            desty = y+1;
            turnTaken = true;
         } else if (key.vk == TCODK_KP1 || key.c == 'b') {
            destx = x-1;
            desty = y+1;
            turnTaken = true;
         } else if (key.vk == TCODK_SPACE || key.vk == TCODK_KP5) {
            turnTaken = true;
         } else if (key.vk == TCODK_ESCAPE) {
            exit(0);
         } else if (key.c == '\'') {
            int direction = getDirection();
            if (direction != 0) {
               int diffX = ((direction-1)%3)-1;
               int diffY = 1-((direction-1)/3);
               map[x+diffX][y+diffY] = WALL;
               mapModel->setProperties(x+diffX, y+diffY, false, false);
               hero.computePath();
            }
         } else if (key.vk == TCODK_F8) {
            heroSpec = 0;
            monsterSpec = 0;
            worldSpec = 0;
            addMessage("", MessageType::NORMAL);
            addMessage("", MessageType::NORMAL);
            addMessage("You have started a new game", MessageType::IMPORTANT);
            goto gameLoop;
         } else if (key.vk == TCODK_F5) {
            fullscreen = !fullscreen;
            console.setFullscreen(fullscreen);
         } else if (key.vk == TCODK_TAB) {
            turnTaken = displaySpellMenu('j');
         } else if (key.c == 'q' || key.c == 'w' || key.c == 'e' || key.c == 'a' || key.c == 's' || key.c == 'd' || key.c == 'z' || key.c == 'x' || key.c == 'c') {
            turnTaken = displaySpellMenu(key.c);
         } else if (key.c == 'm') {
            displayMessageHistory();
         } else if (key.c == 'v') {
            if (randGen->getInt(1, 2) == 1) {
               addMessage("You: HEY!", MessageType::SPELL);
            } else {
               addMessage("You: LISTEN!", MessageType::SPELL);
            }
         }
         if (destx >= 0 && desty >= 0 && destx < MAP_WIDTH && desty < MAP_HEIGHT) {
            if (map[destx][desty] == BLANK) {
               map[x][y] = BLANK;
               map[destx][desty] = PLAYER;
               x = destx;
               y = desty;
            } else if (map[destx][desty] == MONSTER) {
               Monster* curMonster = findMonster(destx, desty);
               addMessage("You are blocked by the " + curMonster->name, MessageType::NORMAL);
            } else if (map[destx][desty] == HERO) {
               if (hero.dead) {
                  addMessage("You are blocked by the hero's corpse", MessageType::NORMAL);
               } else {
                  addMessage("You are blocked by the hero", MessageType::NORMAL);
               }
            } else if (map[destx][desty] == TRAP) {
               addMessage("You shy away from the trap", MessageType::NORMAL);
            } else if (map[destx][desty] == FIELD) {
               addMessage("You are blocked by the forcefield", MessageType::NORMAL);
            } else if (map[destx][desty] == ILLUSION) {
               map[destx][desty] = BLANK;
               illusionX = -1; illusionY = -1;
               addMessage("You disrupt the illusion", MessageType::SPELL);
            } else if (map[destx][desty] == PORTAL) {
               addMessage("You are blocked by a swirling portal", MessageType::NORMAL);
            } else if (map[destx][desty] == CHEST || map[destx][desty] == CHEST_OPEN) {
               addMessage("You are blocked by the chest", MessageType::NORMAL);
            } else if (map[destx][desty] == STAIRS || map[destx][desty] == STAIRS_UP) {
               addMessage("You are blocked by the stairs", MessageType::NORMAL);
            }
         }
         // Assuming the hero pressed a key that made a turn
         if (turnTaken) {
            // Regenerate mana if the player is close to the hero
            if (hero.inSpellRadius()) {
               // Mana regeneration is faster if the hero is meditating
               if (hero.meditationTimer > 0) {
                  heroMana += 2;
                  monsterMana += 2;
                  worldMana += 2;
               } else {
                  heroMana++;
                  monsterMana++;
                  worldMana++;
               }
               // Ensures the mana generation doesn't go over te limit
               if (heroMana > 5*manaBlipSize) heroMana = 5*manaBlipSize;
               if (monsterMana > 5*manaBlipSize) monsterMana = 5*manaBlipSize;
               if (worldMana > 5*manaBlipSize) worldMana = 5*manaBlipSize;
            }
            // If the hero isn't dead, make him move
            if (hero.dead == false) {
               nextLevel = hero.move(); 
               // If the hero finished the level, go to the next level
               if (level == 10 && bossDead) {
                  nextLevel = true;
               }
            }
            // Move monsters
            for (Monster& monster : monsterList) {
               monsterMove(monster);
            }

            // Lowers the amount of clouds and forcefields
            for (int i = 0; i < MAP_WIDTH; i++) {
               for (int j = 0; j < MAP_HEIGHT; j++) {
                  if (cloud[i][j] > 0) {
                     cloud[i][j]--;
                     if (cloud[i][j] == 0) {
                        mapModel->setProperties(i, j, true, true);
                     }
                  }
                  if (field[i][j] > 0) {
                     field[i][j]--;
                     if (field[i][j] == 0) {
                        mapModel->setProperties(i, j, true, true);
                        map[i][j] = BLANK;
                     }
                  }
               }
            }
         }
         if (nextLevel && level < 10) {
            // Display the next level
            addMessage("Hero: " + Hero::heroExit[randGen->getInt(0, 4)], MessageType::HERO);
            addMessage("The hero descends to the next level of the dungeon!", MessageType::IMPORTANT);
         } else {
            mapModel->computeFov(hero.x, hero.y);
            drawScreen();
         }
      }
   }
   drawScreen();
   if (bossDead) {
      showVictoryScreen();
   }
}

void drawScreen() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   // Clear the screen
   console.clear();

   // Draw the border
   console.setDefaultForeground(TCODColor::darkGrey);
   for (int i = LEFT-1; i < RIGHT+2; i++) {
      console.putChar(i, TOP-1, 178, TCOD_BKGND_NONE);
      console.putChar(i, BOTTOM+1, 178, TCOD_BKGND_NONE);
   }
   for (int i = TOP-1; i < BOTTOM+2; i++) {
      console.putChar(LEFT-1, i, 178, TCOD_BKGND_NONE);
      console.putChar(RIGHT+1, i, 178, TCOD_BKGND_NONE);
   }
   console.setDefaultBackground(TCODColor(60, 30, 20));
   for (int i = 0; i < 80; i++) {
      console.putChar(i, 59, ' ', TCOD_BKGND_SET);
   }
   console.setDefaultForeground(TCODColor::white);
   console.setAlignment(TCOD_LEFT);
   console.setBackgroundFlag(TCOD_BKGND_NONE);
   console.print(LEFT+MAP_WIDTH/2-38, 59, "  ( ) Message History   (   ) Spell Menu   (  ) Fullscreen   (  ) Restart  "s);
   console.setDefaultForeground(TCODColor::yellow);
   console.print(05, 59,"m"s);
   console.print(27, 59, "TAB"s);
   console.print(46, 59, "F5"s);
   console.print(64, 59, "F8"s);

   // Print the title
   console.setDefaultBackground(TCODColor::darkGrey);
   console.setDefaultForeground(TCODColor::white);
   {
      WithBackgroundSet set(console);
      console.print(LEFT+MAP_WIDTH/2-7, 1, " DungeonMinder "s); 
      console.printf(LEFT+MAP_WIDTH/2+25, 1, " Level %d ", level); 
   }
   // Draw the key instructions at the bottom

   // Draw the walls
   for (int i = 0; i < MAP_WIDTH; i++) {
      for (int j = 0; j < MAP_HEIGHT; j++) {
         int floorColour = (int)(floorNoise[i][j]*40);
         console.setDefaultBackground(TCODColor(floorColour, floorColour, floorColour));
         if (!hero.dead && mapModel->isInFov(i, j)) {
            float intensity = 150.0f/pow((pow((i-hero.x), 2)+pow((j-hero.y),2)), 0.3);
            console.setDefaultBackground(TCODColor((int)(floorColour+intensity), (int)(floorColour+intensity), floorColour));
         }
         if (cloud[i][j] > 0) {
            console.setDefaultBackground(TCODColor(100, 150, 100));
            if (map[i][j] == BLANK) {
               console.setDefaultForeground(TCODColor(80, 100, 80));
               console.putChar(i+LEFT, j+TOP, 177, TCOD_BKGND_SET);
            } else {
               console.putChar(i+LEFT, j+TOP, ' ', TCOD_BKGND_SET);
            }
         }
         if (map[i][j] == WALL) {
            console.setDefaultForeground(TCODColor(160-(int)(wallNoise[i][j]*30), 90+(int)(wallNoise[i][j]*30), 90+(int)(wallNoise[i][j]*30)));
            console.putChar(i+LEFT, j+TOP, 219, TCOD_BKGND_SET);
         } else if (map[i][j] == STAIRS_UP) {
            console.setDefaultForeground(TCODColor(200, 200, 200));
            console.putChar(i+LEFT, j+TOP, '<', TCOD_BKGND_SET);
         } else if (map[i][j] == CHEST) {
            console.setDefaultForeground(TCODColor::lightBlue);
            console.putChar(i+LEFT, j+TOP, 127, TCOD_BKGND_SET);
         } else if (map[i][j] == CHEST_OPEN) {
            console.setDefaultForeground(TCODColor(128, 64, 0));
            console.putChar(i+LEFT, j+TOP, 127, TCOD_BKGND_SET);
         } else if (map[i][j] == TRAP) {
            console.setDefaultForeground(TCODColor(100,150,100));
            console.putChar(i+LEFT, j+TOP, 207, TCOD_BKGND_SET);
         } else if (map[i][j] == FIELD) {
            console.setDefaultForeground(TCODColor(50, 250, 200));
            console.putChar(i+LEFT, j+TOP, 177, TCOD_BKGND_NONE);
         } else if (map[i][j] == ILLUSION) {
            console.setDefaultForeground(TCODColor(50,250,200));
            console.putChar(i+LEFT, j+TOP, 127, TCOD_BKGND_SET);
         } else if (map[i][j] == PORTAL) {
            console.setDefaultForeground(TCODColor::red);
            console.putChar(i+LEFT, j+TOP, 245, TCOD_BKGND_SET);
         } else if (map[i][j] == MARKER1) {
            console.setDefaultForeground(TCODColor::red);
            console.putChar(i+LEFT, j+TOP, 219, TCOD_BKGND_SET);
            console.setDefaultForeground(TCODColor::grey);
         } else if (map[i][j] == MARKER2) {
            console.setDefaultForeground(TCODColor::lightBlue);
            console.putChar(i+LEFT, j+TOP, 219, TCOD_BKGND_SET);
            console.setDefaultForeground(TCODColor::grey);
         } else {
            if (cloud[i][j] == 0) {
               console.putChar(i+LEFT, j+TOP, ' ', TCOD_BKGND_SET);
            }
         }
      }
   }
   console.setDefaultBackground(TCODColor::black);

   // Show the stairs
   if (level < 10) {
      console.setDefaultForeground(TCODColor::white);
      console.putChar(hero.stairsx+LEFT, hero.stairsy-1+TOP, '>', TCOD_BKGND_NONE);
   }


   // Show the player
   console.setDefaultForeground(TCODColor::white);
   console.putChar(playerX+LEFT, playerY+TOP, TCOD_CHAR_LIGHT, TCOD_BKGND_NONE);

   // Show the hero
   if (hero.dead) {
      console.setDefaultForeground(TCODColor::black);
      console.setDefaultBackground(TCODColor::darkRed);
      console.putChar(hero.x+LEFT, hero.y+TOP, '@', TCOD_BKGND_SET);
   } else {
      console.setDefaultForeground(TCODColor(130-hero.health, (hero.health*12), (25*hero.health)));
      if (hero.shieldTimer > 0) {
         console.setDefaultBackground(TCODColor(50, 250, 200));
      } else {
         console.setDefaultBackground(TCODColor::lightYellow);
      }
      console.putChar(hero.x+LEFT, hero.y+TOP, '@', TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);

   // Show monsters
   for (const Monster& monster : monsterList) {
      // If the monster has spawned
      if (monster.portalTimer == 0) {
         console.setDefaultForeground(TCODColor(monster.health*100/monster.maxhealth+155, 155-(155*monster.health/monster.maxhealth), 155-(155*monster.health/monster.maxhealth)));
         console.putChar(monster.x+LEFT, monster.y+TOP, monster.symbol, TCOD_BKGND_NONE);
      }
   }

   // Display the stat line
   displayStatLine(BOTTOM+2);

   // Print messages
   for (int i = MESSAGE_COUNT-8; i < MESSAGE_COUNT; i++) {
      console.setDefaultForeground(MESSAGE_COLOR.at(messageType[i]));
      console.print(3, BOTTOM - 16+i, messageList[i]);
   }

   if (hero.dead) {
      //console.setFade(200,TCODColor::darkRed);
      console.setDefaultBackground(TCODColor(84, 40, 0));
      for (int i = 52; i < 77; i++) {
         console.putChar(i, 50, ' ', TCOD_BKGND_SET);
      }
      for (int j = 51; j < 57; j++) {
         console.putChar(52, j, ' ', TCOD_BKGND_SET);
      }
      console.setDefaultBackground(TCODColor(210, 96, 0));
      for (int i = 52; i < 77; i++) {
         console.putChar(i, 57, ' ', TCOD_BKGND_SET);
      }
      for (int j = 51; j < 57; j++) {
         console.putChar(76, j, ' ', TCOD_BKGND_SET);
      }
      console.setDefaultBackground(TCODColor(128, 64, 0));
      for (int i = 53; i < 76; i++) {
         for (int j = 51; j < 57; j++) {
            console.putChar(i, j, ' ', TCOD_BKGND_SET);
         }
      }
      console.setDefaultForeground(TCODColor(180, 100, 0));
      console.putChar(53, 51, 201, TCOD_BKGND_NONE);
      console.putChar(75, 51, 187, TCOD_BKGND_NONE);
      console.putChar(53, 56, 200, TCOD_BKGND_NONE);
      console.putChar(75, 56, 188, TCOD_BKGND_NONE);
      for (int i = 54; i < 75; i++) {
         console.putChar(i, 51, 205, TCOD_BKGND_NONE);
         console.putChar(i, 56, 205, TCOD_BKGND_NONE);
      }
      for (int j = 52; j < 56; j++) {
         console.putChar(53, j, 186, TCOD_BKGND_NONE);
         console.putChar(75, j, 186, TCOD_BKGND_NONE);
      }


      console.setDefaultForeground(TCODColor::white);
      console.print(56, 52, "The hero has died!"s);
      console.print(54, 54, "Press (  ) to restart"s);
      console.print(57, 55, "or (   ) to quit"s);
      console.setDefaultForeground(TCODColor::yellow);
      console.print(61, 54, "F8"s);
      console.print(61, 55, "ESC"s);
      console.setDefaultBackground(TCODColor::black);
   } else {
      TCODConsole::setFade(255,TCODColor::grey);
   }
   TCODConsole::flush();
}

Monster* heroFindMonster() {
   for (int i = 0; i < MAP_WIDTH; i++) {
      for (int j = 0; j < MAP_HEIGHT; j++) {
         if (mapModel->isInFov(i, j) && map[i][j] == MONSTER) {
            return findMonster(i, j);
         }
      }
   }
   return nullptr;
}

void monsterMove(Monster& curMonster) {
   // If the monster is still spawning, resolve it
   if (curMonster.portalTimer > 0) {
      curMonster.portalTimer--;
      // Once the monster has spawned
      if (curMonster.portalTimer == 0) {
         mapModel->setProperties(curMonster.x, curMonster.y, true, true);
         map[curMonster.x][curMonster.y] = MONSTER;
      }
   } else {
      // Otherwise, the monster is not spawning
      if (curMonster.conditionTimers[Condition::SLEEPING] == 0) {
         if (curMonster.timer == 0) {
            if (curMonster.symbol != '*') {
               bool rangedAttack = false;
               map[curMonster.x][curMonster.y] = BLANK;
               int diffx = 0, diffy = 0;

               if (curMonster.conditionTimers[Condition::HALTED] > 0) {
                  if (dist(hero.x, hero.y, curMonster.x, curMonster.y) == 1.0) {
                     if (hero.x > curMonster.x) diffx = 1;
                     if (hero.x < curMonster.x) diffx = -1;
                     if (hero.y > curMonster.y) diffy = 1;
                     if (hero.y < curMonster.y) diffy = -1;
                  }
               } else if (curMonster.conditionTimers[Condition::FLEEING] > 0 && mapModel->isInFov(curMonster.x, curMonster.y)) {
                  if (hero.x > curMonster.x) diffx = -1;
                  if (hero.x < curMonster.x) diffx = 1;
                  if (hero.y > curMonster.y) diffy = -1;
                  if (hero.y < curMonster.y) diffy = 1;
               } else if (curMonster.conditionTimers[Condition::RAGED] > 0 || curMonster.conditionTimers[Condition::ALLIED] > 0) {
                  float heroDist = dist(hero.x, hero.y, curMonster.x, curMonster.y);
                  float nearestMonsterDist = 500.0;
                  Monster* nearestMonster;
                  mapModel->computeFov(curMonster.x, curMonster.y);
                  for (Monster& monster : monsterList) {
                     int monX = monster.x;
                     int monY = monster.y;
                     if (&monster != &curMonster && mapModel->isInFov(monX, monY)) {
                        float tempdist = dist(curMonster.x, curMonster.y, monX, monY);
                        if (tempdist < nearestMonsterDist) {
                           nearestMonsterDist = tempdist;
                           nearestMonster = &monster;
                        }
                     }
                  }
                  mapModel->computeFov(hero.x, hero.y);
                  if (heroDist < nearestMonsterDist && curMonster.conditionTimers[Condition::ALLIED] == 0) {
                     if (heroDist == 1.0 || curMonster.conditionTimers[Condition::BLINDED] == 0) {
                        if (hero.x > curMonster.x) diffx = 1;
                        if (hero.x < curMonster.x) diffx = -1;
                        if (hero.y > curMonster.y) diffy = 1;
                        if (hero.y < curMonster.y) diffy = -1;
                     } else {
                        diffx = randGen->getInt(-1, 1);
                        diffy = randGen->getInt(-1, 1);
                     }
                  } else {
                     if (nearestMonsterDist == 1.0 || curMonster.conditionTimers[Condition::BLINDED] == 0) {
                        if (nearestMonster->x > curMonster.x) diffx = 1;
                        if (nearestMonster->x < curMonster.x) diffx = -1;
                        if (nearestMonster->y > curMonster.y) diffy = 1;
                        if (nearestMonster->y < curMonster.y) diffy = -1;
                     } else {
                        diffx = randGen->getInt(-1, 1);
                        diffy = randGen->getInt(-1, 1);
                     }
                  }
               } else if (curMonster.angry && !hero.dead && (curMonster.conditionTimers[Condition::BLINDED] == 0 || hero.isAdjacent(curMonster.x, curMonster.y))) {
                  if (curMonster.ranged /*&& !curMonster.maimed*/ && !hero.isAdjacent(curMonster.x, curMonster.y) && dist(curMonster.x, curMonster.y, hero.x, hero.y) <= curMonster.range && mapModel->isInFov(curMonster.x, curMonster.y)) {
                     rangedAttack = true;
                  }
                  if (hero.x > curMonster.x) diffx = 1;
                  if (hero.x < curMonster.x) diffx = -1;
                  if (hero.y > curMonster.y) diffy = 1;
                  if (hero.y < curMonster.y) diffy = -1;
               } else {
                  if (mapModel->isInFov(curMonster.x, curMonster.y) && (curMonster.conditionTimers[Condition::BLINDED] == 0 || hero.isAdjacent(curMonster.x, curMonster.y))) {
                     if (curMonster.symbol != '@' || curMonster.health != curMonster.maxhealth) {
                        curMonster.angry = true;
                     }
                  }
                  diffx = randGen->getInt(-1, 1);
                  diffy = randGen->getInt(-1, 1);
               }
               if (diffx != 0 && diffy != 0) {
                  if (randGen->getInt(0, 1) == 0) {
                     diffx = 0;
                  } else {
                     diffy = 0;
                  }
               }
               if (rangedAttack) {
                  displayRangedAttack(hero.x, hero.y, curMonster.x, curMonster.y);
                  int damage = curMonster.damage - (int)ceil((double)curMonster.conditionTimers[Condition::WEAKENED]/CONDITION_TIMES.at(Condition::WEAKENED));
                  if (hero.shieldTimer == 0) {
                     if (damage < 0) damage = 0;
                     hero.health -= damage;
                     sprintf(charBuffer, "%d", damage);
                     addMessage("The " + curMonster.name + " " + curMonster.rangedName + " at the hero for " + charBuffer + " damage", MessageType::NORMAL);
                     if (hero.health <= 0) {
                        hero.dead = true;
                        addMessage("The hero has died!", MessageType::IMPORTANT);
                        drawScreen();
                     } else if (hero.meditationTimer > 0) {
                        hero.meditationTimer = 0;
                        addMessage("The hero's meditation is interrupted!", MessageType::SPELL);
                     }
                  } else {
                     hero.shieldTimer -= curMonster.damage;
                     if (hero.shieldTimer <= 0) {
                        hero.shieldTimer = 0;
                        addMessage("The shield is shattered by the " + curMonster.name + "'s attack!", MessageType::SPELL);
                        drawScreen();
                     } else {
                        addMessage("The " + curMonster.name + "'s attack is deflected by the shield", MessageType::SPELL);
                     }
                  }
                  if (curMonster.maimed) {
                     addMessage("The "+curMonster.name+" suffers from the exertion!", MessageType::NORMAL);
                     hitMonster(curMonster.x, curMonster.y, damage);
                  }
               } else {
                  if (curMonster.x+diffx == hero.x && curMonster.y+diffy == hero.y && curMonster.conditionTimers[Condition::ALLIED] == 0) {
                     if (hero.dead) {
                        hero.health -= curMonster.damage;
                        sprintf(charBuffer, "%d", curMonster.damage);
                        if (curMonster.symbol != '@') {
                           addMessage("The " + curMonster.name + " savages the hero's corpse", MessageType::NORMAL);
                        }
                     } else {
                        int damage = curMonster.damage - (int)ceil((double)curMonster.conditionTimers[Condition::WEAKENED]/CONDITION_TIMES.at(Condition::WEAKENED));
                        if (damage < 0) damage = 0;
                        hero.health -= damage;
                        sprintf(charBuffer, "%d", damage);
                        addMessage("The " + curMonster.name + " hits the hero for " + charBuffer + " damage", MessageType::NORMAL);
                        if (hero.health <= 0) {
                           hero.dead = true;
                           addMessage("The hero has died!", MessageType::IMPORTANT);
                           drawScreen();
                        } else if (hero.meditationTimer > 0) {
                           hero.meditationTimer = 0;
                           addMessage("The hero's meditation is interrupted!", MessageType::SPELL);
                        } else if (&curMonster != hero.target && hero.target != nullptr) {
                           if (dist(hero.x, hero.y, curMonster.x, curMonster.y) < dist(hero.x, hero.y, hero.target->x, hero.target->y)) {
                              hero.target = &curMonster;
                           }
                        }
                        if (curMonster.maimed) {
                           addMessage("The "+curMonster.name+" suffers from the exertion!", MessageType::NORMAL);
                           hitMonster(curMonster.x, curMonster.y, damage);
                        }
                     }
                  } else if (map[curMonster.x+diffx][curMonster.y+diffy] == MONSTER && (diffx != 0 || diffy != 0)) {
                     Monster* otherMonster = findMonster(curMonster.x+diffx, curMonster.y+diffy);
                     if (curMonster.conditionTimers[Condition::RAGED] > 0 || curMonster.conditionTimers[Condition::ALLIED] > 0) {
                        sprintf(charBuffer, "%d", curMonster.damage);
                        addMessage("The " + curMonster.name + " hits the " + otherMonster->name + " for " +charBuffer + " damage", MessageType::NORMAL);
                        hitMonster(curMonster.x+diffx, curMonster.y+diffy, curMonster.damage);
                     } else {
                        addMessage("The " + curMonster.name + " bumps into the " + otherMonster->name, MessageType::NORMAL);
                     }
                  } else if (map[curMonster.x+diffx][curMonster.y+diffy] == PLAYER) {
                     std::swap(playerX, curMonster.x);
                     std::swap(playerY, curMonster.y);
                     addMessage("The " + curMonster.name + " passes through you", MessageType::NORMAL);
                     map[playerX][playerY] = PLAYER;
                  } else if (map[curMonster.x+diffx][curMonster.y+diffy] == TRAP) {
                     addMessage("The " + curMonster.name+ " falls into the trap!", MessageType::NORMAL);
                     map[curMonster.x+diffx][curMonster.y+diffy] = BLANK;
                     curMonster.x+=diffx;
                     curMonster.y+=diffy;
                     hitMonster(curMonster.x, curMonster.y, 4);
                  } else if (map[curMonster.x+diffx][curMonster.y+diffy] == ILLUSION) {
                     map[curMonster.x+diffx][curMonster.y+diffy] = BLANK;
                     illusionX = -1; illusionY = -1;
                     addMessage("The "+curMonster.name+" disrupts the illusion", MessageType::SPELL);
                  } else if (map[curMonster.x+diffx][curMonster.y+diffy] == BLANK) {
                     curMonster.x+=diffx;
                     curMonster.y+=diffy;
                  }
               }
               map[curMonster.x][curMonster.y] = MONSTER;
            } else {
               if (monsterList.size() < MAX_MONSTERS) {
                  int lx = 0, ly = 0;
                  while (map[lx][ly] != BLANK) {
                     lx = randGen->getInt(0, MAP_WIDTH-1);
                     ly = randGen->getInt(0, MAP_HEIGHT-1);
                  }
                  addSpecifiedMonster(lx, ly, randGen->getInt(0, 12), true);
               }
            }
            curMonster.timer = curMonster.wait;

            for (auto& [condition, timer]: curMonster.conditionTimers) {
               if (timer > 0) {
                  timer--;
                  if (timer == 0) {
                     const auto& text = CONDITION_END.at(condition);
                     addMessage(text.first + curMonster.name + text.second, MessageType::SPELL);
                  }
               }
            }

         }
         curMonster.timer -= 1;
      } else {
         curMonster.conditionTimers[Condition::SLEEPING]--;
         if (curMonster.conditionTimers[Condition::SLEEPING] == 0) {
            auto& text = CONDITION_END.at(Condition::SLEEPING);
            addMessage(text.first + curMonster.name + text.second, MessageType::SPELL);
         }
      }
   }
}

void addMessage(const std::string& message, MessageType type) {
   for (int i = 0; i < MESSAGE_COUNT-1; i++) {
      messageList[i] = messageList[i+1];
      messageType[i] = messageType[i+1];
   }
   messageList[MESSAGE_COUNT-1] = message;
   messageType[MESSAGE_COUNT-1] = type;
}

void addMonster(const std::string& name, char symbol, int x, int y, int health, int damage, bool ranged, const std::string& rangedName, float range, int wait, bool portalSpawned) {
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

Monster* findMonster(int x, int y) {
   auto found = std::ranges::find_if(
      monsterList,
      [x, y](const auto& monster) {return monster.x == x && monster.y == y;}
   );

   return (found != monsterList.end())
    ? &(*found)
    : nullptr;
}

void hitMonster(int x, int y, int amount) {
   if (Monster* curMonster = findMonster(x, y); curMonster != nullptr) {
      curMonster->health -= amount;
      if (curMonster->health <= 0) {
         if (curMonster->symbol == '*' || curMonster->symbol=='M' || curMonster->symbol=='@') {
            bossDead = true;
         }
         addMessage("The " + curMonster->name + " dies!", MessageType::NORMAL);
         if (hero.target == curMonster) {
            hero.target = nullptr;
            hero.computePath();
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
         addMessage("The attack allows the "+curMonster->name + " to move again", MessageType::SPELL); 
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
            for (int j = y1+randGen->getInt(1, 2); j <= y2-randGen->getInt(1, 2); j++) {
               map[i][j] = BLANK;
               mapModel->setProperties(i, j, true, true);
            }
         }
         for (int j = y1+2; j <= y2-2; j++) {
            if (randGen->getInt(1, 2) == 1) {
               map[x1+1][j] = BLANK;
               mapModel->setProperties(x1+1, j, true, true);
            }
            if (randGen->getInt(1, 2) == 1) {
               map[x2-1][j] = BLANK;
               mapModel->setProperties(x2-1, j, true, true);
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
               map[i][y] = BLANK;
               mapModel->setProperties(i, y, true, true);
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
               map[x][j] = BLANK;
               mapModel->setProperties(x, j, true, true);
            }
         }
      }
   }
}

void createSpellMenu() {
   // Instantiate the spellMenu
   spellMenu = new TCODConsole(66, 35);

   // Draw the edges
   spellMenu->setDefaultBackground(TCODColor(84, 40, 0));
   for (int i = 0; i <= 65; i++) {
      spellMenu->putChar(i, 0, ' ', TCOD_BKGND_SET);
   }
   for (int j = 1; j <= 34; j++) {
      spellMenu->putChar(0, j, ' ', TCOD_BKGND_SET);
   }
   spellMenu->setDefaultBackground(TCODColor(210, 96, 0));
   for (int i = 0; i <= 65; i++) {
      spellMenu->putChar(i, 34, ' ', TCOD_BKGND_SET);
   }
   for (int j = 1; j <= 33; j++) {
      spellMenu->putChar(65, j, ' ', TCOD_BKGND_SET);
   }

   // Draw the backboard
   spellMenu->setDefaultBackground(TCODColor(128, 64, 0));
   for (int i = 1; i <= 64; i++) {
      for (int j = 1; j <= 33; j++) {
         spellMenu->putChar(i, j, ' ', TCOD_BKGND_SET);
      }
   }

}

bool isEmptyPatch(int x, int y) {
   bool result = false;
   if (x > 0 && x < MAP_WIDTH && y > 0 && y < MAP_HEIGHT) {
      result = map[x-1][y-1] == BLANK && map[x-1][y] == BLANK && map[x-1][y+1] == BLANK && map[x][y-1] == BLANK && map[x][y] == BLANK && map[x][y+1] == BLANK && map[x+1][y-1] == BLANK && map[x+1][y] == BLANK && map[x+1][y+1] == BLANK;
   }
   return result;
}

void displayStatLine(int row) {
   using namespace std::string_literals;

   auto& console = *(TCODConsole::root);

   // Print stats
   if (hero.dead) {
      console.setDefaultForeground(TCODColor::black);
      console.setDefaultBackground(TCODColor::darkRed);
      {
         WithBackgroundSet set(console);
         console.print(9, row, "  The hero is dead  "s);
      }
      console.setDefaultBackground(TCODColor::black);
   } else {
      console.setDefaultForeground(TCODColor::white);
      console.print(13, row, "Hero health:"s);
      console.setDefaultForeground(TCODColor(130-hero.health, (hero.health*12), (25*hero.health)));
      console.putChar(26, row, 195, TCOD_BKGND_NONE);
      console.putChar(32, row, 180, TCOD_BKGND_NONE);
      console.setDefaultBackground(TCODColor(130-hero.health, (hero.health*12), (25*hero.health)));
      console.setDefaultForeground(TCODColor::black);
      for (int i = 0; i < hero.health; i+=2) {
         console.putChar(27+(i/2), row, 224, TCOD_BKGND_SET);
      }
      if (hero.items.contains(Item::healthCap)) {
         console.setDefaultForeground(TCODColor(100, 200, 100));
         console.setDefaultBackground(TCODColor::black);
         console.putChar(31, row, 'X', TCOD_BKGND_SET);
      }
   }
   console.setDefaultForeground(
      hero.inSpellRadius()
         ? TCODColor::white
         : TCODColor::red
   );
   console.print(40, row, "Power: "s);
   console.setDefaultForeground(TCODColor::lightBlue);
   console.putChar(47, row, 195, TCOD_BKGND_NONE);
   console.putChar(53, row, 180, TCOD_BKGND_NONE);
   console.setDefaultBackground(TCODColor::lightBlue);
   console.setDefaultForeground(TCODColor::black);
   for (int i = 0; i < heroMana/manaBlipSize; i++) {
      console.putChar(48+i, row, 224, TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);
   console.setDefaultForeground(TCODColor::red);
   console.putChar(55, row, 195, TCOD_BKGND_NONE);
   console.putChar(61, row, 180, TCOD_BKGND_NONE);
   console.setDefaultBackground(TCODColor::red);
   console.setDefaultForeground(TCODColor::black);
   for (int i = 0; i < monsterMana/manaBlipSize; i++) {
      console.putChar(56+i, row, 224, TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);
   console.setDefaultForeground(TCODColor(156, 156, 156));
   console.putChar(63, row, 195, TCOD_BKGND_NONE);
   console.putChar(69, row, 180, TCOD_BKGND_NONE);
   console.setDefaultBackground(TCODColor(156, 156, 156));
   console.setDefaultForeground(TCODColor::black);
   for (int i = 0; i < worldMana/manaBlipSize; i++) {
      console.putChar(64+i, row, 224, TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);
}

void displayMessageHistory() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   // Clear a blank space
   console.setDefaultBackground(TCODColor::black);
   for (int j = 28; j < 59; j++) {
      for (int i = 0; i < 80; i++) {
         console.putChar(i, j, ' ', TCOD_BKGND_SET);
      }
   }

   // Block off the bottom of the map
   console.setDefaultBackground(TCODColor::black);
   console.setDefaultForeground(TCODColor::darkGrey);
   for (int i = LEFT-1; i < RIGHT+2; i++) {
      console.putChar(i, 27, 178, TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::darkGrey);
   console.setDefaultForeground(TCODColor::white);
   {
      WithBackgroundSet set(console);
      console.print(LEFT+MAP_WIDTH/2-22, 27, " Message History   (                      ) "s); 
      console.setDefaultForeground(TCODColor::yellow);
      console.print(LEFT+MAP_WIDTH/2-2, 27, "press any key to close"s); 
   }
   console.setDefaultBackground(TCODColor::black);

   console.setDefaultForeground(TCODColor::darkGrey);
   console.print(05, 59,"m"s);
   console.print(27, 59,"TAB"s);
   console.print(46, 59,"F5"s);
   console.print(64, 59,"F8"s);

   // Display the stat line
   displayStatLine(28);

   // Display the messages
   for (int i = 0; i < MESSAGE_COUNT; i++) {
      console.setDefaultForeground(MESSAGE_COLOR.at(messageType[i]));
      console.print(3, 30+i,messageList[i]);
   }

   // Flush the root console
   console.flush();

   // Wait for keyboard response
   TCOD_key_t key = getKeyPress();
}

void drawCommonGUI() {
   using namespace std::string_literals;

   auto& console = *(TCODConsole::root);

   console.setDefaultForeground(TCODColor(180, 100, 0));
   for (int i = menuX+2; i <= menuX + 63; i++) {
      console.putChar(i, menuY+1, 205, TCOD_BKGND_NONE);
      console.putChar(i, menuY+3, 205, TCOD_BKGND_NONE);
      console.putChar(i, menuY+33, 205, TCOD_BKGND_NONE);
      console.putChar(i, menuY+13, 196, TCOD_BKGND_NONE);
      console.putChar(i, menuY+23, 196, TCOD_BKGND_NONE);
   }
   for (int j = menuY+2; j <= menuY + 32; j++) {
      console.putChar(menuX+1, j, 186, TCOD_BKGND_NONE);
      console.putChar(menuX+22, j, 186, TCOD_BKGND_NONE);
      console.putChar(menuX+43, j, 186, TCOD_BKGND_NONE);
      console.putChar(menuX+64, j, 186, TCOD_BKGND_NONE);
   }
   console.putChar(menuX+1, menuY+1, 201, TCOD_BKGND_NONE);
   console.putChar(menuX+64, menuY+1, 187, TCOD_BKGND_NONE);
   console.putChar(menuX+1, menuY+33, 200, TCOD_BKGND_NONE);
   console.putChar(menuX+64, menuY+33, 188, TCOD_BKGND_NONE);
   console.putChar(menuX+22, menuY+1, 203, TCOD_BKGND_NONE);
   console.putChar(menuX+43, menuY+1, 203, TCOD_BKGND_NONE);
   console.putChar(menuX+22, menuY+3, 206, TCOD_BKGND_NONE);
   console.putChar(menuX+43, menuY+3, 206, TCOD_BKGND_NONE);
   console.putChar(menuX+22, menuY+33, 202, TCOD_BKGND_NONE);
   console.putChar(menuX+43, menuY+33, 202, TCOD_BKGND_NONE);
   console.putChar(menuX+1, menuY+3, 204, TCOD_BKGND_NONE);
   console.putChar(menuX+64, menuY+3, 185, TCOD_BKGND_NONE);

   // Draw the common spell text
   console.setDefaultForeground(TCODColor::lightBlue);
   console.print(menuX+10, menuY+2, "Hero"s);
   console.print(menuX+18, menuY+5, "( )"s);
   console.print(menuX+18, menuY+15, "( )"s);
   console.print(menuX+18, menuY+25, "( )"s);
   console.setDefaultForeground(TCODColor::red);
   console.print(menuX+29, menuY+2, "Monster"s);
   console.print(menuX+39, menuY+5, "( )"s);
   console.print(menuX+39, menuY+15, "( )"s);
   console.print(menuX+39, menuY+25, "( )"s);
   console.setDefaultForeground(TCODColor(156, 156, 156));
   console.print(menuX+51, menuY+2, "World"s);
   console.print(menuX+60, menuY+5, "( )"s);
   console.print(menuX+60, menuY+15, "( )"s);
   console.print(menuX+60, menuY+25, "( )"s);

}

void displayUpgradeMenu() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   addMessage("You are now experienced enough to specialise your magic!", MessageType::IMPORTANT);
   drawScreen();
   // Blit the spell menu on the main console
   TCODConsole::blit(spellMenu, 0, 0, 66, 35, &console, menuX, menuY);

   // Draw the borders
   drawCommonGUI();

   // Draw the placards
   console.setDefaultForeground(TCODColor::yellow);
   console.setDefaultBackground(TCODColor(78, 78, 78));
   {
      WithBackgroundSet set(console);
      console.print(menuX+21, menuY, "  Spell Specialisation  "s);
      console.print(menuX+5, menuY+34, " Choose a specialisation or press <backspace> to cancel "s);
   }

   // Draw specialisation categories
   console.setDefaultForeground(TCODColor::lightBlue);
   console.print(menuX+3, menuY+5, "Pacifism"s);
   console.print(menuX+3, menuY+15, "Speed"s);
   console.print(menuX+3, menuY+25, "Heal"s);
   console.setDefaultForeground(TCODColor::red);
   console.print(menuX+24, menuY+5,"Blind"s);
   console.print(menuX+24, menuY+15,"Rage"s);
   console.print(menuX+24, menuY+25,"Sleep"s);
   console.setDefaultForeground(TCODColor(156, 156, 156));
   console.print(menuX+45, menuY+5,"Clear"s);
   console.print(menuX+45, menuY+15,"Cloud"s);
   console.print(menuX+45, menuY+25,"Trap"s);

   // List the spells in each
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+19, menuY+5, "q"s);
   console.print(menuX+19, menuY+15, "a"s);
   console.print(menuX+19, menuY+25, "z"s);
   if (heroSpec == 0 || heroSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+5, menuY+8, "Pacifism"s);
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (heroSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+5, menuY+9, "Meditation"s);
   console.print(menuX+5, menuY+10, "Charity"s);
   if (heroSpec == 0 || heroSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+5, menuY+19, "Speed"s);
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (heroSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+5, menuY+18, "Slow"s);
   console.print(menuX+5, menuY+20, "Blink"s);
   if (heroSpec == 0 || heroSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+5, menuY+30, "Heal"s);
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (heroSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+5, menuY+28, "Shield"s);
   console.print(menuX+5, menuY+29, "Regenerate"s);

   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+40, menuY+5, "w"s);
   console.print(menuX+40, menuY+15, "s"s);
   console.print(menuX+40, menuY+25, "x"s);
   if (monsterSpec == 0 || monsterSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+26, menuY+8, "Blind"s);
   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (monsterSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+26, menuY+9, "Maim"s);
   console.print(menuX+26, menuY+10, "Cripple"s);
   if (monsterSpec == 0 || monsterSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+26, menuY+19, "Rage"s);
   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (monsterSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+26, menuY+18, "Weaken"s);
   console.print(menuX+26, menuY+20, "Ally"s);
   if (monsterSpec == 0 || monsterSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+26, menuY+30, "Sleep"s);
   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (monsterSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+26, menuY+28, "Halt"s);
   console.print(menuX+26, menuY+29, "Flee"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+61, menuY+5, "e"s);
   console.print(menuX+61, menuY+15, "d"s);
   console.print(menuX+61, menuY+25, "c"s);
   if (worldSpec == 0 || worldSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+47, menuY+8, "Clear"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (worldSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+47, menuY+9, "Blow"s);
   console.print(menuX+47, menuY+10, "Illusion"s);
   if (worldSpec == 0 || worldSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+47, menuY+19, "Cloud"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (worldSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+47, menuY+18, "Screen"s);
   console.print(menuX+47, menuY+20, "Field"s);
   if (worldSpec == 0 || worldSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+47, menuY+30, "Trap"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (worldSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(menuX+47, menuY+28, "Tunnel"s);
   console.print(menuX+47, menuY+29, "Minefield"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }

   // Flush the main console
   console.setDefaultBackground(TCODColor::black);
   console.flush();

   // Get keyboard input
   TCOD_key_t key;
   char spell = '\0';
   while (spell == '\0') {
      key = getKeyPress();
      if (key.vk == TCODK_BACKSPACE) {
         spell = ' ';
      } else if (((key.c == 'q' || key.c == 'a' || key.c == 'z') && heroSpec == 0) || ((key.c == 'w' || key.c == 's' || key.c == 'x') && monsterSpec == 0) || ((key.c == 'e' || key.c == 'd' || key.c == 'c') && worldSpec == 0)) {
         spell = key.c;
      }
   }
   switch (spell) {
      case 'q':
         addMessage("You specialise towards Pacifism! Check your spell menu!", MessageType::IMPORTANT);
         heroSpec = 1;
         break;
      case 'a':
         addMessage("You specialise towards Speed! Check your spell menu!", MessageType::IMPORTANT);
         heroSpec = 2;
         break;
      case 'z':
         addMessage("You specialise towards Heal! Check your spell menu!", MessageType::IMPORTANT);
         heroSpec = 3;
         break;
      case 'w':
         addMessage("You specialise towards Blind! Check your spell menu!", MessageType::IMPORTANT);
         monsterSpec = 1;
         break;
      case 's':
         addMessage("You specialise towards Rage! Check your spell menu!", MessageType::IMPORTANT);
         monsterSpec = 2;
         break;
      case 'x':
         addMessage("You specialise towards Sleep! Check your spell menu!", MessageType::IMPORTANT);
         monsterSpec = 3;
         break;
      case 'e':
         addMessage("You specialise towards Clear! Check your spell menu!", MessageType::IMPORTANT);
         worldSpec = 1;
         break;
      case 'd':
         addMessage("You specialise towards Cloud! Check your spell menu!", MessageType::IMPORTANT);
         worldSpec = 2;
         break;
      case 'c':
         addMessage("You specialise towards Trap! Check your spell menu!", MessageType::IMPORTANT);
         worldSpec = 3;
         break;
      default:
         addMessage("You choose not to specialise your spells", MessageType::IMPORTANT);
         break;
   }
}

bool displaySpellMenu(char spellChar) {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   if (spellChar == 'j') {
      // Blit the spell menu on the main console
      TCODConsole::blit(spellMenu, 0, 0, 66, 35, &console, menuX, menuY);

      console.setDefaultForeground(TCODColor::darkGrey);
      console.print(05, 59, "m"s);
      console.print(27, 59, "TAB"s);
      console.print(46, 59, "F5"s);
      console.print(64, 59, "F8"s);

      // Draw the borders
      drawCommonGUI();
      console.setDefaultForeground(TCODColor(180, 100, 0));
      for (int j = 0; j < 3; j++) {
         console.putChar(menuX+9, menuY+j*10+13, 193, TCOD_BKGND_NONE);
         console.putChar(menuX+21, menuY+j*10+13, 193, TCOD_BKGND_NONE);
         console.putChar(menuX+9, menuY+j*10+12, 179, TCOD_BKGND_NONE);
         console.putChar(menuX+21, menuY+j*10+12, 179, TCOD_BKGND_NONE);
         console.putChar(menuX+9, menuY+j*10+11, 218, TCOD_BKGND_NONE);
         console.putChar(menuX+21, menuY+j*10+11, 191, TCOD_BKGND_NONE);
         for (int i = menuX+10; i <= menuX+ 20; i++) {
            console.putChar(i, menuY+j*10+11, 196, TCOD_BKGND_NONE);
         }
         console.putChar(menuX+30, menuY+j*10+13, 193, TCOD_BKGND_NONE);
         console.putChar(menuX+42, menuY+j*10+13, 193, TCOD_BKGND_NONE);
         console.putChar(menuX+30, menuY+j*10+12, 179, TCOD_BKGND_NONE);
         console.putChar(menuX+42, menuY+j*10+12, 179, TCOD_BKGND_NONE);
         console.putChar(menuX+30, menuY+j*10+11, 218, TCOD_BKGND_NONE);
         console.putChar(menuX+42, menuY+j*10+11, 191, TCOD_BKGND_NONE);
         for (int i = menuX+31; i <= menuX+ 41; i++) {
            console.putChar(i, menuY+j*10+11, 196, TCOD_BKGND_NONE);
         }
         console.putChar(menuX+51, menuY+j*10+13, 193, TCOD_BKGND_NONE);
         console.putChar(menuX+63, menuY+j*10+13, 193, TCOD_BKGND_NONE);
         console.putChar(menuX+51, menuY+j*10+12, 179, TCOD_BKGND_NONE);
         console.putChar(menuX+63, menuY+j*10+12, 179, TCOD_BKGND_NONE);
         console.putChar(menuX+51, menuY+j*10+11, 218, TCOD_BKGND_NONE);
         console.putChar(menuX+63, menuY+j*10+11, 191, TCOD_BKGND_NONE);
         for (int i = menuX+52; i <= menuX+ 62; i++) {
            console.putChar(i, menuY+j*10+11, 196, TCOD_BKGND_NONE);
         }
      }
      // Draw the placards
      console.setDefaultForeground(TCODColor::yellow);
      console.setDefaultBackground(TCODColor(78, 78, 78));
      {
         WithBackgroundSet set(console);
         console.print(menuX+24, menuY, "  List of Spells  "s);
         console.print(menuX+36, menuY+34, " Any other key to cancel "s);
      }

      // Draw the blips
      console.setDefaultForeground(TCODColor(128, 64, 0));
      console.setDefaultBackground(TCODColor::lightBlue);
      console.putChar(menuX+18, menuY+12, 224, TCOD_BKGND_SET);
      console.putChar(menuX+17, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+18, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+19, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+16, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+17, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+18, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+19, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+20, menuY+32, 224, TCOD_BKGND_SET);
      console.setDefaultBackground(TCODColor::red);
      console.putChar(menuX+39, menuY+12, 224, TCOD_BKGND_SET);
      console.putChar(menuX+38, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+39, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+40, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+37, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+38, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+39, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+40, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+41, menuY+32, 224, TCOD_BKGND_SET);
      console.setDefaultBackground(TCODColor(156, 156, 156));
      console.putChar(menuX+60, menuY+12, 224, TCOD_BKGND_SET);
      console.putChar(menuX+59, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+60, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+61, menuY+22, 224, TCOD_BKGND_SET);
      console.putChar(menuX+58, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+59, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+60, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+61, menuY+32, 224, TCOD_BKGND_SET);
      console.putChar(menuX+62, menuY+32, 224, TCOD_BKGND_SET);

      console.setDefaultForeground(TCODColor::white);
      console.print(menuX+19, menuY+5, "q"s);
      console.print(menuX+19, menuY+15, "a"s);
      console.print(menuX+19, menuY+25, "z"s);
      console.print(menuX+40, menuY+5, "w"s);
      console.print(menuX+40, menuY+15, "s"s);
      console.print(menuX+40, menuY+25, "x"s);
      console.print(menuX+61, menuY+5, "e"s);
      console.print(menuX+61, menuY+15, "d"s);
      console.print(menuX+61, menuY+25, "c"s);

      for (int i = 10; i <= 52; i+=21) {
         for (int j = 12; j <=32; j+=10) {
            console.print(menuX+i, menuY+j, "Cost:"s);
         }

      }
      // DRAW THE SPELL INFORMATION
      if (heroSpec == 0) {
         console.setDefaultForeground(TCODColor::lightBlue);
         console.print(menuX+3, menuY+5, "Pacifism"s);
         console.print(menuX+3, menuY+15, "Speed"s);
         console.print(menuX+3, menuY+25, "Heal"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+3, menuY+7, "Hero will not"s);
         console.print(menuX+3, menuY+8, "attack or pursue"s);
         console.print(menuX+3, menuY+9, "monsters"s);
         console.print(menuX+3, menuY+17, "Hero temporarily"s);
         console.print(menuX+3, menuY+18, "moves at double"s);
         console.print(menuX+3, menuY+19, "speed"s);
         console.print(menuX+3, menuY+27, "Hero recovers from"s);
         console.print(menuX+3, menuY+28, "some injuries"s);

      } else if (heroSpec == 1) {
         console.setDefaultForeground(TCODColor::lightBlue);
         console.print(menuX+3, menuY+5, "Pacifism"s);
         console.print(menuX+3, menuY+15, "Meditation"s);
         console.print(menuX+3, menuY+25, "Charity"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+3, menuY+7, "Hero will not"s);
         console.print(menuX+3, menuY+8, "attack or pursue"s);
         console.print(menuX+3, menuY+9, "monsters"s);
         console.print(menuX+3, menuY+17, "Hero stands still,"s);
         console.print(menuX+3, menuY+18, "increasing spell"s);
         console.print(menuX+3, menuY+19, "power generation"s);
         console.print(menuX+3, menuY+27, "Convert all found"s);
         console.print(menuX+3, menuY+28, "treasure into hero"s);
         console.print(menuX+3, menuY+29, "health"s);
      } else if (heroSpec == 2) {
         console.setDefaultForeground(TCODColor::lightBlue);
         console.print(menuX+3, menuY+5, "Slow"s);
         console.print(menuX+3, menuY+15, "Speed"s);
         console.print(menuX+3, menuY+25, "Blink"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+3, menuY+7, "Hero toggles"s);
         console.print(menuX+3, menuY+8, "between slow and"s);
         console.print(menuX+3, menuY+9, "normal speed"s);
         console.print(menuX+3, menuY+17, "Hero temporarily"s);
         console.print(menuX+3, menuY+18, "moves at double"s);
         console.print(menuX+3, menuY+19, "speed"s);
         console.print(menuX+3, menuY+27, "Hero makes instant"s);
         console.print(menuX+3, menuY+28, "moves and heals by"s);
         console.print(menuX+3, menuY+29, "attacking enemies"s);
      } else if (heroSpec == 3) {
         console.setDefaultForeground(TCODColor::lightBlue);
         console.print(menuX+3, menuY+5, "Shield"s);
         console.print(menuX+3, menuY+15, "Regenerate"s);
         console.print(menuX+3, menuY+25, "Heal"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+3, menuY+7, "Hero becomes"s);
         console.print(menuX+3, menuY+8, "temporarily immune"s);
         console.print(menuX+3, menuY+9, "to ranged attacks"s);
         console.print(menuX+3, menuY+17, "Hero slowly heals"s);
         console.print(menuX+3, menuY+18, "over a short"s);
         console.print(menuX+3, menuY+19, "period"s);
         console.print(menuX+3, menuY+27, "Hero recovers from"s);
         console.print(menuX+3, menuY+28, "some injuries"s);
      }
      if (monsterSpec == 0) {
         console.setDefaultForeground(TCODColor::red);
         console.print(menuX+24, menuY+5, "Blind"s);
         console.print(menuX+24, menuY+15, "Rage"s);
         console.print(menuX+24, menuY+25, "Sleep"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+24, menuY+7, "Monster will not"s);
         console.print(menuX+24, menuY+8, "approach hero or"s);
         console.print(menuX+24, menuY+9, "use ranged attacks"s);
         console.print(menuX+24, menuY+17, "Monster will"s);
         console.print(menuX+24, menuY+18, "attack the nearest"s);
         console.print(menuX+24, menuY+19, "living creature"s);
         console.print(menuX+24, menuY+27, "Monster will be"s);
         console.print(menuX+24, menuY+28, "unable to act for"s);
         console.print(menuX+24, menuY+29, "a short time"s);
      } else if (monsterSpec == 1) {
         console.setDefaultForeground(TCODColor::red);
         console.print(menuX+24, menuY+5, "Blind"s);
         console.print(menuX+24, menuY+15, "Maim"s);
         console.print(menuX+24, menuY+25, "Cripple"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+24, menuY+7, "Monster will not"s);
         console.print(menuX+24, menuY+8, "approach hero or"s);
         console.print(menuX+24, menuY+9, "use ranged attacks"s);
         console.print(menuX+24, menuY+17, "Monster suffers"s);
         console.print(menuX+24, menuY+18, "damage when"s);
         console.print(menuX+24, menuY+19, "attacking"s);
         console.print(menuX+24, menuY+27, "Monster is reduced"s);
         console.print(menuX+24, menuY+28, "to half of its"s);
         console.print(menuX+24, menuY+29, "current health"s);
      } else if (monsterSpec == 2) {
         console.setDefaultForeground(TCODColor::red);
         console.print(menuX+24, menuY+5, "Weaken"s);
         console.print(menuX+24, menuY+15, "Rage"s);
         console.print(menuX+24, menuY+25, "Ally"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+24, menuY+7, "Monster does less"s);
         console.print(menuX+24, menuY+8, "damage with each"s);
         console.print(menuX+24, menuY+9, "attack"s);
         console.print(menuX+24, menuY+17, "Monster will"s);
         console.print(menuX+24, menuY+18, "attack the nearest"s);
         console.print(menuX+24, menuY+19, "living creature"s);
         console.print(menuX+24, menuY+27, "Monster fights for"s);
         console.print(menuX+24, menuY+28, "the hero for a"s);
         console.print(menuX+24, menuY+29, "short while"s);
      } else if (monsterSpec == 3) {
         console.setDefaultForeground(TCODColor::red);
         console.print(menuX+24, menuY+5, "Halt"s);
         console.print(menuX+24, menuY+15, "Flee"s);
         console.print(menuX+24, menuY+25, "Sleep"s);
         console.setDefaultForeground(TCODColor::white);
         console.print(menuX+24, menuY+7, "Monster will pause"s);
         console.print(menuX+24, menuY+8, "for a while or"s);
         console.print(menuX+24, menuY+9, "until attacked"s);
         console.print(menuX+24, menuY+17, "Monster will run"s);
         console.print(menuX+24, menuY+18, "if it sees the"s);
         console.print(menuX+24, menuY+19, "hero"s);
         console.print(menuX+24, menuY+27, "Monster will be"s);
         console.print(menuX+24, menuY+28, "unable to act for"s);
         console.print(menuX+24, menuY+29, "a short time"s);
      }
      if (worldSpec == 0) {
         console.setDefaultForeground(TCODColor(156, 156, 156));
         console.print(menuX+45, menuY+5, "Clear"s);
         console.print(menuX+45, menuY+15, "Cloud"s);
         console.print(menuX+45, menuY+25, "Trap"s);
         console.setDefaultForeground(TCODColor::white);

         console.print(menuX+45, menuY+7, "Clears walls and"s);
         console.print(menuX+45, menuY+8, "traps adjacent to"s);
         console.print(menuX+45, menuY+9, "the player"s);
         console.print(menuX+45, menuY+17, "Creates a cloud of"s);
         console.print(menuX+45, menuY+18, "smoke that cannot"s);
         console.print(menuX+45, menuY+19, "be seen through"s);
         console.print(menuX+45, menuY+27, "Creates a trap in"s);
         console.print(menuX+45, menuY+28, "an empty space"s);
      } else if (worldSpec == 1) {
         console.setDefaultForeground(TCODColor(156, 156, 156));
         console.print(menuX+45, menuY+5, "Clear"s);
         console.print(menuX+45, menuY+15, "Blow"s);
         console.print(menuX+45, menuY+25, "Illusion"s);
         console.setDefaultForeground(TCODColor::white);

         console.print(menuX+45, menuY+7, "Clears walls and"s);
         console.print(menuX+45, menuY+8, "traps adjacent to"s);
         console.print(menuX+45, menuY+9, "the player"s);
         console.print(menuX+45, menuY+17, "Pushes creatures"s);
         console.print(menuX+45, menuY+18, "and traps away"s);
         console.print(menuX+45, menuY+19, "from around you"s);
         console.print(menuX+45, menuY+27, "Makes an illusion"s);
         console.print(menuX+45, menuY+28, "that grabs the"s);
         console.print(menuX+45, menuY+29, "hero's attention"s);
      } else if (worldSpec == 2) {
         console.setDefaultForeground(TCODColor(156, 156, 156));
         console.print(menuX+45, menuY+5, "Screen"s);
         console.print(menuX+45, menuY+15, "Cloud"s);
         console.print(menuX+45, menuY+25, "Field"s);
         console.setDefaultForeground(TCODColor::white);

         console.print(menuX+45, menuY+7, "Creates a cloud"s);
         console.print(menuX+45, menuY+8, "screen that can"s);
         console.print(menuX+45, menuY+9, "dissolve walls"s);
         console.print(menuX+45, menuY+17, "Creates a cloud of"s);
         console.print(menuX+45, menuY+18, "smoke that cannot"s);
         console.print(menuX+45, menuY+19, "be seen through"s);
         console.print(menuX+45, menuY+27, "Creates a magical"s);
         console.print(menuX+45, menuY+28, "wall that cannot"s);
         console.print(menuX+45, menuY+29, "be moved through"s);
      } else if (worldSpec == 3) {
         console.setDefaultForeground(TCODColor(156, 156, 156));
         console.print(menuX+45, menuY+5, "Tunnel"s);
         console.print(menuX+45, menuY+15, "Minefield"s);
         console.print(menuX+45, menuY+25, "Trap"s);
         console.setDefaultForeground(TCODColor::white);

         console.print(menuX+45, menuY+7, "Digs a short"s);
         console.print(menuX+45, menuY+8, "corridor in a"s);
         console.print(menuX+45, menuY+9, "chosen direction"s);
         console.print(menuX+45, menuY+17, "Places some traps"s);
         console.print(menuX+45, menuY+18, "randomly around"s);
         console.print(menuX+45, menuY+19, "the player"s);
         console.print(menuX+45, menuY+27, "Creates a trap in"s);
         console.print(menuX+45, menuY+28, "an empty space"s);
      }
      console.setDefaultBackground(TCODColor::black);
      // Flush the main console
      console.flush();

      // Get keyboard input
      TCOD_key_t key = getKeyPress();
      spellChar = key.c;
   }

   // DETERMINE THE SPELL TO CAST BASED ON THE KEY AND SPELL SPECIALISATION
   bool spellCast = false;
   switch (spellChar) {
      case 'q':
         if (heroMana >= manaBlipSize) {
            spellCast = castSpell(spellLists[0][heroSpec][0]);
            if (spellCast) heroMana -= manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'a':
         if (heroMana >= 3*manaBlipSize) {
            spellCast = castSpell(spellLists[0][heroSpec][1]);
            if (spellCast) heroMana -= 3*manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'z':
         if (heroMana >= 5*manaBlipSize) {
            spellCast = castSpell(spellLists[0][heroSpec][2]);
            if (spellCast) heroMana -= 5*manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'w':
         if (monsterMana >= manaBlipSize) {
            spellCast = castSpell(spellLists[1][monsterSpec][0]);
            if (spellCast) monsterMana -= manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 's':
         if (monsterMana >= 3*manaBlipSize) {
            spellCast = castSpell(spellLists[1][monsterSpec][1]);
            if (spellCast) monsterMana -= 3*manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'x':
         if (monsterMana >= 5*manaBlipSize) {
            spellCast = castSpell(spellLists[1][monsterSpec][2]);
            if (spellCast) monsterMana -= 5*manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'e':
         if (worldMana >= manaBlipSize) {
            spellCast = castSpell(spellLists[2][worldSpec][0]);
            if (spellCast) worldMana -= manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'd':
         if (worldMana >= 3*manaBlipSize) {
            spellCast = castSpell(spellLists[2][worldSpec][1]);
            if (spellCast) worldMana -= 3*manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'c':
         if (worldMana >= 5*manaBlipSize) {
            spellCast = castSpell(spellLists[2][worldSpec][2]);
            if (spellCast) worldMana -= 5*manaBlipSize;
         } else {
            addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      default:
         break;
   }
   // CAST THE CHOSEN SPELL
   return spellCast;
}

bool castSpell(Spell chosenSpell) {
   int spellCast = false;
   int direction;
   bool itemDropped = false;
   bool minePlaced = false;
   switch (chosenSpell) {
      case Spell::PACIFISM:
         if (level < 10) {
            if (hero.inSpellRadius()) {
               if (hero.dead) {
                  addMessage("The hero is dead!", MessageType::SPELL);
               } else {
                  hero.pacifismTimer = hero.items.contains(Item::magicResist)
                     ? (PACIFISM_TIME/2)
                     : PACIFISM_TIME;
                  hero.target = nullptr;
                  addMessage("The hero appears calmer!", MessageType::SPELL);
                  hero.computePath();
               }
               spellCast = true;
            } else {
               addMessage("You are too far from the hero!", MessageType::SPELL);
            }
         } else {
            addMessage("The hero is too enCondition::RAGED to be pacified!", MessageType::SPELL);
         }
         break;
      case Spell::SPEED:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.hasteTimer = hero.items.contains(Item::magicResist)
                  ? (SPEED_TIME/2)
                  : SPEED_TIME;
               addMessage("The hero becomes a blur!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::HEAL:
         if (hero.inSpellRadius()) {
            if (hero.dead){
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               bool gained = false;
               gained = hero.gainHealth(hero.items.contains(Item::magicResist) ? 1 : 3);
               if (gained) {
                  addMessage("The hero looks healthier!", MessageType::SPELL);
               } else {
                  addMessage("The spell has no effect!", MessageType::SPELL);
               }
               spellCast = true;
            }
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
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
         for (int i = playerX-1; i <= playerX+1; i++) {
            for (int j = playerY-1; j <= playerY+1; j++) {
               if (i>=0 && j>=0 && i<MAP_WIDTH && j <MAP_HEIGHT) {
                  if (map[i][j] == WALL || map[i][j] == TRAP) {
                     map[i][j] = BLANK;
                     mapModel->setProperties(i, j, true, true);
                     if (hero.target == nullptr) {
                        hero.computePath();
                     }
                  }
               }
            }
         }
         addMessage("The area around you clears!", MessageType::SPELL);
         spellCast = true;
         break;
      case Spell::CLOUD:
         for (int i = playerX-2; i <= playerX+2; i++) {
            for (int j = playerY-2; j <= playerY+2; j++) {
               int clouddist = abs(playerX-i)+abs(playerY-j);
               if ((clouddist < 4) && i>=0 && i<MAP_WIDTH && j>=0 && j <MAP_HEIGHT) {
                  if (map[i][j] != WALL) {
                     int newCloudLevel = CLOUD_TIME*(8-clouddist)/8;
                     if (newCloudLevel > cloud[i][j]) {
                        cloud[i][j] = newCloudLevel;
                     }
                     mapModel->setProperties(i, j, false, true);
                  }
               }
            }
         }
         addMessage("A thick cloud of smoke appears around you!", MessageType::SPELL);
         spellCast = true;

         break;
      case Spell::MTRAP:
         direction = getDirection();
         if (direction != 0) {
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            int targetX = playerX + diffX;
            int targetY = playerY + diffY;
            if ((targetX >= 0) && (targetY >= 0) && (targetX < MAP_WIDTH) && (targetY < MAP_HEIGHT) && map[targetX][targetY] == BLANK) {
               addMessage("You create a trap in the ground", MessageType::SPELL);
               map[targetX][targetY] = TRAP;
            } else {
               addMessage("Without empty ground, the spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::MEDITATION:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.meditationTimer = hero.items.contains(Item::magicResist)
                  ? (MEDITATION_TIME / 2)
                  : MEDITATION_TIME;
               addMessage("The hero begins quiet introspection", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::CHARITY:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               // Check for each item the hero might have
               for (int i = 0; i < ITEM_COUNT; i++) {
                  const Item curItem = static_cast<Item>(i);
                  if (hero.items.contains(curItem)) {
                     // Take off the item
                     hero.items.erase(curItem);
                     addMessage("The hero takes off "+ITEM_NAME.at(curItem), MessageType::SPELL);
                     itemDropped = true;
                     // Specific effects of taking off each item
                     switch(curItem) {
                        case Item::monsterHelm:
                           for (auto& monster : monsterList) {
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
                  addMessage("Hero: " + Hero::heroCharity[randGen->getInt(0, 4)], MessageType::HERO);
                  hero.gainHealth(10);
                  spellCast = true;
               } else {
                  addMessage("The hero is not wearing any treasure!", MessageType::SPELL);
               }
            }
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::SLOW:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               if (hero.slow) {
                  hero.slow = false;
                  addMessage("The hero's actions speed up!", MessageType::SPELL);
               } else {
                  hero.slow = true;
                  addMessage("The hero's actions slow down!", MessageType::SPELL);
               }
            }
            spellCast = true;
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::SHIELD:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.shieldTimer = hero.items.contains(Item::magicResist)
                  ? (SHIELD_TIME / 2)
                  : SHIELD_TIME;
               addMessage("A transclucent shield surrounds the hero!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::REGENERATE:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.regenTimer = hero.items.contains(Item::magicResist)
                  ? (REGEN_TIME / 2)
                  : REGEN_TIME;
               addMessage("You project some healing magic onto the hero!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::BLINK:
         if (hero.inSpellRadius()) {
            if (hero.dead) {
               addMessage("The hero is dead!", MessageType::SPELL);
            } else {
               hero.blinking = true;
               const auto limit = hero.items.contains(Item::magicResist)
                  ? (BLINK_MOVES / 2)
                  : BLINK_MOVES;
               for (int i = 0; i < limit; ++i) {
                  hero.move();
                  drawScreen();
                  TCODSystem::sleepMilli(10);
               }
               hero.blinking = false;
               addMessage("The hero's actions are almost instant!", MessageType::SPELL);
            }
            spellCast = true;
         } else {
            addMessage("You are too far from the hero!", MessageType::SPELL);
         }
         break;
      case Spell::TUNNEL:
         direction = getDirection();
         if (direction != 0) {
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            for (int i = 1; i <= 3; i++) {
               int stepX = diffX*i+playerX;
               int stepY = diffY*i+playerY;
               if (stepX>=0 && stepY>=0 && stepX<MAP_WIDTH && stepY <MAP_HEIGHT) {
                  if (map[stepX][stepY] == WALL || map[stepX][stepY] == TRAP) {
                     map[stepX][stepY] = BLANK;
                     mapModel->setProperties(stepX, stepY, true, true);
                  }
               }
            }
            addMessage("A path is cleared for you!", MessageType::SPELL);
            if (hero.target == nullptr) {
               hero.computePath();
            }
            spellCast = true;
         }
         break;
      case Spell::MINEFIELD:
         for (int i = 0; i < 5; i++) {
            int tempx = playerX+randGen->getInt(-2, 2);
            int tempy = playerY+randGen->getInt(-2, 2);
            if (tempx >= 0 && tempy >= 0 && tempx < MAP_WIDTH && tempy < MAP_HEIGHT) {
               if (map[tempx][tempy] == BLANK) {
                  map[tempx][tempy] = TRAP;
                  minePlaced = true;
               }
            }
         }
         if (minePlaced) {
            addMessage("Traps materialise in the surrounding area!", MessageType::SPELL);
         } else {
            addMessage("You cast the spell, but no traps materialise!", MessageType::SPELL);
         }
         spellCast = true;
         break;
      case Spell::MAIM:
         direction = getDirection();
         if (direction != 0) {
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            int targetX = playerX + diffX;
            int targetY = playerY + diffY;
            if (map[targetX][targetY] == MONSTER) {
               Monster* target = findMonster(targetX, targetY);
               addMessage("The " + target->name + " looks pained!", MessageType::SPELL);
               target->maimed = true;
            } else {
               addMessage("The spell fizzles in empty air", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::CRIPPLE:
         direction = getDirection();
         if (direction != 0) {
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            int targetX = playerX + diffX;
            int targetY = playerY + diffY;
            if (map[targetX][targetY] == MONSTER) {
               Monster* target = findMonster(targetX, targetY);
               addMessage("A terrible snap comes from inside the " + target->name + "!", MessageType::SPELL);
               target->health = (target->health+1)/2;
            } else {
               addMessage("The spell fizzles in empty air", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::MILLUSION:
         direction = getDirection();
         if (direction != 0) {
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            int targetX = playerX + diffX;
            int targetY = playerY + diffY;
            if (targetX >= 0 && targetY >= 0 && targetX < MAP_WIDTH && targetY < MAP_HEIGHT && map[targetX][targetY] == BLANK ) {
               if (illusionX != -1) {
                  map[illusionX][illusionY] = BLANK;
               }
               illusionX = targetX; illusionY = targetY;
               map[targetX][targetY] = ILLUSION;
               drawScreen();
            } else {
               addMessage("Without empty ground, the spell fizzles", MessageType::SPELL);
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
         direction = getDirection();
         if (direction != 0) {
            bool screenMade = false;
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            for (int i = -1; i < 2; i++) {
               for (int j = -1; j < 2; j++) {
                  int diff = abs(diffX-i)+abs(diffY-j);
                  if ((i != 0 || j != 0) && diff < 2) {
                     int curX = playerX+i;
                     int curY = playerY+j;
                     if (curX >= 0 && curY >= 0 && curX < MAP_WIDTH && curY < MAP_HEIGHT) {
                        if (map[curX][curY] == TRAP || map[curX][curY] == WALL) {
                           map[curX][curY] = BLANK;
                        }
                        cloud[curX][curY] = (diff == 0?CLOUD_TIME:CLOUD_TIME*7/8);
                        mapModel->setProperties(curX, curY, false, true);
                        screenMade = true;
                     }
                  }
               }
            }
            if (screenMade) {
               addMessage("A wall of cloud appears before you", MessageType::SPELL);
               hero.computePath();
            } else {
               addMessage("The spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::MFIELD:
         direction = getDirection();
         if (direction != 0) {
            bool fieldMade = false;
            int diffX = ((direction-1)%3)-1;
            int diffY = 1-((direction-1)/3);
            int curX = 0;
            int curY = 0;
            if (diffX == 0 || diffY == 0) {
               for (int i = -2; i <= 2; i++) {
                  curX = playerX+diffX+(i*(1-abs(diffX)));
                  curY = playerY+diffY+(i*(1-abs(diffY)));
                  if (map[curX][curY] == BLANK) {
                     field[curX][curY] = FIELD_TIME-randGen->getInt(1, 3);
                     map[curX][curY] = FIELD;
                     mapModel->setProperties(curX, curY, false, false);
                     fieldMade = true;
                  }
               }
               for (int i = -1; i <= 1; i++) {
                  curX = playerX+diffX*2+(i*(1-abs(diffX)));
                  curY = playerY+diffY*2+(i*(1-abs(diffY)));
                  if (map[curX][curY] == BLANK) {
                     field[curX][curY] = FIELD_TIME-randGen->getInt(1, 3);
                     map[curX][curY] = FIELD;
                     mapModel->setProperties(curX, curY, false, false);
                     fieldMade = true;
                  }
               }
            } else {
               for (int i = -1; i <= 1; i++) {
                  curX = playerX+diffX-(i*diffX);
                  curY = playerY+diffY+(i*diffY);
                  if (map[curX][curY] == BLANK) {
                     field[curX][curY] = FIELD_TIME-randGen->getInt(1, 3);
                     map[curX][curY] = FIELD;
                     mapModel->setProperties(curX, curY, false, false);
                     fieldMade = true;
                  }
               }
               for (int i = 0; i <= 3; i++) {
                  curX = (int)(playerX+(float)diffX/2-((i-1.5)*diffX));
                  curY = (int)(playerY+(float)diffY/2+((i-1.5)*diffY));
                  if (map[curX][curY] == BLANK) {
                     field[curX][curY] = FIELD_TIME-randGen->getInt(1, 3);
                     map[curX][curY] = FIELD;
                     mapModel->setProperties(curX, curY, false, false);
                     fieldMade = true;
                  }
               }
            }
            if (fieldMade) {
               hero.computePath();
               addMessage("An impassable field appears before you!", MessageType::SPELL);
            } else {
               addMessage("The spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::BLOW:
         spellCast = true;
         addMessage("Magical force bursts out from you!", MessageType::SPELL);
         for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
               if (i != 0 || j != 0) {
                  int step1x = playerX+i;
                  int step1y = playerY+j;
                  int step2x = playerX+2*i;
                  int step2y = playerY+2*j;
                  int step3x = playerX+3*i;
                  int step3y = playerY+3*j;
                  int tempx = step1x, tempy = step1y;
                  bool trapSprung = false;
                  if (step1x >= 0 && step1y >= 0 && step2x>= 0 && step2y >= 0 && step1x < MAP_WIDTH && step1y < MAP_HEIGHT && step2x < MAP_WIDTH && step2y < MAP_HEIGHT) {
                     if (map[step1x][step1y] == MONSTER) {
                        if (map[step2x][step2y] == BLANK) {
                           if (step3x>= 0 && step3x < MAP_WIDTH && step3y >= 0 && step3y < MAP_HEIGHT && (map[step3x][step3y] == BLANK || map[step3x][step3y] == TRAP)) {
                              tempx = step3x;
                              tempy = step3y;
                              if (map[step3x][step3y] == TRAP) {
                                 trapSprung = true;
                              }
                           } else {
                              tempx = step2x;
                              tempy = step2y;
                           }
                        } else if (map[step2x][step2y] == TRAP) {
                           trapSprung = true;
                           tempx = step2x;
                           tempy = step2y;
                        }
                        Monster* target = findMonster(step1x, step1y);
                        target->timer = target->wait;
                        map[step1x][step1y] = BLANK;
                        map[tempx][tempy] = MONSTER;
                        target->x = tempx; target->y = tempy;
                        if (trapSprung) {
                           addMessage("The " + target->name+ " is blown into the trap!", MessageType::SPELL);
                           hitMonster(target->x, target->y, 4);
                        }
                     } else if (map[step1x][step1y] == HERO) {
                        if (map[step2x][step2y] == BLANK) {
                           if (step3x>= 0 && step3x < MAP_WIDTH && step3y >= 0 && step3y < MAP_HEIGHT && (map[step3x][step3y] == BLANK || map[step3x][step3y] == TRAP)) {
                              tempx = step3x;
                              tempy = step3y;
                              if (map[step3x][step3y] == TRAP) {
                                 trapSprung = true;
                              }
                           } else {
                              tempx = step2x;
                              tempy = step2y;
                           }
                        } else if (map[step2x][step2y] == TRAP) {
                           trapSprung = true;
                           tempx = step2x;
                           tempy = step2y;
                        }
                        map[step1x][step1y] = BLANK;
                        map[tempx][tempy] = HERO;
                        hero.x = tempx; hero.y = tempy;
                        hero.timer = hero.wait;
                        if (trapSprung) {
                           if (!hero.dead) {
                              addMessage("The hero is blown into the trap!", MessageType::SPELL);
                              hero.health -= 4;
                              map[step1x][step1y] = BLANK;
                              map[tempx][tempy] = HERO;
                              hero.x = tempx; hero.y = tempy;
                              if (hero.health <= 0) {
                                 hero.dead = true;
                                 addMessage("The hero has died!", MessageType::IMPORTANT);
                                 drawScreen();
                              } else {
                                 addMessage("Hero: " + Hero::heroBlow[randGen->getInt(0, 4)], MessageType::HERO);
                              }
                           } else {
                              map[step1x][step1y] = BLANK;
                              map[tempx][tempy] = HERO;
                              hero.x = tempx; hero.y = tempy;
                              addMessage("The hero's corpse is blown into the trap!", MessageType::SPELL);
                           }
                        } else {
                           if (!hero.dead) {
                              addMessage("Hero: " + Hero::heroBlow[randGen->getInt(0, 4)], MessageType::HERO);
                           }
                        }
                     } else if (map[step1x][step1y] == TRAP && (map[step2x][step2y] == BLANK || map[step2x][step2y] == HERO || map[step2x][step2y] == MONSTER)) {
                        if (map[step2x][step2y] == BLANK) {
                           if (step3x>= 0 && step3x < MAP_WIDTH && step3y >= 0 && step3y < MAP_HEIGHT && (map[step3x][step3y] == BLANK || map[step3x][step3y] == HERO || map[step3x][step3y] == MONSTER)) {
                              tempx = step3x;
                              tempy = step3y;
                           } else {
                              tempx = step2x;
                              tempy = step2y;
                           }
                        } else {
                           tempx = step2x;
                           tempy = step2y;
                        }
                        map[step1x][step1y] = BLANK;
                        if (map[tempx][tempy] == BLANK) {
                           map[tempx][tempy] = TRAP;
                        } else if (map[tempx][tempy] == HERO) {
                           if (!hero.dead) {
                              addMessage("You blow a trap into the hero!", MessageType::SPELL);
                              hero.health -= 4;
                              if (hero.health <= 0) {
                                 hero.dead = true;
                                 addMessage("The hero has died!", MessageType::IMPORTANT);
                                 drawScreen();
                              } else {
                                 addMessage("Hero: " + Hero::heroBlow[randGen->getInt(0, 4)], MessageType::HERO);
                              }
                           } else {
                              addMessage("You blow a trap into the hero's corpse!", MessageType::SPELL);
                           }
                        } else if (map[tempx][tempy] == MONSTER) {
                           Monster* target = findMonster(tempx, tempy);
                           addMessage("You blow a trap into the " + target->name+ "!", MessageType::SPELL);
                           hitMonster(target->x, target->y, 4);
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

   drawScreen();
   return spellCast;
}

int getDirection() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   int result = 0;
   drawScreen();

   // Display a screen to the user
   console.setDefaultForeground(TCODColor::yellow);
   console.setDefaultBackground(TCODColor(128, 64, 0));
   for (int i = 21; i <= 59; i++) {
      for (int j = BOTTOM+4; j <= BOTTOM+6; j++) {
         console.putChar(i, j, ' ', TCOD_BKGND_SET);
      }
   }
   for (int i = 21; i <= 59; i++) {
      console.putChar(i, BOTTOM+3, 196, TCOD_BKGND_SET);
      console.putChar(i, BOTTOM+7, 196, TCOD_BKGND_SET);
   }
   for (int j = BOTTOM+3; j <= BOTTOM+7; j++) {
      console.putChar(20, j, 179, TCOD_BKGND_SET);
      console.putChar(60, j, 179, TCOD_BKGND_SET);
   }
   console.putChar(20, BOTTOM+3, 218, TCOD_BKGND_SET);
   console.putChar(60, BOTTOM+3, 191, TCOD_BKGND_SET);
   console.putChar(20, BOTTOM+7, 192, TCOD_BKGND_SET);
   console.putChar(60, BOTTOM+7, 217, TCOD_BKGND_SET);
   console.setDefaultBackground(TCODColor::black);
   console.setDefaultForeground(TCODColor::white);
   console.print(28, BOTTOM+5, "Please choose a direction"s);
   console.flush();

   // Get user response
   TCOD_key_t key = getKeyPress();
   if (key.vk == TCODK_UP || key.vk == TCODK_KP8 || key.c == 'k') {
      result = 8;
   } else if (key.vk == TCODK_DOWN || key.vk == TCODK_KP2 || key.c == 'j') {
      result = 2;
   } else if (key.vk == TCODK_LEFT || key.vk == TCODK_KP4 || key.c == 'h') {
      result = 4;
   } else if (key.vk == TCODK_RIGHT || key.vk == TCODK_KP6 || key.c == 'l') {
      result = 6;
   } else if (key.vk == TCODK_KP7 || key.c == 'y') {
      result = 7;
   } else if (key.vk == TCODK_KP9 || key.c == 'u') {
      result = 9;
   } else if (key.vk == TCODK_KP3 || key.c == 'n') {
      result = 3;
   } else if (key.vk == TCODK_KP1 || key.c == 'b') {
      result = 1;
   }   

   return result;
}
void generateMonsters(int level, int amount) {
   // Clear all existing monsters
   monsterList.clear();
   int tempx = 0, tempy = 0;
   for (int a = 0; a < 4; a++) {
      for (int i = 0; i < amount; i++) {
         tempx = 0; tempy = 0;
         while (map[tempx][tempy] != BLANK) {
            if (a == 0) {
               tempx = randGen->getInt(0, (MAP_WIDTH-1)/2);
               tempy = randGen->getInt(0, (MAP_HEIGHT-1)/2);
            } else if (a == 1) {
               tempx = randGen->getInt((MAP_WIDTH-1)/2, MAP_WIDTH-1);
               tempy = randGen->getInt(0, (MAP_HEIGHT-1)/2);
            } else if (a == 2) {
               tempx = randGen->getInt(0, (MAP_WIDTH-1)/2);
               tempy = randGen->getInt((MAP_HEIGHT-1)/2, MAP_HEIGHT-1);
            } else {
               tempx = randGen->getInt((MAP_WIDTH-1)/2, MAP_WIDTH-1);
               tempy = randGen->getInt((MAP_HEIGHT-1)/2, MAP_HEIGHT-1);
            }
         }
         int randomMonster = randGen->getInt(level-1, level+3);
         addSpecifiedMonster(tempx, tempy, randomMonster, false);
      }
   }
}

void addSpecifiedMonster(int tempx, int tempy, int number, bool portalSpawned) {
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

void generateEndBoss() {
   int tempx = 0, tempy = 0;
   while (map[tempx][tempy] != BLANK || dist(tempx, tempy, hero.x, hero.y) < 30) {
      tempx = randGen->getInt(0, MAP_WIDTH-1);
      tempy = randGen->getInt(0, MAP_HEIGHT-1);
   }
   int boss = randGen->getInt(0, 2);
   if (boss == 0) {
      addMonster("Master Summoner", '*', tempx, tempy, 20, 2, false, " ", 0.0f, 15, false); 
      addMessage("Master Summoner: You have come to your grave! I will bury you in monsters!", MessageType::VILLAIN);
   } else if (boss == 1) {
      addMonster("Noble Hero", '@', tempx, tempy, 30, 3, false, " ", 0.0f, 2, false); 
      addMessage("Noble Hero: I have made it to the treasure first, my boastful rival.", MessageType::VILLAIN);
      addMessage("Noble Hero: I wish you no harm, do not force me to defend myself.", MessageType::VILLAIN);
   } else {
      addMonster("Evil Mage", 'M', tempx, tempy, 15, 4, true, "shoots lightning", 20.0f, 5, false); 
      addMessage("Evil Mage: How did you make it this far?! DIE!!!", MessageType::VILLAIN);
   }
   hero.stairsx = tempx;
   hero.stairsy = tempy;
}

void showVictoryScreen() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   // Blit the spell menu on the main console
   TCODConsole::blit(spellMenu, 0, 0, 66, 35, &console, menuX, menuY);

   // Draw the borders
   console.setDefaultForeground(TCODColor(180, 100, 0));
   for (int i = menuX+2; i <= menuX + 63; i++) {
      console.putChar(i, menuY+1, 205, TCOD_BKGND_NONE);
      console.putChar(i, menuY+33, 205, TCOD_BKGND_NONE);
   }
   for (int i = menuX+5; i <= menuX + 60; i++) {
      console.putChar(i, menuY+4, 205, TCOD_BKGND_NONE);
      console.putChar(i, menuY+30, 205, TCOD_BKGND_NONE);
   }
   for (int j = menuY+2; j <= menuY + 32; j++) {
      console.putChar(menuX+1, j, 186, TCOD_BKGND_NONE);
      console.putChar(menuX+64, j, 186, TCOD_BKGND_NONE);
   }
   for (int j = menuY+5; j <= menuY + 29; j++) {
      console.putChar(menuX+4, j, 186, TCOD_BKGND_NONE);
      console.putChar(menuX+61, j, 186, TCOD_BKGND_NONE);
   }
   console.setDefaultBackground(TCODColor(210, 96, 0));
   for (int i = menuX+3; i<=menuX+63; i++) {
      console.putChar(i, menuY+2, ' ', TCOD_BKGND_SET);
   }
   for (int i = menuX+4; i<=menuX+62; i++) {
      console.putChar(i, menuY+3, ' ', TCOD_BKGND_SET);
   }
   for (int j = menuY+2; j <= menuY + 32; j++) {
      console.putChar(menuX+2, j, ' ', TCOD_BKGND_SET);
   }
   for (int j = menuY+3; j <= menuY + 31; j++) {
      console.putChar(menuX+3, j, ' ', TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor(84, 40, 0));
   for (int i = menuX+3; i<=menuX+63; i++) {
      console.putChar(i, menuY+32, ' ', TCOD_BKGND_SET);
   }
   for (int i = menuX+4; i<=menuX+62; i++) {
      console.putChar(i, menuY+31, ' ', TCOD_BKGND_SET);
   }
   for (int j = menuY+3; j <= menuY + 32; j++) {
      console.putChar(menuX+63, j, ' ', TCOD_BKGND_SET);
   }
   for (int j = menuY+4; j <= menuY + 31; j++) {
      console.putChar(menuX+62, j, ' ', TCOD_BKGND_SET);
   }

   console.putChar(menuX+4, menuY+4, 201, TCOD_BKGND_NONE);
   console.putChar(menuX+61, menuY+4, 187, TCOD_BKGND_NONE);
   console.putChar(menuX+4, menuY+30, 200, TCOD_BKGND_NONE);
   console.putChar(menuX+61, menuY+30, 188, TCOD_BKGND_NONE);
   console.putChar(menuX+1, menuY+1, 201, TCOD_BKGND_NONE);
   console.putChar(menuX+64, menuY+1, 187, TCOD_BKGND_NONE);
   console.putChar(menuX+1, menuY+33, 200, TCOD_BKGND_NONE);
   console.putChar(menuX+64, menuY+33, 188, TCOD_BKGND_NONE);
   console.setDefaultForeground(TCODColor::white);
   console.setAlignment(TCOD_CENTER);
   console.print(menuX+MAP_WIDTH/2-3, menuY+7, "Congratulations!"s);
   console.setAlignment(TCOD_LEFT);
   console.print(menuX+MAP_WIDTH/2-28, menuY+10, "The hero rejoices over his victory of the dungeon,"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+11, "still completely unaware of your influence. You"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+12, "continue to safeguard the hero until he leaves the"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+13, "dungeon. With the magical artifacts gone and the"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+14, "monsters decimated, the evil aura of the dungeon"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+15, "diminishes. Soon the grass, vines and forest"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+16, "creatures will begin to encroach into the dungeon."s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+19, "The next greatest annoyance to the forest begins"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+20, "to grab your attention. The town, infested with"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+21, "rampaging heroes and greedy lumberjacks, must be"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+22, "the next target. It will take a few weeks before"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+23, "you can recruit enough wolves and bears, but you"s);
   console.print(menuX+MAP_WIDTH/2-28, menuY+24, "know you will have no problem protecting them."s);
   console.print(menuX+MAP_WIDTH/2-7, menuY+27, "THE END"s);

   console.setDefaultForeground(TCODColor::yellow);
   console.setDefaultBackground(TCODColor(78, 78, 78));
   {
      WithBackgroundSet set(console);
      console.print(menuX+40, menuY+34, " Press ESC to quit "s);
   }
   console.flush();
   for (TCOD_key_t key = getKeyPress(); key.vk != TCODK_ESCAPE; key = getKeyPress()) { }
}

bool applyMonsterCondition(Condition curCondition, bool append) {
   bool spellCast = false;
   int direction = getDirection();
   if (direction != 0) {
      int diffX = ((direction-1)%3)-1;
      int diffY = 1-((direction-1)/3);
      int targetX = playerX + diffX;
      int targetY = playerY + diffY;
      if (map[targetX][targetY] == MONSTER) {
         Monster* target = findMonster(targetX, targetY);
         const auto& text = CONDITION_START.at(curCondition);
         addMessage(text.first + target->name + text.second, MessageType::SPELL);
         int& timer = target->conditionTimers.at(curCondition);
         const int amount = CONDITION_TIMES.at(curCondition);
         if (append) {
            timer += amount;
         } else {
            timer = amount;
         }
      } else {
         addMessage("The spell fizzles in empty air", MessageType::SPELL);
      }
      spellCast = true;
   }
   return spellCast;
}

TCOD_key_t getKeyPress() {
   TCOD_key_t key;
   SDL_Event event;

   for (; event.type != SDL_KEYDOWN; SDL_WaitEvent(&event)) {}
   tcod::sdl2::process_event(event, key);  // Convert a SDL key to a libtcod key event, to help port older code.

   return key;
}