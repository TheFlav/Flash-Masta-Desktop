#include "ws_game_catalog.h"

#include "cartridge/cartridge.h"
#include "sqlite/sqlite3.h"

ws_game_catalog::ws_game_catalog(const char* db_file_name)
  : m_sqlite(nullptr)
{
  (void) db_file_name;
  (void) m_sqlite;
}

ws_game_catalog::~ws_game_catalog()
{
  // Nothing else to do
}

const game_descriptor ws_game_catalog::identify_game(cartridge* cart, int slot_num)
{
  (void) cart;
  (void) slot_num;
  return game_descriptor("","");
}

