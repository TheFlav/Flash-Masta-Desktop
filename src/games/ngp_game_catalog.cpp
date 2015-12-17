#include "ngp_game_catalog.h"

#include <cstring>
#include <string>

#include "cartridge/ngp_cartridge.h"
#include "sqlite/sqlite3.h"

using namespace std;

ngp_game_catalog::ngp_game_catalog(const char* db_file_name)
  : m_sqlite(nullptr)
{
  if (sqlite3_open_v2(db_file_name, &m_sqlite, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
  {
    sqlite3_close_v2(m_sqlite);
    throw std::runtime_error("Unable to open database");
  }
}

ngp_game_catalog::~ngp_game_catalog()
{
  sqlite3_close_v2(m_sqlite);
}

const game_descriptor* ngp_game_catalog::identify_game(cartridge* cart, int slot_num)
{
  // Build hash from cartridge metadata
  // Match hash to game in database
  //   If multiple matches, return first
  
  
  // Verify arguments
  if (cart->system() != system_type::SYSTEM_NEO_GEO_POCKET)
  {
    return nullptr;
  }
  
  // TODO Fetch game metadata
  
  // TODO Build hash
  
  // Query database for hash match in database and only get first matching result
  string query = "SELECT GameName, CartName FROM Games WHERE Hash=:hash LIMIT 1";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare(msqlite, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
  {
    return nullptr;
  }
  
  if (sqlite3_bind_int64(stmt, sqlite3_bind_parameter_index(stmt, ":hash"), hash) != SQLITE_OK
      || sqlite3_step(stmt) != SQLITE_ROW)
  {
    sqlite3_finalize(stmt);
    return nullptr;
  }
  
  // Build descriptor from database result set
  string game_name = string((const char*) sqlite3_column_text(stmt, 0));
  string cart_name = string((const char*) sqlite3_column_text(stmt, 1));
  // TODO Build descriptor
  
  sqlite3_finalize(stmt);
  return descriptor;
}
