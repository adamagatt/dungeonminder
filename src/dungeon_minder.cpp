#include "dungeon_minder.hpp"

#include "hero.hpp"
#include "position.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>

// Main Method
int main() {
   // Creates the console
   TCODConsole::initRoot(80,60,"DungeonMinder",false);
   TCODSystem::setFps(25);

gameLoop:
   for (state.level = 1; state.level <= 10 && !TCODConsole::isWindowClosed(); state.level++) {
      if (state.level%3 == 0) {
         presentUpgradeMenu();
      }
      bool nextlevel = false;
      state.addMessage("Hero: " + Hero::heroEntry[Utils::randGen->getInt(0, 4)], MessageType::HERO);

      draw.generateMapNoise();

      state.createMap();

      // Initialise the hero and player
      Position heroPos = Utils::randomMapPosWithCondition(
         [](const auto& pos){return state.isEmptyPatch(pos);},
         2
      );

      state.setTile(heroPos.offset(0, 1), Tile::STAIRS_UP);


      Hero& hero = *(state.hero);
      Player& player = state.player;

      player.pos = heroPos.offset(0, -1);
      state.tileAt(player.pos) = Tile::PLAYER;
      hero.pos = heroPos;
      state.tileAt(hero.pos) = Tile::HERO;
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
      hero.items.clear();
      player.heroMana = 5*MANA_BLIP_SIZE;
      player.monsterMana = 5*MANA_BLIP_SIZE;
      player.worldMana = 5*MANA_BLIP_SIZE;

      state.illusion = {-1, -1};
      state.map.chest1Goal = {-1, -1};
      state.map.chest2Goal = {-1, -1};
      state.map.exitGoal = {-1, -1};

      if (state.level < 10) {
         // Place the stairs
         Position stairsPos = Utils::randomMapPosWithCondition(
            [&hero](const auto& pos){return state.isEmptyPatch(pos) && Utils::dist(hero.pos, pos) >= 20;}
         );
         state.setTile(stairsPos, Tile::STAIRS);
         state.map.exitGoal = stairsPos.offset(0, 1);
         
         // Place the chests
         Position chest1Pos = Utils::randomMapPosWithCondition([&hero](const auto& pos){
            return state.isEmptyPatch(pos) &&
                   Utils::dist(hero.pos, pos) >= 20 &&
                   Utils::dist(state.map.exitGoal, pos) >= 20;
         });
         state.setTile(chest1Pos, Tile::CHEST);
         state.map.chest1Goal = chest1Pos.offset(0, 1);

         Position chest2Pos = Utils::randomMapPosWithCondition([&hero](const auto& pos){
            return state.isEmptyPatch(pos) &&
                   Utils::dist(hero.pos, pos) >= 20 &&
                   Utils::dist(state.map.chest1Goal, pos) >= 20 &&
                   Utils::dist(state.map.exitGoal, pos) >= 20;
         });
         state.setTile(chest2Pos, Tile::CHEST);
         state.map.chest2Goal = chest2Pos.offset(0, 1);
      }

      // PLACE TRAPS
      for (int i = 0; i < 10; i++) {
         Position trapPos = Utils::randomMapPosWithCondition(
            [](const auto& pos){return state.tileAt(pos) == Tile::BLANK;}
         );
         state.tileAt(trapPos) = Tile::TRAP;
      }

      if (state.level < 10) {
         generateMonsters(state.level, 3);
      } else {
         generateMonsters(3, 1);
         generateEndBoss();
      }

      // Draw the map
      state.map.model->computeFov(hero.pos.x, hero.pos.y);
      draw.screen();

      // Game loop
      bool turnTaken = false;
      auto& console = *(TCODConsole::root);
      while (!TCODConsole::isWindowClosed() && !nextlevel) {
         turnTaken = false;
         Position dest = player.pos;

         TCOD_key_t key = Utils::getKeyPress();

         if (key.vk == TCODK_UP || key.vk == TCODK_KP8 || key.c == 'k') {
            dest = state.player.pos.offset(0, -1);
            turnTaken = true;
         } else if (key.vk == TCODK_DOWN || key.vk == TCODK_KP2 || key.c == 'j') {
            dest = state.player.pos.offset(0, +1);
            turnTaken = true;
         } else if (key.vk == TCODK_LEFT || key.vk == TCODK_KP4 || key.c == 'h') {
            dest = state.player.pos.offset(-1, 0);
            turnTaken = true;
         } else if (key.vk == TCODK_RIGHT || key.vk == TCODK_KP6 || key.c == 'l') {
            dest = state.player.pos.offset(+1, 0);
            turnTaken = true;
         } else if (key.vk == TCODK_KP7 || key.c == 'y') {
            dest = state.player.pos.offset(-1, -1);
            turnTaken = true;
         } else if (key.vk == TCODK_KP9 || key.c == 'u') {
            dest = state.player.pos.offset(+1, -1);
            turnTaken = true;
         } else if (key.vk == TCODK_KP1 || key.c == 'b') {
            dest = state.player.pos.offset(-1, +1);
            turnTaken = true;
         } else if (key.vk == TCODK_KP3 || key.c == 'n') {
            dest = state.player.pos.offset(+1, +1);
            turnTaken = true;
         } else if (key.vk == TCODK_SPACE || key.vk == TCODK_KP5) {
            turnTaken = true;
         } else if (key.vk == TCODK_ESCAPE) {
            exit(0);
         } else if (key.c == '\'') {
            int direction = Utils::getDirection();
            if (direction != 0) {
               Position target = state.player.pos.directionOffset(direction);
               state.setTile(target, Tile::WALL);
               hero.computePath();
            }
         } else if (key.vk == TCODK_F8) {
            player.heroSpec = 0;
            player.monsterSpec = 0;
            player.worldSpec = 0;
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
            draw.showMessageHistory();
            Utils::getKeyPress();
         } else if (key.c == 'v') {
            if (Utils::randGen->getInt(1, 2) == 1) {
               state.addMessage("You: HEY!", MessageType::SPELL);
            } else {
               state.addMessage("You: LISTEN!", MessageType::SPELL);
            }
         }
         if (dest.withinMap()) {
            Tile& destTile = state.tileAt(dest);
            if (destTile == Tile::BLANK) {
               state.tileAt(state.player.pos) = Tile::BLANK;
               destTile = Tile::PLAYER;
               player.pos = dest;
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
                  player.heroMana += 2;
                  player.monsterMana += 2;
                  player.worldMana += 2;
               } else {
                  player.heroMana++;
                  player.monsterMana++;
                  player.worldMana++;
               }
               // Ensures the mana generation doesn't go over te limit
               if (player.heroMana > 5*MANA_BLIP_SIZE) player.heroMana = 5*MANA_BLIP_SIZE;
               if (player.monsterMana > 5*MANA_BLIP_SIZE) player.monsterMana = 5*MANA_BLIP_SIZE;
               if (player.worldMana > 5*MANA_BLIP_SIZE) player.worldMana = 5*MANA_BLIP_SIZE;
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
               monster.takeTurn(state);
            }

            // Lowers the amount of clouds and forcefields
            for (int i = 0; i < MAP_WIDTH; i++) {
               for (int j = 0; j < MAP_HEIGHT; j++) {
                  if (state.map.cloud[i][j] > 0) {
                     state.map.cloud[i][j]--;
                     if (state.map.cloud[i][j] == 0) {
                        state.map.model->setProperties(i, j, true, true);
                     }
                  }
                  if (state.map.field[i][j] > 0) {
                     state.map.field[i][j]--;
                     if (state.map.field[i][j] == 0) {
                        state.map.model->setProperties(i, j, true, true);
                        state.tileAt({i, j}) = Tile::BLANK;
                     }
                  }
               }
            }
         }
         if (nextlevel && state.level < 10) {
            // Display the next state.level
            state.addMessage("Hero: " + Hero::heroExit[Utils::randGen->getInt(0, 4)], MessageType::HERO);
            state.addMessage("The hero descends to the next level of the dungeon!", MessageType::IMPORTANT);
         } else {
            state.map.model->computeFov(hero.pos.x, hero.pos.y);
         }
         draw.screen();
      }
   }
   draw.screen();
   if (state.bossDead) {
      draw.victoryScreen();
      for (TCOD_key_t key = Utils::getKeyPress(); key.vk != TCODK_ESCAPE; key = Utils::getKeyPress()) { }
   }
}

void presentUpgradeMenu() {
   Player& player = state.player;
   state.addMessage("You are now experienced enough to specialise your magic!", MessageType::IMPORTANT);

   draw.upgradeMenu();

   // Get keyboard input
   TCOD_key_t key;
   char spell = '\0';
   while (spell == '\0') {
      key = Utils::getKeyPress();
      if (key.vk == TCODK_BACKSPACE) {
         spell = ' ';
      } else if (((key.c == 'q' || key.c == 'a' || key.c == 'z') && player.heroSpec == 0) || ((key.c == 'w' || key.c == 's' || key.c == 'x') && player.monsterSpec == 0) || ((key.c == 'e' || key.c == 'd' || key.c == 'c') && player.worldSpec == 0)) {
         spell = key.c;
      }
   }
   switch (spell) {
      case 'q':
         state.addMessage("You specialise towards Pacifism! Check your spell menu!", MessageType::IMPORTANT);
         player.heroSpec = 1;
         break;
      case 'a':
         state.addMessage("You specialise towards Speed! Check your spell menu!", MessageType::IMPORTANT);
         player.heroSpec = 2;
         break;
      case 'z':
         state.addMessage("You specialise towards Heal! Check your spell menu!", MessageType::IMPORTANT);
         player.heroSpec = 3;
         break;
      case 'w':
         state.addMessage("You specialise towards Blind! Check your spell menu!", MessageType::IMPORTANT);
         player.monsterSpec = 1;
         break;
      case 's':
         state.addMessage("You specialise towards Rage! Check your spell menu!", MessageType::IMPORTANT);
         player.monsterSpec = 2;
         break;
      case 'x':
         state.addMessage("You specialise towards Sleep! Check your spell menu!", MessageType::IMPORTANT);
         player.monsterSpec = 3;
         break;
      case 'e':
         state.addMessage("You specialise towards Clear! Check your spell menu!", MessageType::IMPORTANT);
         player.worldSpec = 1;
         break;
      case 'd':
         state.addMessage("You specialise towards Cloud! Check your spell menu!", MessageType::IMPORTANT);
         player.worldSpec = 2;
         break;
      case 'c':
         state.addMessage("You specialise towards Trap! Check your spell menu!", MessageType::IMPORTANT);
         player.worldSpec = 3;
         break;
      default:
         state.addMessage("You choose not to specialise your spells", MessageType::IMPORTANT);
         break;
   }
}

bool castSpell(char spellChar) {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);
   Player& player = state.player;

   if (spellChar == 'j') {
      draw.spellMenu();

      // Get keyboard input
      TCOD_key_t key = Utils::getKeyPress();
      spellChar = key.c;
   }

   // DETERMINE THE SPELL TO CAST BASED ON THE KEY AND SPELL SPECIALISATION
   bool spellCast = false;
   switch (spellChar) {
      case 'q':
         if (player.heroMana >= MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[0][player.heroSpec][0]);
            if (spellCast) player.heroMana -= MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'a':
         if (player.heroMana >= 3*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[0][player.heroSpec][1]);
            if (spellCast) player.heroMana -= 3*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'z':
         if (player.heroMana >= 5*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[0][player.heroSpec][2]);
            if (spellCast) player.heroMana -= 5*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'w':
         if (player.monsterMana >= MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[1][player.monsterSpec][0]);
            if (spellCast) player.monsterMana -= MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 's':
         if (player.monsterMana >= 3*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[1][player.monsterSpec][1]);
            if (spellCast) player.monsterMana -= 3*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'x':
         if (player.monsterMana >= 5*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[1][player.monsterSpec][2]);
            if (spellCast) player.monsterMana -= 5*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'e':
         if (player.worldMana >= MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[2][player.worldSpec][0]);
            if (spellCast) player.worldMana -= MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'd':
         if (player.worldMana >= 3*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[2][player.worldSpec][1]);
            if (spellCast) player.worldMana -= 3*MANA_BLIP_SIZE;
         } else {
            state.addMessage("Insufficient power", MessageType::SPELL);
         }
         break;
      case 'c':
         if (player.worldMana >= 5*MANA_BLIP_SIZE) {
            spellCast = effectSpell(spellLists[2][player.worldSpec][2]);
            if (spellCast) player.worldMana -= 5*MANA_BLIP_SIZE;
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

   const Position& playerPos = state.player.pos;
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
         spellCast = castEffectSpellAtMonster(Condition::BLINDED, false);
         break;
      case Spell::RAGE:
         spellCast = castEffectSpellAtMonster(Condition::RAGED, false);
         break;
      case Spell::SLEEP:
         spellCast = castEffectSpellAtMonster(Condition::SLEEPING, false);
         break;
      case Spell::CLEAR:
         for (int i = playerPos.x-1; i <= playerPos.x+1; i++) {
            for (int j = playerPos.y-1; j <= playerPos.y+1; j++) {
               if (i>=0 && j>=0 && i<MAP_WIDTH && j <MAP_HEIGHT) {
                  Position curPos{i, j};
                  if (Tile& curTile = state.tileAt(curPos); curTile == Tile::WALL || curTile == Tile::TRAP) {
                     state.setTile(curPos, Tile::BLANK);
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
         for (int i = playerPos.x-2; i <= playerPos.x+2; i++) {
            for (int j = playerPos.y-2; j <= playerPos.y+2; j++) {
               int cloudDist = abs(playerPos.x-i)+abs(playerPos.y-j);
               if ((cloudDist < 4) && i>=0 && i<MAP_WIDTH && j>=0 && j <MAP_HEIGHT) {
                  if (state.tileAt({i, j}) != Tile::WALL) {
                     int newCloudLevel = CLOUD_TIME*(8-cloudDist)/8;
                     if (newCloudLevel > state.map.cloud[i][j]) {
                        state.map.cloud[i][j] = newCloudLevel;
                     }
                     state.map.model->setProperties(i, j, false, true);
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
            Position target = state.player.pos.directionOffset(direction);
            if (target.withinMap() && state.tileAt(target) == Tile::BLANK) {
               state.addMessage("You create a trap in the ground", MessageType::SPELL);
               state.tileAt(target) = Tile::TRAP;
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
               Position stepPos = state.player.pos.offset(diffX*i, diffY*i);
               if (stepPos.withinMap()) {
                  if (Tile& stepTile = state.tileAt(stepPos); stepTile == Tile::WALL || stepTile == Tile::TRAP) {
                     state.setTile(stepPos, Tile::BLANK);
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
            Position temp = state.player.pos.offset(
               Utils::randGen->getInt(-2, 2),
               Utils::randGen->getInt(-2, 2)
            );
            if (temp.withinMap()) {
               if (Tile& tempTile = state.tileAt(temp); tempTile == Tile::BLANK) {
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
            Position target = state.player.pos.directionOffset(direction);
            if (state.tileAt(target) == Tile::MONSTER) {
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
            Position target = state.player.pos.directionOffset(direction);
            if (state.tileAt(target)  == Tile::MONSTER) {
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
            Position target = state.player.pos.directionOffset(direction);
            if (target.withinMap() && state.tileAt(target) == Tile::BLANK ) {
               if (state.illusion.x != -1) {
                  state.tileAt(state.illusion) = Tile::BLANK;
               }
               state.illusion = target;
               state.tileAt(target) = Tile::ILLUSION;
            } else {
               state.addMessage("Without empty ground, the spell fizzles", MessageType::SPELL);
            }
            spellCast = true;
         }
         break;
      case Spell::WEAKEN:
         spellCast = castEffectSpellAtMonster(Condition::WEAKENED, true);
         break;
      case Spell::ALLY:
         spellCast = castEffectSpellAtMonster(Condition::ALLIED, false);
         break;
      case Spell::HALT:
         spellCast = castEffectSpellAtMonster(Condition::HALTED, false);
         break;
      case Spell::FLEE:
         spellCast = castEffectSpellAtMonster(Condition::FLEEING, false);
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
                     if (Position curr = state.player.pos.offset(i, j); curr.withinMap()) {
                        Tile& currTile = state.tileAt(curr);
                        if (currTile == Tile::TRAP || currTile == Tile::WALL) {
                           currTile = Tile::BLANK;
                        }
                        state.map.cloud[curr.x][curr.y] = (diff == 0 ? CLOUD_TIME : (CLOUD_TIME*7/8));
                        state.map.model->setProperties(curr.x, curr.y, false, true);
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
                  curr = state.player.pos.offset(diffX+(i*(1-abs(diffX))), diffY+(i*(1-abs(diffY))));
                  if (Tile& currTile = state.tileAt(curr); currTile == Tile::BLANK) {
                     state.map.field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     state.setTile(curr, Tile::FIELD);
                     fieldMade = true;
                  }
               }
               for (int i = -1; i <= 1; i++) {
                  curr = state.player.pos.offset(diffX*2+(i*(1-abs(diffX))), diffY*2+(i*(1-abs(diffY))));
                  if (Tile& currTile = state.tileAt(curr); currTile == Tile::BLANK) {
                     state.map.field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     state.setTile(curr, Tile::FIELD);
                     fieldMade = true;
                  }
               }
            } else {
               for (int i = -1; i <= 1; i++) {
                  curr = state.player.pos.offset(diffX-(i*diffX), diffY+(i*diffY));
                  if (Tile& currTile = state.tileAt(curr); currTile == Tile::BLANK) {
                     state.map.field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     state.setTile(curr, Tile::FIELD);
                     fieldMade = true;
                  }
               }
               for (int i = 0; i <= 3; i++) {
                  curr = state.player.pos.offset(
                     static_cast<int>(diffX/2.0f-((i-1.5)*diffX)),
                     static_cast<int>(diffY/2.0f+((i-1.5)*diffY))
                  );
                  if (Tile& currTile = state.tileAt(curr); currTile == Tile::BLANK) {
                     state.map.field[curr.x][curr.y] = FIELD_TIME-Utils::randGen->getInt(1, 3);
                     state.setTile(curr, Tile::FIELD);
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
                  Position step1 = state.player.pos.offset(i, j);
                  Tile& step1Tile = state.tileAt(step1);
                  Position step2 = state.player.pos.offset(2*i, 2*j);
                  Tile& step2Tile = state.tileAt(step2);
                  Position step3 = state.player.pos.offset(3*i, 3*j);
                  Tile& step3Tile = state.tileAt(step3);
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
                        state.tileAt(temp) = Tile::MONSTER;
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
                        state.tileAt(temp) = Tile::HERO;
                        hero.pos = temp;
                        hero.timer = hero.wait;
                        if (trapSprung) {
                           if (!hero.dead) {
                              state.addMessage("The hero is blown into the trap!", MessageType::SPELL);
                              hero.health -= 4;
                              step1Tile = Tile::BLANK;
                              state.tileAt(temp) = Tile::HERO;
                              hero.pos.x = temp.x; hero.pos.y = temp.y;
                              if (hero.health <= 0) {
                                 hero.die();
                              } else {
                                 state.addMessage("Hero: " + Hero::heroBlow[Utils::randGen->getInt(0, 4)], MessageType::HERO);
                              }
                           } else {
                              step1Tile = Tile::BLANK;
                              state.tileAt(temp) = Tile::HERO;
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
                        Tile& tempTile = state.tileAt(temp);
                        if (tempTile == Tile::BLANK) {
                           tempTile = Tile::TRAP;
                        } else if (tempTile == Tile::HERO) {
                           if (!hero.dead) {
                              state.addMessage("You blow a trap into the hero!", MessageType::SPELL);
                              hero.health -= 4;
                              if (hero.health <= 0) {
                                 hero.die();
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
         while (state.tileAt(temp) != Tile::BLANK) {
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

void generateEndBoss() {
   Hero& hero = *(state.hero);
   Position bossPos = Utils::randomMapPosWithCondition(
      [&hero](const auto& pos){return state.tileAt(pos) == Tile::BLANK && Utils::dist(pos, hero.pos) >= 30;}
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
   state.map.exitGoal = bossPos;
}

bool castEffectSpellAtMonster(Condition curCondition, bool append) {
   int direction = Utils::getDirection();
   if (direction == 0)
      return false;

   Position target = state.player.pos.directionOffset(direction);
   if (state.tileAt(target) == Tile::MONSTER) {
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