#ifndef __ENTITY_HPP_
#define __ENTITY_HPP_

#include "position.hpp"

class Entity {
   public:
   Position pos;   
   int health;
   int timer;
};

#endif