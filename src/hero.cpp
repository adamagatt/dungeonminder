#include "hero.hpp"
#include "utils.hpp"


using namespace std::string_literals;

TCODRandom * randGen = TCODRandom::getInstance();

bool Hero::checkWin() const {
   bool result = false;
   if (dest1x == -1 && dest2x == -1 && x == stairsx && y == stairsy && !dead) {
      result = true;
   }
   return result;
}

void Hero::giveItem() {
   const int itemIdx = randGen->getInt(0, ITEM_COUNT-1);
   const Item itemFound = static_cast<Item>(itemIdx);
   // Display messages
   message("The hero finds " + ITEM_NAME.at(itemFound) + "!", MessageType::IMPORTANT);
   message("Hero: " + heroItem[randGen->getInt(0, 9)], MessageType::HERO);
   // Apply immediate effects of any item
   switch (itemFound) {
      case Item::scrollEarthquake:
         for (int i =0; i < MAP_WIDTH; i++) {
            for (int j = 0; j < MAP_HEIGHT; j++) {
               if (map[i][j] == BLANK && randGen->getInt(0, 29) == 0) {
                  map[i][j] = WALL;
                  mapModel->setProperties(i, j, false, false);
               } else if (map[i][j] == WALL && randGen->getInt(0, 10) == 0) {
                  map[i][j] = BLANK;
                  mapModel->setProperties(i, j, true, true);
               }
            }
         }
         // Ensure the hero can get to all of his goals
         for (int j = 0; j < 2; j++) {
            int srcX, srcY, destX, destY;
            if (j == 0) {
               srcX = dest1x;
               srcY = dest1y;
               destX = dest2x;
               destY = dest2y;
            } else {
               srcX = dest2x;
               srcY = dest2y;
               destX = stairsx;
               destY = stairsy;
            }
            path.compute(srcX, srcY, destX, destY);
            int pathX = -1, pathY = -1;
            for (int i = 0; i < path.size(); i++) {
               path.get(i, &pathX, &pathY);
               if (map[pathX][pathY] == WALL) {
                  map[pathX][pathY] = BLANK;
                  mapModel->setProperties(pathX, pathY, true, true);
               }

            }
         }
         computePath();
         break;
      case Item::scrollSeeInvisible:
         message("The hero is temporarily able to see you!", MessageType::SPELL);
         seeInvisibleTimer = 20;
         break;
      case Item::scrollSummonMonsters:
         summonMonsterTimer = 35;
         break;
      case Item::monsterHelm:
         for (auto& monster : monsterList) {
            monster.angry = true;
         }
         break;
      case Item::healthCap:
         if (health > 8) health = 8;
         break;
      case Item::rustedSword:
         damage = 4;
         break;
      default:
         break;
   }
   // Add the item to the hero's inventory (if applicable)
   if (itemFound != Item::scrollEarthquake && itemFound != Item::scrollSummonMonsters && itemFound != Item::scrollSeeInvisible) {
      items.insert(itemFound);
   }
}

bool Hero::gainHealth(int amount) {
   int gained = false;
   if (!(health == 8 && items.contains(Item::healthCap)) && !(health == 10 && !items.contains(Item::healthCap))) {
      health += amount;
      gained = true;
      if (health > 8 && items.contains(Item::healthCap)) {
         health = 8;
      } else if (health > 10) {
         health = 10;
      }
   }
   return gained;
}


void Hero::computePath() {
   if (dest1x != -1) {
      path.compute(x, y, dest1x, dest1y);
   } else if (dest2x != -1) {
      path.compute(x, y, dest2x, dest2y);
   } else {
      path.compute(x, y, stairsx, stairsy);
   }
   pathstep = 0;
}

bool Hero::move() {
   bool nextLevel = false;
   if (meditationTimer == 0) {
      // If the hero is ready to move
      if (timer == 0 || hasteTimer > 0) {
         nextLevel = checkWin();
         map[x][y] = BLANK;
         int diffx = 0, diffy = 0;
         // If the hero can see the player
         if (seeInvisibleTimer > 0 && Utils::dist(x, y, playerX, playerY) < 2 && mapModel->isInFov(x, y)) {
            diffx = x-playerX;
            diffy = y-playerY;
            message("Hero: " + heroScared[randGen->getInt(0, 4)], MessageType::HERO);
         } else if (illusionX != -1 && mapModel->isInFov(illusionX, illusionY)) {
            // If the hero sees the illusion, it takes priority
            int ptx = 0, pty = 0;
            bool reachable = path.compute(x, y, illusionX, illusionY); 
            if (reachable) {
               target = nullptr;
               path.get(0, &ptx, &pty);
               if (x > ptx) diffx = -1;
               if (x < ptx) diffx = 1;
               if (y > pty) diffy = -1;
               if (y < pty) diffy = 1;
            } else {
               diffx = randGen->getInt(-1, 1);
               diffy = randGen->getInt(-1, 1);
            }
         } else if (target == nullptr || pacifismTimer > 0) {
            // If the hero doesn't have a target, attempt to find one
            if (pacifismTimer == 0) {
               target = heroFindMonster();
            }
            if (target != nullptr) {
               message("Hero: " + heroFight[randGen->getInt(0, 9)], MessageType::HERO);
               pathstep = 0;
               path.compute(x, y, target->x, target->y); 
            } else {
               if (x == dest1x && y == dest1y) {
                  map[dest1x][dest1y-1] = CHEST_OPEN;
                  giveItem();
                  dest1x = -1; dest1y = -1;
               } else if (dest1x == -1 && x == dest2x && y == dest2y) {
                  map[dest2x][dest2y-1] = CHEST_OPEN;
                  giveItem();
                  dest2x = -1; dest2y = -1;
               } else {
                  // If the hero doesn't have a path, give him one
                  if (path.isEmpty()) {
                     computePath();
                  }
                  // Follow the hero's path
                  int ptx = 0, pty = 0;
                  if (pathstep < path.size()) {
                     path.get(pathstep, &ptx, &pty);
                     pathstep++;
                     if (x > ptx) diffx = -1;
                     if (x < ptx) diffx = 1;
                     if (y > pty) diffy = -1;
                     if (y < pty) diffy = 1;
                  } else {
                     computePath();
                  }
               }
            }
         } else {
            // If the hero had an existing target, move to attack
            int ptx = 0, pty = 0;
            bool reachable = path.compute(x, y, target->x, target->y); 
            if (reachable) {
               path.get(0, &ptx, &pty);
               if (x > ptx) diffx = -1;
               if (x < ptx) diffx = 1;
               if (y > pty) diffy = -1;
               if (y < pty) diffy = 1;
            } else {
               diffx = randGen->getInt(-1, 1);
               diffy = randGen->getInt(-1, 1);
            }
         }
         if (diffx != 0 && diffy != 0) {
            if (randGen->getInt(0, 1) == 0) {
               diffx = 0;
            } else {
               diffy = 0;
            }
         }
         if (map[x+diffx][y+diffy] == MONSTER) {
            if (pacifismTimer > 0) {
               mapModel->setProperties(x+diffx, y+diffy, true, false);
               computePath();
               mapModel->setProperties(x+diffx, y+diffy, true, true);
               message("Hero: "+heroBump[randGen->getInt(0, 4)], MessageType::HERO);
            } else {
               target = findMonster(x+diffx, y+diffy);
               sprintf(charBuffer, "%d", damage);
               message("The hero hits the " + target->name + " for " + charBuffer + " damage", MessageType::NORMAL);
               int selfDamage = 0;
               if (items.contains(Item::carelessGauntlets) && (target->health < damage)) {
                  selfDamage = damage - target->health;
               }
               hitMonster(x+diffx, y+diffy, damage);
               if (blinking) {
                  gainHealth(2);
               }
               if (target == nullptr) {
                  if (selfDamage > 1) {
                     sprintf(charBuffer, "%d", selfDamage/2);
                     message(blankString+"The hero suffers "+charBuffer+" damage from the effort!", MessageType::NORMAL);
                     health -= selfDamage;
                     if (health <= 0) {
                        dead = true;
                        message("The hero has died!", MessageType::IMPORTANT);
                        drawScreen();
                     }

                  }
                  if (!dead) {
                     message("Hero: "+heroKills[randGen->getInt(0, 9)], MessageType::HERO);
                     computePath();
                  }
               }
            }
         } else if (map[x+diffx][y+diffy] == FIELD) {
            message("The hero is blocked by the forcefield", MessageType::SPELL);
            computePath();
         } else if (map[x+diffx][y+diffy] == WALL) {
            computePath();
         } else if (map[x+diffx][y+diffy] == TRAP) {
            message("The hero falls into the trap!", MessageType::NORMAL);
            health -= 4;
            map[x+diffx][y+diffx] = BLANK;
            x += diffx;
            y += diffy;
            if (health <= 0) {
               dead = true;
               message("The hero has died!", MessageType::IMPORTANT);
               drawScreen();
            }
         } else if (map[x+diffx][y+diffy] == ILLUSION) {
            map[x+diffx][y+diffy] = BLANK;
            illusionX = -1; illusionY = -1;
            message("The hero disrupts the illusion", MessageType::SPELL);
            message("Hero: " + heroIllusion[randGen->getInt(0, 4)], MessageType::HERO);
         } else if (map[x+diffx][y+diffy] == BLANK) {
            x += diffx;
            y += diffy;
         } else if (map[x+diffx][y+diffy] == PLAYER) {
            std::swap(playerX, x);
            std::swap(playerY, y);
            message("The hero passes through you", MessageType::NORMAL);
            map[playerX][playerY] = PLAYER;
         }
         map[x][y] = HERO;
         if ((items.contains(Item::slowBoots) && randGen->getInt(1, 2) == 1) || slow) {
            timer = wait*3/2;
         } else {
            timer = wait;
         }
         if (hasteTimer != 0) {
            if (hasteTimer > 0) hasteTimer--;
            if (hasteTimer == 0) {
               message("The hero resumes normal speed", MessageType::SPELL);
            }
         }
         if (shieldTimer != 0) { 
            shieldTimer--;
            if (shieldTimer == 0) {
               message("The hero's magical shield fades", MessageType::SPELL);
               drawScreen();
            }
         }
         if (seeInvisibleTimer > 0) {
            seeInvisibleTimer--;
            if (seeInvisibleTimer == 0) {
               message("The hero is no longer able to see you", MessageType::SPELL);
            }
         }
         if (regenTimer != 0) {
            regenTimer--;
            if (regenTimer%2 == 0) {
               gainHealth(1);
            }
            if (regenTimer == 0) {
               message("Your healing magic is exhausted", MessageType::SPELL);
            }
         }
      }
      if (pacifismTimer > 0) {
         pacifismTimer--;
         if (pacifismTimer == 0) {
            message("The hero begins looking for enemies", MessageType::SPELL);
         }
      }
      timer -= 1;
   } else {
      meditationTimer--;
      if (meditationTimer == 0) {
         message("His mind cleared, the hero picks up his equipment", MessageType::SPELL);
      }
   }
   // Effects of the summon monster scroll
   if (summonMonsterTimer > 0) {
      summonMonsterTimer--;
      if (monsterList.size() < MAX_MONSTERS && summonMonsterTimer%15 == 1) {
         int lx = 0, ly = 0;
         while (map[lx][ly] != BLANK || Utils::dist(x, y, lx, ly) > 10) {
            lx = randGen->getInt(0, MAP_WIDTH-1);
            ly = randGen->getInt(0, MAP_HEIGHT-1);
         }
         int randomMonster = randGen->getInt((level/2+1)-1, (level/2+1)+3);
         addSpecifiedMonster(lx, ly, randomMonster, true);
      }
   }
   // Effects of the belt of trap attraction
   if (items.contains(Item::beltTrapAttraction)) {
      bool moved[5][5];
      for (int i = 0; i < 5; i++) {
         for (int j = 0; j < 5; j++) {
            moved[i][j] = false;
         }
      }
      for (int i = x-2; i <= x+2; i++) {
         for (int j = y-2; j <= y+2; j++) {
            if (i>=0 && i<MAP_WIDTH && j>=0 && j<MAP_HEIGHT) {
               if (map[i][j] == TRAP && !moved[i-x+2][j-y+2]) {
                  int dirX = x-i;
                  int dirY = y-j;
                  if ((abs(dirX)+abs(dirY))>=2) {
                     if (abs(dirX) > abs(dirY)) {
                        dirX /= 2;
                        dirY = 0;
                     } else if (abs(dirX) < abs(dirY)) {
                        dirY /= 2;
                        dirX = 0;
                     } else if (abs(dirX)+abs(dirY) == 4){
                        dirX /= 2;
                        dirY /= 2;
                     }
                  }
                  if (map[i+dirX][j+dirY] == BLANK) {
                     map[i+dirX][j+dirY] = TRAP;
                     map[i][j] = BLANK;
                     moved[i+dirX-x+2][j+dirY-y+2] = true;
                  } else if (map[i+dirX][j+dirY] == HERO) {
                     map[i][j] = BLANK;
                     if (!dead) {
                        message("A trap is pulled onto the hero!", MessageType::NORMAL);
                        health -= 4;
                        if (health <= 0) {
                           dead = true;
                           message("The hero has died!", MessageType::IMPORTANT);
                           drawScreen();
                        }
                     } else {
                        message("A trap is pulled onto the hero's corpse!", MessageType::NORMAL);
                     }
                  } else if (map[i+dirX][j+dirY] == MONSTER) {
                     map[i][j] = BLANK;
                     Monster* target = findMonster(i+dirX, j+dirY);
                     message("A trap is pulled onto the " + target->name+ "!", MessageType::NORMAL);
                     hitMonster(target->x, target->y, 4);
                  }
               }
            }
         }
      }
   }
   mapModel->computeFov(x, y);
   return nextLevel;
}

bool Hero::inSpellRadius() const {
   return (Utils::dist(playerX, playerY, x, y) <= heroSpellRadius);
}

bool Hero::isAdjacent(int x, int y) const {
   bool result = false;
   if (x > 0 && x < MAP_WIDTH && y > 0 && y < MAP_HEIGHT) {
      result = map[x-1][y-1] == HERO || map[x-1][y] == HERO || map[x-1][y+1] == HERO || map[x][y-1] == HERO || map[x][y] == HERO || map[x][y+1] == HERO || map[x+1][y-1] == HERO || map[x+1][y] == HERO || map[x+1][y+1] == HERO;
   }
   return result;
}

// When the hero starts a new level
const std::array<std::string, 5> Hero::heroEntry {
   "I will spread the light to this dank pit!"s,
   "What tests await me here?"s,
   "This dungeon is afraid of me. I have seen its true face."s,
   "Come out, loathsome creatures, and meet your fate!"s,
   "These corridors will clamour with glorious bloodshed!"s
};

// When the hero finishes a level
const std::array<std::string, 5> Hero::heroExit {
   "Den of evil! You were nothing before me!"s,
   "A solid warm-up! What challenges are next?"s,
   "Haha! Victory is mine yet again!"s,
   "You thought you could stop me, miserable dungeon?"s,
   "This place was no match for my wits and my strength!"s
};

// When the hero picks a fight
const std::array<std::string, 10> Hero::heroFight {
   "Let battle be joined!"s,
   "Prepare to die, foul cur!"s,
   "Aha! A foul beast for me to slay!"s,
   "Taste steel, scum!"s,
   "Fiendish wretch! Soon the world will be rid of you!"s,
   "I'm going to push this sword through your face!"s,
   "Monster! This will be the last day of your life!"s,
   "Just when I thought there'd be no more killing today..."s,
   "Another glorious battle is at hand!"s,
   "Your life has become forfeit!"s
};

// When the hero kills a monster
const std::array<std::string, 10> Hero::heroKills {
   "And with barely a scratch!"s,
   "WHAT NEXT? WHAT WILL BE NEXT?!"s,
   "And stay down!"s,
   "Ballads will surely be sung of this glorious day!"s,
   "Haha! Just as I expected!"s,
   "You never had a chance!"s,
   "Off to the underworld with you!"s,
   "Is there nothing I cannot kill?"s,
   "That was all too easy!"s,
   "Ha! Haha! HAHAHAHAHAHA!!!"s
};

// When a pacified hero walks into a monster
const std::array<std::string, 5> Hero::heroBump {
   "Pardon me, good fellow!"s,
   "Ah, a kindly critter!"s,
   "It appears one of us must step aside."s,
   "Whoa! Hope I didn't hurt you."s,
   "I believe you're in my way."s
};

// When the hero finds an item
const std::array<std::string, 10> Hero::heroItem {
   "I always wanted one of these!"s,
   "Mine! All mine!"s,
   "A fitting trophy for my conquests!"s,
   "HAHA! AT LONG LAST!"s,
   "A worthy prize! This shall do nicely!"s,
   "What trash! I already have one of these!"s,
   "It's not a very good artifact, but it's a start."s,
   "Gimme gimme gimme!"s,
   "This will look great in my castle!"s,
   "Finally! Proof that I am the best!"s
};

// When the hero disrupts an illusion
const std::array<std::string, 5> Hero::heroIllusion {
   "Where did it go? WHERE DID IT GO?!"s,
   "Was I imagining it?"s,
   "Where's my treasure?!"s,
   "Foul magic! I shall not be tricked again!"s,
   "WHO STOLE MY TREASURE? COME FORTH!"s
};

// When "Charity" is cast on the hero
const std::array<std::string, 5> Hero::heroCharity {
   "I know others who could use this more than me"s,
   "Everything I have for the poor!"s,
   "This is worth enough to clothe a hundred beggars"s,
   "To be humble and frugal: That is the true path!"s,
   "I should sell this and give the wealth to the poor"s
};

// When the hero is able to see you
const std::array<std::string, 5> Hero::heroScared {
   "What the hell is that thing?!"s,
   "Stay away from me, spirit!"s,
   "Get back!"s,
   "What's this? Another foul monster?!"s,
   "Leave me alone, stranger!"s
};

// When the hero is blown by the "Blow" spell
const std::array<std::string, 5> Hero::heroBlow {
   "Whoa!"s,
   "What just happened?"s,
   "What devilry is this?"s,
   "WHOA!"s,
   "Is this foul magic?"s
};