#include "ngp_game_catalog.h"

#include <cstring>
#include <string>
#include <stdexcept>

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
  if (slot_num == -1)
  {
    slot_num = 0;
  }
  
  // Fetch game metadata
  const ngp_cartridge::game_metadata* metadata = ((ngp_cartridge*) cart)->get_game_metadata(slot_num);
  if (metadata == nullptr)
  {
    return nullptr;
  }
  
  // Build hash
  long long hash = 0;
  hash |= ((long long) metadata->startup_address) << (4 * 8);
  hash |= ((long long) metadata->game_id) << (2 * 8);
  hash |= ((long long) metadata->game_version) << (1 * 8);
  hash |= ((long long) metadata->minimum_system);
  
  // Query database for hash match in database and only get first matching result
  string query = "SELECT GameName, CartSize FROM Games WHERE `Hash`=:hash LIMIT 1";
  sqlite3_stmt* stmt = nullptr;
  int error = sqlite3_prepare_v2(m_sqlite, query.c_str(), -1, &stmt, nullptr);
  if (error != SQLITE_OK)
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
  game_descriptor* descriptor = new game_descriptor(game_name.c_str(), "");
  descriptor->system = game_descriptor::game_system::NEO_GEO_POCKET;
  descriptor->num_bytes = (sqlite3_column_type(stmt, 1) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 1)) << 17;
  
  sqlite3_finalize(stmt);
  return descriptor;
}
