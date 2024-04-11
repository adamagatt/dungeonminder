#ifndef __DRAW_HPP_
#define __DRAW_HPP_

#include "libtcod.hpp"

#include "game_state.hpp"

#include <memory>

class Draw {
   public:
   Draw(const GameState& state);

   void screen();
   void spellMenu();
   void upgradeMenu();
   void victoryScreen();
   void showMessageHistory();
   void statLine(int row);
   static void directionScreen();
   static void rangedAttack(int, int, int, int);
   static void rangedAttack(const Position& p1, const Position& p2);

   void generateMapNoise();

   private:
   void initialiseSpellMenu();
   void showCommonSpellGUI();
   static void directionFrame(int x, int y, int width, int height);

   std::unique_ptr<TCODConsole> backboard;
   const GameState& state;

   std::array<std::array<float, MAP_HEIGHT>, MAP_WIDTH> floorNoise;
   std::array<std::array<float, MAP_HEIGHT>, MAP_WIDTH> wallNoise;
};

#endif