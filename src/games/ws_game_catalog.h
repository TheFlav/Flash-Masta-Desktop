#ifndef __WS_GAME_CATALOG_H__
#define __WS_GAME_CATALOG_H__

#include "game_catalog.h"

struct sqlite3;

class ws_game_catalog: public game_catalog
{
public:
  ws_game_catalog(const char* db_file_name);
  ~ws_game_catalog();
  
  const game_descriptor identify_game(cartridge* cart, int slot_num = -1);
  
private:
  bool verify_database_schema();
  
  sqlite3* m_sqlite;
};

#endif // defined(__WS_GAME_CATALOG_H__)
