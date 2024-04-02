#include "draw.hpp"

#include "config.hpp"
#include "hero.hpp"
#include "utils.hpp"

#include <cmath>
#include <iterator>

Draw::Draw(const GameState& state) : state(state) {
   // Initialise the spell menu
   initialiseSpellMenu();
}

void Draw::directionScreen() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   // Display a screen to the user
   directionFrame(20, BOTTOM+3, 41, 5);

   console.setDefaultBackground(TCODColor::black);
   console.setDefaultForeground(TCODColor::white);
   console.print(28, BOTTOM+5, "Please choose a direction"s);
   console.flush();
}

void Draw::directionFrame(int x, int y, int width, int height) {
   auto& console = *(TCODConsole::root);

   console.setDefaultForeground(TCODColor::yellow);
   console.setDefaultBackground(TCODColor(128, 64, 0));
   console.rect(x+1, y+1, width-2, height-2, true, TCOD_BKGND_SET);

   for (int i = x; i < x+width; ++i) {
      console.putChar(i, y, 196, TCOD_BKGND_SET);
      console.putChar(i, y+height-1, 196, TCOD_BKGND_SET);
   }
   for (int j = y; j < y+height; ++j) {
      console.putChar(x, j, 179, TCOD_BKGND_SET);
      console.putChar(x+width-1, j, 179, TCOD_BKGND_SET);
   }
   console.putChar(x, y, 218, TCOD_BKGND_SET);
   console.putChar(x+width-1, y, 191, TCOD_BKGND_SET);
   console.putChar(x, y+height-1, 192, TCOD_BKGND_SET);
   console.putChar(x+width-1, y+height-1, 217, TCOD_BKGND_SET);
}

void Draw::initialiseSpellMenu() {
   // Instantiate the spell menu
   backboard = std::make_unique<TCODConsole>(66, 35);

   // Draw the edges
   backboard->setDefaultBackground(TCODColor(84, 40, 0));
   for (int i = 0; i <= 65; i++) {
      backboard->putChar(i, 0, ' ', TCOD_BKGND_SET);
   }
   for (int j = 1; j <= 34; j++) {
      backboard->putChar(0, j, ' ', TCOD_BKGND_SET);
   }
   backboard->setDefaultBackground(TCODColor(210, 96, 0));
   for (int i = 0; i <= 65; i++) {
      backboard->putChar(i, 34, ' ', TCOD_BKGND_SET);
   }
   for (int j = 1; j <= 33; j++) {
      backboard->putChar(65, j, ' ', TCOD_BKGND_SET);
   }

   backboard->setDefaultBackground(TCODColor(128, 64, 0));
   for (int i = 1; i <= 64; i++) {
      for (int j = 1; j <= 33; j++) {
         backboard->putChar(i, j, ' ', TCOD_BKGND_SET);
      }
   }
}

void Draw::spellMenu() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   TCODConsole::blit(backboard.get(), 0, 0, 66, 35, &console, MENU_X, MENU_Y);

   console.setDefaultForeground(TCODColor::darkGrey);
   console.print(05, 59, "m"s);
   console.print(27, 59, "TAB"s);
   console.print(46, 59, "F5"s);
   console.print(64, 59, "F8"s);

   // Draw the borders
   showCommonSpellGUI();
   console.setDefaultForeground(TCODColor(180, 100, 0));
   for (int j = 0; j < 3; j++) {
      console.putChar(MENU_X+9, MENU_Y+j*10+13, 193, TCOD_BKGND_NONE);
      console.putChar(MENU_X+21, MENU_Y+j*10+13, 193, TCOD_BKGND_NONE);
      console.putChar(MENU_X+9, MENU_Y+j*10+12, 179, TCOD_BKGND_NONE);
      console.putChar(MENU_X+21, MENU_Y+j*10+12, 179, TCOD_BKGND_NONE);
      console.putChar(MENU_X+9, MENU_Y+j*10+11, 218, TCOD_BKGND_NONE);
      console.putChar(MENU_X+21, MENU_Y+j*10+11, 191, TCOD_BKGND_NONE);
      for (int i = MENU_X+10; i <= MENU_X+ 20; i++) {
         console.putChar(i, MENU_Y+j*10+11, 196, TCOD_BKGND_NONE);
      }
      console.putChar(MENU_X+30, MENU_Y+j*10+13, 193, TCOD_BKGND_NONE);
      console.putChar(MENU_X+42, MENU_Y+j*10+13, 193, TCOD_BKGND_NONE);
      console.putChar(MENU_X+30, MENU_Y+j*10+12, 179, TCOD_BKGND_NONE);
      console.putChar(MENU_X+42, MENU_Y+j*10+12, 179, TCOD_BKGND_NONE);
      console.putChar(MENU_X+30, MENU_Y+j*10+11, 218, TCOD_BKGND_NONE);
      console.putChar(MENU_X+42, MENU_Y+j*10+11, 191, TCOD_BKGND_NONE);
      for (int i = MENU_X+31; i <= MENU_X+ 41; i++) {
         console.putChar(i, MENU_Y+j*10+11, 196, TCOD_BKGND_NONE);
      }
      console.putChar(MENU_X+51, MENU_Y+j*10+13, 193, TCOD_BKGND_NONE);
      console.putChar(MENU_X+63, MENU_Y+j*10+13, 193, TCOD_BKGND_NONE);
      console.putChar(MENU_X+51, MENU_Y+j*10+12, 179, TCOD_BKGND_NONE);
      console.putChar(MENU_X+63, MENU_Y+j*10+12, 179, TCOD_BKGND_NONE);
      console.putChar(MENU_X+51, MENU_Y+j*10+11, 218, TCOD_BKGND_NONE);
      console.putChar(MENU_X+63, MENU_Y+j*10+11, 191, TCOD_BKGND_NONE);
      for (int i = MENU_X+52; i <= MENU_X+ 62; i++) {
         console.putChar(i, MENU_Y+j*10+11, 196, TCOD_BKGND_NONE);
      }
   }
   // Draw the placards
   console.setDefaultForeground(TCODColor::yellow);
   console.setDefaultBackground(TCODColor(78, 78, 78));
   {
      Utils::WithBackgroundSet set(console);
      console.print(MENU_X+24, MENU_Y, "  List of Spells  "s);
      console.print(MENU_X+36, MENU_Y+34, " Any other key to cancel "s);
   }

   // Draw the blips
   console.setDefaultForeground(TCODColor(128, 64, 0));
   console.setDefaultBackground(TCODColor::lightBlue);
   console.putChar(MENU_X+18, MENU_Y+12, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+17, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+18, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+19, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+16, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+17, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+18, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+19, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+20, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.setDefaultBackground(TCODColor::red);
   console.putChar(MENU_X+39, MENU_Y+12, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+38, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+39, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+40, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+37, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+38, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+39, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+40, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+41, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.setDefaultBackground(TCODColor(156, 156, 156));
   console.putChar(MENU_X+60, MENU_Y+12, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+59, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+60, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+61, MENU_Y+22, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+58, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+59, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+60, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+61, MENU_Y+32, 224, TCOD_BKGND_SET);
   console.putChar(MENU_X+62, MENU_Y+32, 224, TCOD_BKGND_SET);

   console.setDefaultForeground(TCODColor::white);
   console.print(MENU_X+19, MENU_Y+5, "q"s);
   console.print(MENU_X+19, MENU_Y+15, "a"s);
   console.print(MENU_X+19, MENU_Y+25, "z"s);
   console.print(MENU_X+40, MENU_Y+5, "w"s);
   console.print(MENU_X+40, MENU_Y+15, "s"s);
   console.print(MENU_X+40, MENU_Y+25, "x"s);
   console.print(MENU_X+61, MENU_Y+5, "e"s);
   console.print(MENU_X+61, MENU_Y+15, "d"s);
   console.print(MENU_X+61, MENU_Y+25, "c"s);

   for (int i = 10; i <= 52; i+=21) {
      for (int j = 12; j <=32; j+=10) {
         console.print(MENU_X+i, MENU_Y+j, "Cost:"s);
      }

   }

   int heroSpec = state.getHeroSpec();
   int monsterSpec = state.getMonsterSpec();
   int worldSpec = state.getWorldSpec();

   // DRAW THE SPELL INFORMATION
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::lightBlue);
      console.print(MENU_X+3, MENU_Y+5, "Pacifism"s);
      console.print(MENU_X+3, MENU_Y+15, "Speed"s);
      console.print(MENU_X+3, MENU_Y+25, "Heal"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+3, MENU_Y+7, "Hero will not"s);
      console.print(MENU_X+3, MENU_Y+8, "attack or pursue"s);
      console.print(MENU_X+3, MENU_Y+9, "monsters"s);
      console.print(MENU_X+3, MENU_Y+17, "Hero temporarily"s);
      console.print(MENU_X+3, MENU_Y+18, "moves at double"s);
      console.print(MENU_X+3, MENU_Y+19, "speed"s);
      console.print(MENU_X+3, MENU_Y+27, "Hero recovers from"s);
      console.print(MENU_X+3, MENU_Y+28, "some injuries"s);
   } else if (heroSpec == 1) {
      console.setDefaultForeground(TCODColor::lightBlue);
      console.print(MENU_X+3, MENU_Y+5, "Pacifism"s);
      console.print(MENU_X+3, MENU_Y+15, "Meditation"s);
      console.print(MENU_X+3, MENU_Y+25, "Charity"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+3, MENU_Y+7, "Hero will not"s);
      console.print(MENU_X+3, MENU_Y+8, "attack or pursue"s);
      console.print(MENU_X+3, MENU_Y+9, "monsters"s);
      console.print(MENU_X+3, MENU_Y+17, "Hero stands still,"s);
      console.print(MENU_X+3, MENU_Y+18, "increasing spell"s);
      console.print(MENU_X+3, MENU_Y+19, "power generation"s);
      console.print(MENU_X+3, MENU_Y+27, "Convert all found"s);
      console.print(MENU_X+3, MENU_Y+28, "treasure into hero"s);
      console.print(MENU_X+3, MENU_Y+29, "health"s);
   } else if (heroSpec == 2) {
      console.setDefaultForeground(TCODColor::lightBlue);
      console.print(MENU_X+3, MENU_Y+5, "Slow"s);
      console.print(MENU_X+3, MENU_Y+15, "Speed"s);
      console.print(MENU_X+3, MENU_Y+25, "Blink"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+3, MENU_Y+7, "Hero toggles"s);
      console.print(MENU_X+3, MENU_Y+8, "between slow and"s);
      console.print(MENU_X+3, MENU_Y+9, "normal speed"s);
      console.print(MENU_X+3, MENU_Y+17, "Hero temporarily"s);
      console.print(MENU_X+3, MENU_Y+18, "moves at double"s);
      console.print(MENU_X+3, MENU_Y+19, "speed"s);
      console.print(MENU_X+3, MENU_Y+27, "Hero makes instant"s);
      console.print(MENU_X+3, MENU_Y+28, "moves and heals by"s);
      console.print(MENU_X+3, MENU_Y+29, "attacking enemies"s);
   } else if (heroSpec == 3) {
      console.setDefaultForeground(TCODColor::lightBlue);
      console.print(MENU_X+3, MENU_Y+5, "Shield"s);
      console.print(MENU_X+3, MENU_Y+15, "Regenerate"s);
      console.print(MENU_X+3, MENU_Y+25, "Heal"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+3, MENU_Y+7, "Hero becomes"s);
      console.print(MENU_X+3, MENU_Y+8, "temporarily immune"s);
      console.print(MENU_X+3, MENU_Y+9, "to ranged attacks"s);
      console.print(MENU_X+3, MENU_Y+17, "Hero slowly heals"s);
      console.print(MENU_X+3, MENU_Y+18, "over a short"s);
      console.print(MENU_X+3, MENU_Y+19, "period"s);
      console.print(MENU_X+3, MENU_Y+27, "Hero recovers from"s);
      console.print(MENU_X+3, MENU_Y+28, "some injuries"s);
   }
   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::red);
      console.print(MENU_X+24, MENU_Y+5, "Blind"s);
      console.print(MENU_X+24, MENU_Y+15, "Rage"s);
      console.print(MENU_X+24, MENU_Y+25, "Sleep"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+24, MENU_Y+7, "Monster will not"s);
      console.print(MENU_X+24, MENU_Y+8, "approach hero or"s);
      console.print(MENU_X+24, MENU_Y+9, "use ranged attacks"s);
      console.print(MENU_X+24, MENU_Y+17, "Monster will"s);
      console.print(MENU_X+24, MENU_Y+18, "attack the nearest"s);
      console.print(MENU_X+24, MENU_Y+19, "living creature"s);
      console.print(MENU_X+24, MENU_Y+27, "Monster will be"s);
      console.print(MENU_X+24, MENU_Y+28, "unable to act for"s);
      console.print(MENU_X+24, MENU_Y+29, "a short time"s);
   } else if (monsterSpec == 1) {
      console.setDefaultForeground(TCODColor::red);
      console.print(MENU_X+24, MENU_Y+5, "Blind"s);
      console.print(MENU_X+24, MENU_Y+15, "Maim"s);
      console.print(MENU_X+24, MENU_Y+25, "Cripple"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+24, MENU_Y+7, "Monster will not"s);
      console.print(MENU_X+24, MENU_Y+8, "approach hero or"s);
      console.print(MENU_X+24, MENU_Y+9, "use ranged attacks"s);
      console.print(MENU_X+24, MENU_Y+17, "Monster suffers"s);
      console.print(MENU_X+24, MENU_Y+18, "damage when"s);
      console.print(MENU_X+24, MENU_Y+19, "attacking"s);
      console.print(MENU_X+24, MENU_Y+27, "Monster is reduced"s);
      console.print(MENU_X+24, MENU_Y+28, "to half of its"s);
      console.print(MENU_X+24, MENU_Y+29, "current health"s);
   } else if (monsterSpec == 2) {
      console.setDefaultForeground(TCODColor::red);
      console.print(MENU_X+24, MENU_Y+5, "Weaken"s);
      console.print(MENU_X+24, MENU_Y+15, "Rage"s);
      console.print(MENU_X+24, MENU_Y+25, "Ally"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+24, MENU_Y+7, "Monster does less"s);
      console.print(MENU_X+24, MENU_Y+8, "damage with each"s);
      console.print(MENU_X+24, MENU_Y+9, "attack"s);
      console.print(MENU_X+24, MENU_Y+17, "Monster will"s);
      console.print(MENU_X+24, MENU_Y+18, "attack the nearest"s);
      console.print(MENU_X+24, MENU_Y+19, "living creature"s);
      console.print(MENU_X+24, MENU_Y+27, "Monster fights for"s);
      console.print(MENU_X+24, MENU_Y+28, "the hero for a"s);
      console.print(MENU_X+24, MENU_Y+29, "short while"s);
   } else if (monsterSpec == 3) {
      console.setDefaultForeground(TCODColor::red);
      console.print(MENU_X+24, MENU_Y+5, "Halt"s);
      console.print(MENU_X+24, MENU_Y+15, "Flee"s);
      console.print(MENU_X+24, MENU_Y+25, "Sleep"s);
      console.setDefaultForeground(TCODColor::white);
      console.print(MENU_X+24, MENU_Y+7, "Monster will pause"s);
      console.print(MENU_X+24, MENU_Y+8, "for a while or"s);
      console.print(MENU_X+24, MENU_Y+9, "until attacked"s);
      console.print(MENU_X+24, MENU_Y+17, "Monster will run"s);
      console.print(MENU_X+24, MENU_Y+18, "if it sees the"s);
      console.print(MENU_X+24, MENU_Y+19, "hero"s);
      console.print(MENU_X+24, MENU_Y+27, "Monster will be"s);
      console.print(MENU_X+24, MENU_Y+28, "unable to act for"s);
      console.print(MENU_X+24, MENU_Y+29, "a short time"s);
   }
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor(156, 156, 156));
      console.print(MENU_X+45, MENU_Y+5, "Clear"s);
      console.print(MENU_X+45, MENU_Y+15, "Cloud"s);
      console.print(MENU_X+45, MENU_Y+25, "Trap"s);
      console.setDefaultForeground(TCODColor::white);

      console.print(MENU_X+45, MENU_Y+7, "Clears walls and"s);
      console.print(MENU_X+45, MENU_Y+8, "traps adjacent to"s);
      console.print(MENU_X+45, MENU_Y+9, "the player"s);
      console.print(MENU_X+45, MENU_Y+17, "Creates a cloud of"s);
      console.print(MENU_X+45, MENU_Y+18, "smoke that cannot"s);
      console.print(MENU_X+45, MENU_Y+19, "be seen through"s);
      console.print(MENU_X+45, MENU_Y+27, "Creates a trap in"s);
      console.print(MENU_X+45, MENU_Y+28, "an empty space"s);
   } else if (worldSpec == 1) {
      console.setDefaultForeground(TCODColor(156, 156, 156));
      console.print(MENU_X+45, MENU_Y+5, "Clear"s);
      console.print(MENU_X+45, MENU_Y+15, "Blow"s);
      console.print(MENU_X+45, MENU_Y+25, "Illusion"s);
      console.setDefaultForeground(TCODColor::white);

      console.print(MENU_X+45, MENU_Y+7, "Clears walls and"s);
      console.print(MENU_X+45, MENU_Y+8, "traps adjacent to"s);
      console.print(MENU_X+45, MENU_Y+9, "the player"s);
      console.print(MENU_X+45, MENU_Y+17, "Pushes creatures"s);
      console.print(MENU_X+45, MENU_Y+18, "and traps away"s);
      console.print(MENU_X+45, MENU_Y+19, "from around you"s);
      console.print(MENU_X+45, MENU_Y+27, "Makes an illusion"s);
      console.print(MENU_X+45, MENU_Y+28, "that grabs the"s);
      console.print(MENU_X+45, MENU_Y+29, "hero's attention"s);
   } else if (worldSpec == 2) {
      console.setDefaultForeground(TCODColor(156, 156, 156));
      console.print(MENU_X+45, MENU_Y+5, "Screen"s);
      console.print(MENU_X+45, MENU_Y+15, "Cloud"s);
      console.print(MENU_X+45, MENU_Y+25, "Field"s);
      console.setDefaultForeground(TCODColor::white);

      console.print(MENU_X+45, MENU_Y+7, "Creates a cloud"s);
      console.print(MENU_X+45, MENU_Y+8, "screen that can"s);
      console.print(MENU_X+45, MENU_Y+9, "dissolve walls"s);
      console.print(MENU_X+45, MENU_Y+17, "Creates a cloud of"s);
      console.print(MENU_X+45, MENU_Y+18, "smoke that cannot"s);
      console.print(MENU_X+45, MENU_Y+19, "be seen through"s);
      console.print(MENU_X+45, MENU_Y+27, "Creates a magical"s);
      console.print(MENU_X+45, MENU_Y+28, "wall that cannot"s);
      console.print(MENU_X+45, MENU_Y+29, "be moved through"s);
   } else if (worldSpec == 3) {
      console.setDefaultForeground(TCODColor(156, 156, 156));
      console.print(MENU_X+45, MENU_Y+5, "Tunnel"s);
      console.print(MENU_X+45, MENU_Y+15, "Minefield"s);
      console.print(MENU_X+45, MENU_Y+25, "Trap"s);
      console.setDefaultForeground(TCODColor::white);

      console.print(MENU_X+45, MENU_Y+7, "Digs a short"s);
      console.print(MENU_X+45, MENU_Y+8, "corridor in a"s);
      console.print(MENU_X+45, MENU_Y+9, "chosen direction"s);
      console.print(MENU_X+45, MENU_Y+17, "Places some traps"s);
      console.print(MENU_X+45, MENU_Y+18, "randomly around"s);
      console.print(MENU_X+45, MENU_Y+19, "the player"s);
      console.print(MENU_X+45, MENU_Y+27, "Creates a trap in"s);
      console.print(MENU_X+45, MENU_Y+28, "an empty space"s);
   }
   console.setDefaultBackground(TCODColor::black);
   // Flush the main console
   console.flush();
}

void Draw::upgradeMenu() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   screen();
   TCODConsole::blit(backboard.get(), 0, 0, 66, 35, &console, MENU_X, MENU_Y);

   // Draw the borders
   showCommonSpellGUI();

   // Draw the placards
   console.setDefaultForeground(TCODColor::yellow);
   console.setDefaultBackground(TCODColor(78, 78, 78));
   {
      Utils::WithBackgroundSet set(console);
      console.print(MENU_X+21, MENU_Y, "  Spell Specialisation  "s);
      console.print(MENU_X+5, MENU_Y+34, " Choose a specialisation or press <backspace> to cancel "s);
   }

   // Draw specialisation categories
   console.setDefaultForeground(TCODColor::lightBlue);
   console.print(MENU_X+3, MENU_Y+5, "Pacifism"s);
   console.print(MENU_X+3, MENU_Y+15, "Speed"s);
   console.print(MENU_X+3, MENU_Y+25, "Heal"s);
   console.setDefaultForeground(TCODColor::red);
   console.print(MENU_X+24, MENU_Y+5,"Blind"s);
   console.print(MENU_X+24, MENU_Y+15,"Rage"s);
   console.print(MENU_X+24, MENU_Y+25,"Sleep"s);
   console.setDefaultForeground(TCODColor(156, 156, 156));
   console.print(MENU_X+45, MENU_Y+5,"Clear"s);
   console.print(MENU_X+45, MENU_Y+15,"Cloud"s);
   console.print(MENU_X+45, MENU_Y+25,"Trap"s);

   int heroSpec = state.getHeroSpec();
   int monsterSpec = state.getMonsterSpec();
   int worldSpec = state.getWorldSpec();

   // List the spells in each
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+19, MENU_Y+5, "q"s);
   console.print(MENU_X+19, MENU_Y+15, "a"s);
   console.print(MENU_X+19, MENU_Y+25, "z"s);
   if (heroSpec == 0 || heroSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+5, MENU_Y+8, "Pacifism"s);
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (heroSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+5, MENU_Y+9, "Meditation"s);
   console.print(MENU_X+5, MENU_Y+10, "Charity"s);
   if (heroSpec == 0 || heroSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+5, MENU_Y+19, "Speed"s);
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (heroSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+5, MENU_Y+18, "Slow"s);
   console.print(MENU_X+5, MENU_Y+20, "Blink"s);
   if (heroSpec == 0 || heroSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+5, MENU_Y+30, "Heal"s);
   if (heroSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (heroSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+5, MENU_Y+28, "Shield"s);
   console.print(MENU_X+5, MENU_Y+29, "Regenerate"s);

   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+40, MENU_Y+5, "w"s);
   console.print(MENU_X+40, MENU_Y+15, "s"s);
   console.print(MENU_X+40, MENU_Y+25, "x"s);
   if (monsterSpec == 0 || monsterSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+26, MENU_Y+8, "Blind"s);
   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (monsterSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+26, MENU_Y+9, "Maim"s);
   console.print(MENU_X+26, MENU_Y+10, "Cripple"s);
   if (monsterSpec == 0 || monsterSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+26, MENU_Y+19, "Rage"s);
   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (monsterSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+26, MENU_Y+18, "Weaken"s);
   console.print(MENU_X+26, MENU_Y+20, "Ally"s);
   if (monsterSpec == 0 || monsterSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+26, MENU_Y+30, "Sleep"s);
   if (monsterSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (monsterSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+26, MENU_Y+28, "Halt"s);
   console.print(MENU_X+26, MENU_Y+29, "Flee"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+61, MENU_Y+5, "e"s);
   console.print(MENU_X+61, MENU_Y+15, "d"s);
   console.print(MENU_X+61, MENU_Y+25, "c"s);
   if (worldSpec == 0 || worldSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+47, MENU_Y+8, "Clear"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (worldSpec == 1) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+47, MENU_Y+9, "Blow"s);
   console.print(MENU_X+47, MENU_Y+10, "Illusion"s);
   if (worldSpec == 0 || worldSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+47, MENU_Y+19, "Cloud"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (worldSpec == 2) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+47, MENU_Y+18, "Screen"s);
   console.print(MENU_X+47, MENU_Y+20, "Field"s);
   if (worldSpec == 0 || worldSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+47, MENU_Y+30, "Trap"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else if (worldSpec == 3) {
      console.setDefaultForeground(TCODColor::yellow);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }
   console.print(MENU_X+47, MENU_Y+28, "Tunnel"s);
   console.print(MENU_X+47, MENU_Y+29, "Minefield"s);
   if (worldSpec == 0) {
      console.setDefaultForeground(TCODColor::white);
   } else {
      console.setDefaultForeground(TCODColor::darkGrey);
   }

   // Flush the main console
   console.setDefaultBackground(TCODColor::black);
   console.flush();
}

void Draw::showCommonSpellGUI() {
   using namespace std::string_literals;

   auto& console = *(TCODConsole::root);

   console.setDefaultForeground(TCODColor(180, 100, 0));
   for (int i = MENU_X+2; i <= MENU_X + 63; i++) {
      console.putChar(i, MENU_Y+1, 205, TCOD_BKGND_NONE);
      console.putChar(i, MENU_Y+3, 205, TCOD_BKGND_NONE);
      console.putChar(i, MENU_Y+33, 205, TCOD_BKGND_NONE);
      console.putChar(i, MENU_Y+13, 196, TCOD_BKGND_NONE);
      console.putChar(i, MENU_Y+23, 196, TCOD_BKGND_NONE);
   }
   for (int j = MENU_Y+2; j <= MENU_Y + 32; j++) {
      console.putChar(MENU_X+1, j, 186, TCOD_BKGND_NONE);
      console.putChar(MENU_X+22, j, 186, TCOD_BKGND_NONE);
      console.putChar(MENU_X+43, j, 186, TCOD_BKGND_NONE);
      console.putChar(MENU_X+64, j, 186, TCOD_BKGND_NONE);
   }
   console.putChar(MENU_X+1, MENU_Y+1, 201, TCOD_BKGND_NONE);
   console.putChar(MENU_X+64, MENU_Y+1, 187, TCOD_BKGND_NONE);
   console.putChar(MENU_X+1, MENU_Y+33, 200, TCOD_BKGND_NONE);
   console.putChar(MENU_X+64, MENU_Y+33, 188, TCOD_BKGND_NONE);
   console.putChar(MENU_X+22, MENU_Y+1, 203, TCOD_BKGND_NONE);
   console.putChar(MENU_X+43, MENU_Y+1, 203, TCOD_BKGND_NONE);
   console.putChar(MENU_X+22, MENU_Y+3, 206, TCOD_BKGND_NONE);
   console.putChar(MENU_X+43, MENU_Y+3, 206, TCOD_BKGND_NONE);
   console.putChar(MENU_X+22, MENU_Y+33, 202, TCOD_BKGND_NONE);
   console.putChar(MENU_X+43, MENU_Y+33, 202, TCOD_BKGND_NONE);
   console.putChar(MENU_X+1, MENU_Y+3, 204, TCOD_BKGND_NONE);
   console.putChar(MENU_X+64, MENU_Y+3, 185, TCOD_BKGND_NONE);

   // Draw the common spell text
   console.setDefaultForeground(TCODColor::lightBlue);
   console.print(MENU_X+10, MENU_Y+2, "Hero"s);
   console.print(MENU_X+18, MENU_Y+5, "( )"s);
   console.print(MENU_X+18, MENU_Y+15, "( )"s);
   console.print(MENU_X+18, MENU_Y+25, "( )"s);
   console.setDefaultForeground(TCODColor::red);
   console.print(MENU_X+29, MENU_Y+2, "Monster"s);
   console.print(MENU_X+39, MENU_Y+5, "( )"s);
   console.print(MENU_X+39, MENU_Y+15, "( )"s);
   console.print(MENU_X+39, MENU_Y+25, "( )"s);
   console.setDefaultForeground(TCODColor(156, 156, 156));
   console.print(MENU_X+51, MENU_Y+2, "World"s);
   console.print(MENU_X+60, MENU_Y+5, "( )"s);
   console.print(MENU_X+60, MENU_Y+15, "( )"s);
   console.print(MENU_X+60, MENU_Y+25, "( )"s);
}

void Draw::victoryScreen() {
   using namespace std::string_literals;
   auto& console = *(TCODConsole::root);

   TCODConsole::blit(backboard.get(), 0, 0, 66, 35, &console, MENU_X, MENU_Y);

   // Draw the borders
   console.setDefaultForeground(TCODColor(180, 100, 0));
   for (int i = MENU_X+2; i <= MENU_X + 63; i++) {
      console.putChar(i, MENU_Y+1, 205, TCOD_BKGND_NONE);
      console.putChar(i, MENU_Y+33, 205, TCOD_BKGND_NONE);
   }
   for (int i = MENU_X+5; i <= MENU_X + 60; i++) {
      console.putChar(i, MENU_Y+4, 205, TCOD_BKGND_NONE);
      console.putChar(i, MENU_Y+30, 205, TCOD_BKGND_NONE);
   }
   for (int j = MENU_Y+2; j <= MENU_Y + 32; j++) {
      console.putChar(MENU_X+1, j, 186, TCOD_BKGND_NONE);
      console.putChar(MENU_X+64, j, 186, TCOD_BKGND_NONE);
   }
   for (int j = MENU_Y+5; j <= MENU_Y + 29; j++) {
      console.putChar(MENU_X+4, j, 186, TCOD_BKGND_NONE);
      console.putChar(MENU_X+61, j, 186, TCOD_BKGND_NONE);
   }
   console.setDefaultBackground(TCODColor(210, 96, 0));
   for (int i = MENU_X+3; i<=MENU_X+63; i++) {
      console.putChar(i, MENU_Y+2, ' ', TCOD_BKGND_SET);
   }
   for (int i = MENU_X+4; i<=MENU_X+62; i++) {
      console.putChar(i, MENU_Y+3, ' ', TCOD_BKGND_SET);
   }
   for (int j = MENU_Y+2; j <= MENU_Y + 32; j++) {
      console.putChar(MENU_X+2, j, ' ', TCOD_BKGND_SET);
   }
   for (int j = MENU_Y+3; j <= MENU_Y + 31; j++) {
      console.putChar(MENU_X+3, j, ' ', TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor(84, 40, 0));
   for (int i = MENU_X+3; i<=MENU_X+63; i++) {
      console.putChar(i, MENU_Y+32, ' ', TCOD_BKGND_SET);
   }
   for (int i = MENU_X+4; i<=MENU_X+62; i++) {
      console.putChar(i, MENU_Y+31, ' ', TCOD_BKGND_SET);
   }
   for (int j = MENU_Y+3; j <= MENU_Y + 32; j++) {
      console.putChar(MENU_X+63, j, ' ', TCOD_BKGND_SET);
   }
   for (int j = MENU_Y+4; j <= MENU_Y + 31; j++) {
      console.putChar(MENU_X+62, j, ' ', TCOD_BKGND_SET);
   }

   console.putChar(MENU_X+4, MENU_Y+4, 201, TCOD_BKGND_NONE);
   console.putChar(MENU_X+61, MENU_Y+4, 187, TCOD_BKGND_NONE);
   console.putChar(MENU_X+4, MENU_Y+30, 200, TCOD_BKGND_NONE);
   console.putChar(MENU_X+61, MENU_Y+30, 188, TCOD_BKGND_NONE);
   console.putChar(MENU_X+1, MENU_Y+1, 201, TCOD_BKGND_NONE);
   console.putChar(MENU_X+64, MENU_Y+1, 187, TCOD_BKGND_NONE);
   console.putChar(MENU_X+1, MENU_Y+33, 200, TCOD_BKGND_NONE);
   console.putChar(MENU_X+64, MENU_Y+33, 188, TCOD_BKGND_NONE);
   console.setDefaultForeground(TCODColor::white);
   console.setAlignment(TCOD_CENTER);
   console.print(MENU_X+MAP_WIDTH/2-3, MENU_Y+7, "Congratulations!"s);
   console.setAlignment(TCOD_LEFT);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+10, "The hero rejoices over his victory of the dungeon,"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+11, "still completely unaware of your influence. You"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+12, "continue to safeguard the hero until he leaves the"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+13, "dungeon. With the magical artifacts gone and the"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+14, "monsters decimated, the evil aura of the dungeon"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+15, "diminishes. Soon the grass, vines and forest"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+16, "creatures will begin to encroach into the dungeon."s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+19, "The next greatest annoyance to the forest begins"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+20, "to grab your attention. The town, infested with"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+21, "rampaging heroes and greedy lumberjacks, must be"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+22, "the next target. It will take a few weeks before"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+23, "you can recruit enough wolves and bears, but you"s);
   console.print(MENU_X+MAP_WIDTH/2-28, MENU_Y+24, "know you will have no problem protecting them."s);
   console.print(MENU_X+MAP_WIDTH/2-7, MENU_Y+27, "THE END"s);

   console.setDefaultForeground(TCODColor::yellow);
   console.setDefaultBackground(TCODColor(78, 78, 78));
   {
      Utils::WithBackgroundSet set(console);
      console.print(MENU_X+40, MENU_Y+34, " Press ESC to quit "s);
   }
   console.flush();
}

void Draw::screen() {
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
      Utils::WithBackgroundSet set(console);
      console.print(LEFT+MAP_WIDTH/2-7, 1, " DungeonMinder "s); 
      console.printf(LEFT+MAP_WIDTH/2+25, 1, " Level %d ", state.level); 
   }
   // Draw the key instructions at the bottom

   Hero& hero = *(state.hero);

   // Draw the walls
   for (int i = 0; i < MAP_WIDTH; i++) {
      for (int j = 0; j < MAP_HEIGHT; j++) {
         const Tile& curTile = state.tileAt({i, j});

         int floorColour = static_cast<int>(floorNoise[i][j]*40);
         console.setDefaultBackground(TCODColor(floorColour, floorColour, floorColour));
         if (!hero.dead && state.isInFov(i, j)) {
            float intensity = 150.0f/std::pow((std::pow((i-hero.pos.x), 2)+std::pow((j-hero.pos.y),2)), 0.3);
            console.setDefaultBackground(TCODColor(static_cast<int>(floorColour+intensity), static_cast<int>(floorColour+intensity), floorColour));
         }
         if (state.map.cloud[i][j] > 0) {
            console.setDefaultBackground(TCODColor(100, 150, 100));
            if (curTile == Tile::BLANK) {
               console.setDefaultForeground(TCODColor(80, 100, 80));
               console.putChar(i+LEFT, j+TOP, 177, TCOD_BKGND_SET);
            } else {
               console.putChar(i+LEFT, j+TOP, ' ', TCOD_BKGND_SET);
            }
         }
         switch(curTile) {
            case Tile::WALL:
               console.setDefaultForeground(TCODColor(160-static_cast<int>(wallNoise[i][j]*30), 90+static_cast<int>(wallNoise[i][j]*30), 90+static_cast<int>(wallNoise[i][j]*30)));
               console.putChar(i+LEFT, j+TOP, 219, TCOD_BKGND_SET);
               break;
            case Tile::STAIRS_UP:
               console.setDefaultForeground(TCODColor(200, 200, 200));
               console.putChar(i+LEFT, j+TOP, '<', TCOD_BKGND_SET);
               break;
            case Tile::CHEST:
               console.setDefaultForeground(TCODColor::lightBlue);
               console.putChar(i+LEFT, j+TOP, 127, TCOD_BKGND_SET);
               break;
            case Tile::CHEST_OPEN:
               console.setDefaultForeground(TCODColor(128, 64, 0));
               console.putChar(i+LEFT, j+TOP, 127, TCOD_BKGND_SET);
               break;
            case Tile::TRAP:
               console.setDefaultForeground(TCODColor(100,150,100));
               console.putChar(i+LEFT, j+TOP, 207, TCOD_BKGND_SET);
               break;
            case Tile::FIELD:
               console.setDefaultForeground(TCODColor(50, 250, 200));
               console.putChar(i+LEFT, j+TOP, 177, TCOD_BKGND_NONE);
               break;
            case Tile::ILLUSION:
               console.setDefaultForeground(TCODColor(50,250,200));
               console.putChar(i+LEFT, j+TOP, 127, TCOD_BKGND_SET);
               break;
            case Tile::PORTAL:
               console.setDefaultForeground(TCODColor::red);
               console.putChar(i+LEFT, j+TOP, 245, TCOD_BKGND_SET);
               break;
            default:
               if (state.map.cloud[i][j] == 0) {
                  console.putChar(i+LEFT, j+TOP, ' ', TCOD_BKGND_SET);
               }
               break;
         }
      }
   }
   console.setDefaultBackground(TCODColor::black);

   // Show the stairs
   if (state.level < 10) {
      console.setDefaultForeground(TCODColor::white);
      console.putChar(hero.stairs.x+LEFT, hero.stairs.y-1+TOP, '>', TCOD_BKGND_NONE);
   }

   // Show the player
   const Position& playerPos = state.player.pos;
   console.setDefaultForeground(TCODColor::white);
   console.putChar(playerPos.x+LEFT, playerPos.y+TOP, TCOD_CHAR_LIGHT, TCOD_BKGND_NONE);

   // Show the hero
   if (hero.dead) {
      console.setDefaultForeground(TCODColor::black);
      console.setDefaultBackground(TCODColor::darkRed);
      console.putChar(hero.pos.x+LEFT, hero.pos.y+TOP, '@', TCOD_BKGND_SET);
   } else {
      console.setDefaultForeground(TCODColor(130-hero.health, (hero.health*12), (25*hero.health)));
      if (hero.shieldTimer > 0) {
         console.setDefaultBackground(TCODColor(50, 250, 200));
      } else {
         console.setDefaultBackground(TCODColor::lightYellow);
      }
      console.putChar(hero.pos.x+LEFT, hero.pos.y+TOP, '@', TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);

   // Show monsters
   for (const Monster& monster : state.monsterList) {
      // If the monster has spawned
      if (monster.portalTimer == 0) {
         console.setDefaultForeground(TCODColor(monster.health*100/monster.maxhealth+155, 155-(155*monster.health/monster.maxhealth), 155-(155*monster.health/monster.maxhealth)));
         console.putChar(monster.pos.x+LEFT, monster.pos.y+TOP, monster.symbol, TCOD_BKGND_NONE);
      }
   }

   // Display the stat line
   statLine(BOTTOM+2);

   // Print messages
   auto it = state.messageList.size() <= 8
      ? state.messageList.begin()
      : std::prev(state.messageList.end(), 8);
   for (int i = 0; it != state.messageList.end(); ++it, ++i) {
      console.setDefaultForeground(MESSAGE_COLOR.at(it->type));
      console.print(3, 50+i, it->text);
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

void Draw::showMessageHistory() {
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
      Utils::WithBackgroundSet set(console);
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
   statLine(28);

   // Display the messages
   int i = 0;
   for (auto it = state.messageList.begin(); it != state.messageList.end(); ++it, ++i) {
      console.setDefaultForeground(MESSAGE_COLOR.at(it->type));
      console.print(3, 30+i, it->text);
   }

   // Flush the root console
   console.flush();
}

void Draw::statLine(int row) {
   using namespace std::string_literals;

   auto& console = *(TCODConsole::root);
   Hero& hero = *(state.hero);

   // Print stats
   if (hero.dead) {
      console.setDefaultForeground(TCODColor::black);
      console.setDefaultBackground(TCODColor::darkRed);
      {
         Utils::WithBackgroundSet set(console);
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
   for (int i = 0; i < state.player.heroMana / MANA_BLIP_SIZE; ++i) {
      console.putChar(48+i, row, 224, TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);
   console.setDefaultForeground(TCODColor::red);
   console.putChar(55, row, 195, TCOD_BKGND_NONE);
   console.putChar(61, row, 180, TCOD_BKGND_NONE);
   console.setDefaultBackground(TCODColor::red);
   console.setDefaultForeground(TCODColor::black);
   for (int i = 0; i < state.player.monsterMana / MANA_BLIP_SIZE; ++i) {
      console.putChar(56+i, row, 224, TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);
   console.setDefaultForeground(TCODColor(156, 156, 156));
   console.putChar(63, row, 195, TCOD_BKGND_NONE);
   console.putChar(69, row, 180, TCOD_BKGND_NONE);
   console.setDefaultBackground(TCODColor(156, 156, 156));
   console.setDefaultForeground(TCODColor::black);
   for (int i = 0; i < state.player.worldMana / MANA_BLIP_SIZE; ++i) {
      console.putChar(64+i, row, 224, TCOD_BKGND_SET);
   }
   console.setDefaultBackground(TCODColor::black);
}

void Draw::generateMapNoise() {
   TCODNoise wallNoiseGen = TCODNoise(2, Utils::randGen);
   TCODNoise floorNoiseGen = TCODNoise(2, Utils::randGen);
   for (int j = 0; j < MAP_HEIGHT; j++) {
      for (int i = 0; i < MAP_WIDTH; i++) {
         float location[2] = {((float)i*2)/MAP_WIDTH-1, ((float)j*2)/MAP_HEIGHT-1};
         wallNoise[i][j] = (1+wallNoiseGen.get(location)/2);
         floorNoise[i][j] = (1+floorNoiseGen.get(location)/2);
      }
   }
}