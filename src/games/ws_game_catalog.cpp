#include "ws_game_catalog.h"

#include <cstring>

#include "cartridge/cartridge.h"
#include "sqlite/sqlite3.h"

ws_game_catalog::ws_game_catalog(const char* db_file_name)
  : m_sqlite(nullptr)
{
  sqlite3_open_v2(db_file_name, &m_sqlite, SQLITE_OPEN_READONLY, "");
  
  if (!verify_database_schema())
  {
    // TODO Throw an error
  }
}

ws_game_catalog::~ws_game_catalog()
{
  sqlite3_close_v2(m_sqlite);
}

const game_descriptor ws_game_catalog::identify_game(cartridge* cart, int slot_num)
{
  // Build hash from cartridge metadata
  // Match hash to cartridge in database
  //   If multiple matches, return first?
  
  (void) cart;
  (void) slot_num;
  return game_descriptor("","");
}

bool ws_game_catalog::verify_database_schema()
{
  return true;
  
  /*
  bool good = true;
  sqlite3_stmt* query;
  
  // Make sure necessary tables exist
  if (good)
  {
    sqlite3_prepare_v2(m_sqlite, "SELECT NULL FROM sqlite_master WHERE type='table' AND name='Games'", -1, &query, nullptr);
    if (sqlite3_step(query) == SQLITE_DONE)
    {
      good = false;
    }
  }
  
  // Verify table schema
  if (good)
  {
    sqlite3_prepare_v2(m_sqlite, "SELECT * FROM Games LIMIT 1", -1, &query, nullptr);
    if (sqlite3_step(query) == SQLITE_DONE) good = false;
    else if (strcmp(sqlite3_column_name(query, 0), "ID") != 0) good = false;
    else if (strcmp(sqlite3_column_name(query, 1), "Name") != 0) good = false;
  }
  
  // Cleanup
  sqlite3_finalize(query);
  
  return good;
  */
}

