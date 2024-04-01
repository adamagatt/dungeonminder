#ifndef __DRAW_HPP_
#define __DRAW_HPP_

#include "libtcod.hpp"

#include "game_state.hpp"

#include <memory>

class Draw {
   public:
   Draw(const GameState& state);

   void screen();
   void spellMenu(int heroSpec, int monsterSpec, int worldSpec);
   void upgradeMenu(int heroSpec, int monsterSpec, int worldSpec);
   void victoryScreen();
   void showMessageHistory();
   void statLine(int row);
   static void directionScreen();

   void generateMapNoise();

   private:
   void initialiseSpellMenu();
   void showCommonSpellGUI();
   static void directionFrame(int x, int y, int width, int height);

   std::unique_ptr<TCODConsole> backboard;
   const GameState& state;

   float floorNoise [MAP_WIDTH][MAP_HEIGHT];
   float wallNoise [MAP_WIDTH][MAP_HEIGHT];
};

#endif