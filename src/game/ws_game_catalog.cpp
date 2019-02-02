#include "ws_game_catalog.h"

#include <cstring>
#include <string>
#include <stdexcept>

#include "cartridge/ws_cartridge.h"
#include "sqlite/sqlite3.h"

using namespace std;

ws_game_catalog::ws_game_catalog(const char* db_file_name)
  : m_sqlite(nullptr)
{
  if (sqlite3_open_v2(db_file_name, &m_sqlite, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
  {
    sqlite3_close_v2(m_sqlite);
    throw std::runtime_error("Unable to open database");
  }
}

ws_game_catalog::~ws_game_catalog()
{
  sqlite3_close_v2(m_sqlite);
}

const game_descriptor* ws_game_catalog::identify_game(cartridge* cart, int slot_num)
{
  // Build hash from cartridge metadata
  // Match hash to cartridge in database
  //   If multiple matches, return first
  
  
  // Verify arguments
  if (cart->system() != system_type::SYSTEM_WONDERSWAN)
  {
    return nullptr;
  }
  if (slot_num == -1)
  {
    slot_num = 0;
  }
  
  // Fetch game metadata
  const ws_cartridge::game_metadata* metadata = ((ws_cartridge*) cart)->get_game_metadata(slot_num);
  if (metadata == nullptr)
  {
    return nullptr;
  }
  
  // Build hash
  long long hash = 0;
  hash |= ((long long) metadata->developer_id) << (7*8);
  hash |= ((long long) metadata->minimum_system) << (6*8);
  hash |= ((long long) metadata->game_id) << (5*8);
  hash |= ((long long) metadata->rom_size) << (4*8);
  hash |= ((long long) metadata->save_size) << (3*8);
  hash |= ((long long) metadata->flags) << (2*8);
  hash |= ((long long) metadata->checksum);
  
  // Query database for hash match in database and only get 1st matching result
  string query = "SELECT GameName, Developer FROM Games WHERE Hash=:hash LIMIT 1";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare(m_sqlite, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
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
  string developer_name = string((const char*) sqlite3_column_text(stmt, 1));
  game_descriptor* descriptor = new game_descriptor(game_name.c_str(), developer_name.c_str());
  descriptor->system = game_descriptor::game_system::WONDERSWAN;
  descriptor->num_bytes = ((ws_cartridge*) cart)->get_game_size(slot_num);
  
  sqlite3_finalize(stmt);
  return descriptor;
}
