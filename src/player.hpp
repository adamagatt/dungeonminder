#ifndef __PLAYER_HPP_
#define __PLAYER_HPP_

#include "config.hpp"
#include "position.hpp"

struct Player {
   Position pos;
   int heroMana {MANA_BLIP_SIZE*5};
   int monsterMana {MANA_BLIP_SIZE*5};
   int worldMana {MANA_BLIP_SIZE*5};
   int heroSpec {0};
   int monsterSpec {0};
   int worldSpec {0};

   void resetForNewLevel(const Position& startPos) {
      pos = startPos;
      heroMana = 5*MANA_BLIP_SIZE;
      monsterMana = 5*MANA_BLIP_SIZE;
      worldMana = 5*MANA_BLIP_SIZE;
   }
};

#endif