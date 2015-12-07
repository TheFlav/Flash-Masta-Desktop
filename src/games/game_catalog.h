#ifndef __GAME_CATALOG_H__
#define __GAME_CATALOG_H__

#include "game_descriptor.h"

class cartridge;

class game_catalog
{
public:
  virtual ~game_catalog() {}
  virtual const game_descriptor* identify_game(cartridge* cart, int slot_num = -1) = 0;
};

#endif // defined(__GAME_CATALOG_H__)
