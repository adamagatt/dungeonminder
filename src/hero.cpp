#include "hero.hpp"
#include "utils.hpp"

using namespace std::string_literals;

Hero::Hero(GameState& game, const MessageCallback& message, const RedrawCallback& redrawCallback) :
   game(game),
   heroPathCallback(game.map),
   message(message),
   redrawCallback(redrawCallback)
   { }

bool Hero::checkWin() const {
   bool result = false;
   if (dest1.x == -1 && dest2.x == -1 && pos == stairs && !dead) {
      result = true;
   }
   return result;
}

void Hero::giveItem() {
   const int itemIdx = Utils::randGen->getInt(0, ITEM_COUNT-1);
   const Item itemFound = static_cast<Item>(itemIdx);
   // Display messages
   message("The hero finds " + ITEM_NAME.at(itemFound) + "!", MessageType::IMPORTANT);
   message("Hero: " + heroItem[Utils::randGen->getInt(0, 9)], MessageType::HERO);
   // Apply immediate effects of any item
   switch (itemFound) {
      case Item::scrollEarthquake:
         for (int i = 0; i < MAP_WIDTH; i++) {
            for (int j = 0; j < MAP_HEIGHT; j++) {
               Tile& testTile = Utils::tileAt(game.map, {i, j});
               if (testTile == BLANK && Utils::randGen->getInt(0, 29) == 0) {
                  testTile = WALL;
                  game.mapModel->setProperties(i, j, false, false);
               } else if (testTile == WALL && Utils::randGen->getInt(0, 10) == 0) {
                  testTile = BLANK;
                  game.mapModel->setProperties(i, j, true, true);
               }
            }
         }
         // Ensure the hero can get to all of his goals
         for (int j = 0; j < 2; j++) {
            Position src, dest;
            if (j == 0) {
               src = dest1;
               dest = dest2;
            } else {
               src = dest2;
               dest = stairs;
            }
            path.compute(src.x, src.y, dest.x, dest.y);
            int pathX = -1, pathY = -1;
            for (int i = 0; i < path.size(); i++) {
               path.get(i, &pathX, &pathY);
               if (game.map[pathX][pathY] == WALL) {
                  game.map[pathX][pathY] = BLANK;
                  game.mapModel->setProperties(pathX, pathY, true, true);
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
         for (auto& monster : game.monsterList) {
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
   if (dest1.x != -1) {
      path.compute(pos.x, pos.y, dest1.x, dest1.y);
   } else if (dest2.x != -1) {
      path.compute(pos.x, pos.y, dest2.x, dest2.y);
   } else {
      path.compute(pos.x, pos.y, stairs.x, stairs.y);
   }
   pathstep = 0;
}

bool Hero::move() {
   bool nextLevel = false;
   if (meditationTimer == 0) {
      // If the hero is ready to move
      if (timer == 0 || hasteTimer > 0) {
         nextLevel = checkWin();
         Utils::tileAt(game.map, pos) = BLANK;
         Position diff{0, 0};
         // If the hero can see the player
         if (seeInvisibleTimer > 0 && Utils::dist(pos, game.player) < 2 && game.mapModel->isInFov(pos.x, pos.y)) {
            diff = pos.offset(-game.player.x, -game.player.y);
            message("Hero: " + heroScared[Utils::randGen->getInt(0, 4)], MessageType::HERO);
         } else if (game.illusion.x != -1 && game.mapModel->isInFov(game.illusion.x, game.illusion.y)) {
            // If the hero sees the illusion, it takes priority
            int ptx = 0, pty = 0;
            bool reachable = path.compute(pos.x, pos.y, game.illusion.x, game.illusion.y); 
            if (reachable) {
               target = nullptr;
               path.get(0, &ptx, &pty);
               if (pos.x > ptx) diff.x = -1;
               if (pos.x < ptx) diff.x = 1;
               if (pos.y > pty) diff.y = -1;
               if (pos.y < pty) diff.y = 1;
            } else {
               diff = {
                  Utils::randGen->getInt(-1, 1),
                  Utils::randGen->getInt(-1, 1)
               };
            }
         } else if (target == nullptr || pacifismTimer > 0) {
            // If the hero doesn't have a target, attempt to find one
            if (pacifismTimer == 0) {
               target = game.heroFindMonster();
            }
            if (target != nullptr) {
               message("Hero: " + heroFight[Utils::randGen->getInt(0, 9)], MessageType::HERO);
               pathstep = 0;
               path.compute(pos.x, pos.y, target->x, target->y); 
            } else {
               if (pos == dest1) {
                  Utils::tileAt(game.map, dest1.offset(0, -1)) = CHEST_OPEN;
                  giveItem();
                  dest1 = {-1, -1};
               } else if (dest1.x == -1 && pos == dest2) {
                  Utils::tileAt(game.map, dest2.offset(0, -1)) = CHEST_OPEN;
                  giveItem();
                  dest2 = {-1, -1};
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
                     if (pos.x > ptx) diff.x = -1;
                     if (pos.x < ptx) diff.x = 1;
                     if (pos.y > pty) diff.y = -1;
                     if (pos.y < pty) diff.y = 1;
                  } else {
                     computePath();
                  }
               }
            }
         } else {
            // If the hero had an existing target, move to attack
            int ptx = 0, pty = 0;
            bool reachable = path.compute(pos.x, pos.y, target->x, target->y); 
            if (reachable) {
               path.get(0, &ptx, &pty);
               if (pos.x > ptx) diff.x = -1;
               if (pos.x < ptx) diff.x = 1;
               if (pos.y > pty) diff.y = -1;
               if (pos.y < pty) diff.y = 1;
            } else {
               diff = {
                  Utils::randGen->getInt(-1, 1),
                  Utils::randGen->getInt(-1, 1)
               };
            }
         }
         if (diff.x != 0 && diff.y != 0) {
            if (Utils::randGen->getInt(0, 1) == 0) {
               diff.x = 0;
            } else {
               diff.y = 0;
            }
         }
         Position dest = pos.offset(diff);
         if (Utils::tileAt(game.map, dest) == MONSTER) {
            if (pacifismTimer > 0) {
               game.mapModel->setProperties(dest.x, dest.y, true, false);
               computePath();
               game.mapModel->setProperties(dest.x, dest.y, true, true);
               message("Hero: "+heroBump[Utils::randGen->getInt(0, 4)], MessageType::HERO);
            } else {
               target = game.findMonster(dest.x, dest.y);
               char buffer[20];
               sprintf(buffer, "%d", damage);
               message("The hero hits the " + target->name + " for " + buffer + " damage", MessageType::NORMAL);
               int selfDamage = 0;
               if (items.contains(Item::carelessGauntlets) && (target->health < damage)) {
                  selfDamage = damage - target->health;
               }
               game.hitMonster(dest.x, dest.y, damage);
               if (blinking) {
                  gainHealth(2);
               }
               if (target == nullptr) {
                  if (selfDamage > 1) {
                     char buffer[20];
                     sprintf(buffer, "%d", selfDamage/2);
                     message("The hero suffers "s + buffer + " damage from the effort!", MessageType::NORMAL);
                     health -= selfDamage;
                     if (health <= 0) {
                        dead = true;
                        message("The hero has died!", MessageType::IMPORTANT);
                        redrawCallback();
                     }

                  }
                  if (!dead) {
                     message("Hero: "+heroKills[Utils::randGen->getInt(0, 9)], MessageType::HERO);
                     computePath();
                  }
               }
            }
         } else if (Utils::tileAt(game.map, dest) == FIELD) {
            message("The hero is blocked by the forcefield", MessageType::SPELL);
            computePath();
         } else if (Utils::tileAt(game.map, dest) == WALL) {
            computePath();
         } else if (Utils::tileAt(game.map, dest) == TRAP) {
            message("The hero falls into the trap!", MessageType::NORMAL);
            health -= 4;
            Utils::tileAt(game.map, dest) = BLANK;
            pos = dest;
            if (health <= 0) {
               dead = true;
               message("The hero has died!", MessageType::IMPORTANT);
               redrawCallback();
            }
         } else if (Utils::tileAt(game.map, dest) == ILLUSION) {
            Utils::tileAt(game.map, dest) = BLANK;
            game.illusion.x = -1; game.illusion.y = -1;
            message("The hero disrupts the illusion", MessageType::SPELL);
            message("Hero: " + heroIllusion[Utils::randGen->getInt(0, 4)], MessageType::HERO);
         } else if (Utils::tileAt(game.map, dest) == BLANK) {
            pos = dest;
         } else if (Utils::tileAt(game.map, dest) == PLAYER) {
            std::swap(game.player, pos);
            message("The hero passes through you", MessageType::NORMAL);
            game.map[game.player.x][game.player.y] = PLAYER;
         }
         Utils::tileAt(game.map, pos) = HERO;
         if ((items.contains(Item::slowBoots) && Utils::randGen->getInt(1, 2) == 1) || slow) {
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
               redrawCallback();
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
      if (game.monsterList.size() < MAX_MONSTERS && summonMonsterTimer%15 == 1) {
         Position l{0, 0};
         while (Utils::tileAt(game.map, l) != BLANK || Utils::dist(pos, l) > 10) {
            l = {
               Utils::randGen->getInt(0, MAP_WIDTH-1),
               Utils::randGen->getInt(0, MAP_HEIGHT-1)
            };
         }
         int randomMonster = Utils::randGen->getInt((game.level/2+1)-1, (game.level/2+1)+3);
         game.addSpecifiedMonster(l.x, l.y, randomMonster, true);
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
      for (int i = pos.x-2; i <= pos.x+2; i++) {
         for (int j = pos.y-2; j <= pos.y+2; j++) {
            Position testPos{i, j};
            if (testPos.withinMap()) {
               if (Utils::tileAt(game.map, testPos) == TRAP && !moved[i-pos.x+2][j-pos.y+2]) {
                  Position dir = pos.offset(-i, -j);
                  if ((abs(dir.x)+abs(dir.y))>=2) {
                     if (abs(dir.x) > abs(dir.y)) {
                        dir.x /= 2;
                        dir.y = 0;
                     } else if (abs(dir.x) < abs(dir.y)) {
                        dir.y /= 2;
                        dir.x = 0;
                     } else if (abs(dir.x)+abs(dir.y) == 4){
                        dir.x /= 2;
                        dir.y /= 2;
                     }
                  }
                  Position trapPos = dir.offset(i, j);
                  Tile& trapTile = Utils::tileAt(game.map, trapPos);
                  Tile& testTile = Utils::tileAt(game.map, testPos);
                  if (trapTile == BLANK) {
                     trapTile = TRAP;
                     testTile = BLANK;
                     moved[i+dir.x-pos.x+2][j+dir.y-pos.y+2] = true;
                  } else if (trapTile == HERO) {
                     testTile = BLANK;
                     if (!dead) {
                        message("A trap is pulled onto the hero!", MessageType::NORMAL);
                        health -= 4;
                        if (health <= 0) {
                           dead = true;
                           message("The hero has died!", MessageType::IMPORTANT);
                           redrawCallback();
                        }
                     } else {
                        message("A trap is pulled onto the hero's corpse!", MessageType::NORMAL);
                     }
                  } else if (trapTile == MONSTER) {
                     testTile = BLANK;
                     Monster* target = game.findMonster(dir.offset(i, j));
                     message("A trap is pulled onto the " + target->name+ "!", MessageType::NORMAL);
                     game.hitMonster(target->x, target->y, 4);
                  }
               }
            }
         }
      }
   }
   game.mapModel->computeFov(pos.x, pos.y);
   return nextLevel;
}

bool Hero::inSpellRadius() const {
   return Utils::dist(game.player, pos) <= SPELL_RADIUS;
}

bool Hero::isAdjacent(int x, int y) const {
   bool result = false;
   if (x > 0 && x < MAP_WIDTH && y > 0 && y < MAP_HEIGHT) {
      result = game.map[x-1][y-1] == HERO || game.map[x-1][y] == HERO || game.map[x-1][y+1] == HERO || game.map[x][y-1] == HERO || game.map[x][y] == HERO || game.map[x][y+1] == HERO || game.map[x+1][y-1] == HERO || game.map[x+1][y] == HERO || game.map[x+1][y+1] == HERO;
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