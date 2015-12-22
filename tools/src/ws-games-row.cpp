#include "ws-games-row.h"

namespace ws
{

string ws_games_row::insert_query() const
{
  return string("INSERT INTO Games ("
    "`Hash`,"
    "GameID,"
    "GameName,"
    "Developer,"
    "RomSize,"
    "SaveSize,"
    "MinSystem,"
    "MapperVersion,"
    "RTC,"
    "`Checksum`,"
    "Flags"
    ") VALUES ("
    ":hash,"
    ":gameid,"
    ":gamename,"
    ":developer,"
    ":romsize,"
    ":savesize,"
    ":minsystem,"
    ":mapperversion,"
    ":rtc,"
    ":checksum,"
    ":flags"
    ")");
}

bool ws_games_row::parse_xml(const node_t* node, sqlite3* db)
{
  (void) db;
  
  bool success = true;
  node_t* n;
  
  // Game name
  if (success)
  {
    node_t* n_ = node->first_node("NAME");
    if (n_ != nullptr && n_->first_node() != nullptr) n_ = n_->first_node(); // handles fields with CDATA
    if (n_ == nullptr)
    {
      cerr << "Missing XML element NAME" << endl;
      success = false;
    }
    else GameName = string(n_->value());
  }
  
  // Metadata and hash
  if (success)
  {
    Hash = 0;
    node = node->first_node("METADATA");
    if (node == nullptr)
    {
      cerr << "Missing XML element METADATA" << endl;
      success = false;
    }
    else n = node->first_node("HEX");
    if (success) if (n == nullptr)
    {
      cerr << "MISSING XML element HEX" << endl;
      success = false;
    }
  }
  
  // Developer ID
  if (success)
  {
    node_t* n_ = n->first_node("HEX0");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX0" << endl;
      success = false;
    }
    else Hash |= (std::strtol(n_->value(), 0, 16) & 0xFF) << (7*8);
  }
  
  // Minimum system
  if (success)
  {
    node_t* n_ = n->first_node("HEX1");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX1" << endl;
      success = false;
    }
    else
    {
      MinSystem = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= ((long long) MinSystem) << (6*8);
    }
  }
  
  // Game ID
  if (success)
  {
    node_t* n_ = n->first_node("HEX2");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX2" << endl;
      success = false;
    }
    else
    {
      GameID = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= ((long long) GameID) << (5*8);
    }
  }
  
  // Mapper version
  if (success)
  {
    node_t* n_ = n->first_node("HEX3");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX3" << endl;
      success = false;
    }
    else MapperVersion = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
  }
  
  // ROM size
  if (success)
  {
    node_t* n_ = n->first_node("HEX4");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX4" << endl;
      success = false;
    }
    else
    {
      RomSize = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= ((long long) RomSize) << (4*8);
    }
  }
  
  // Save size/type
  if (success)
  {
    node_t* n_ = n->first_node("HEX5");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX5" << endl;
      success = false;
    }
    else
    {
      SaveSize = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= ((long long) SaveSize) << (3*8);
    }
  }
  
  // Flags
  if (success)
  {
    node_t* n_ = n->first_node("HEX6");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX6" << endl;
      success = false;
    }
    else
    {
      Flags = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= ((long long) Flags) << (2*8);
    }
  }
  
  // RTC
  if (success)
  {
    node_t* n_ = n->first_node("HEX7");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX7" << endl;
      success = false;
    }
    else RTC = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
  }
  
  // Checksum 1
  if (success)
  {
    node_t* n_ = n->first_node("HEX8");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX8" << endl;
      success = false;
    }
    else Checksum = (int) (std::strtol(n_->value(), 0, 16) & 0xFF) << 8;
  }
  
  // Checksum 2
  if (success)
  {
    node_t* n_ = n->first_node("HEX9");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX9" << endl;
      success = false;
    }
    else
    {
      Checksum |= (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= (long long) Checksum;
    }
  }
  
  // Developer name
  if (success)
  {
    node_t* n_ = node->first_node("DEVELOPER_ID");
    if (n_ != nullptr && n_->first_node() != nullptr) n_ = n_->first_node(); // handles fields with CDATA
    if (n_ == nullptr)
    {
      cerr << "Missing XML element DEVELOPER_ID" << endl;
      success = false;
    }
    else Developer = string(n_->value());
  }
  
  
  
  return success;
}

bool ws_games_row::bind_to_stmt(sqlite3_stmt* stmt, int query)
{
  int ind;
  
  switch (query)
  {
  case 0:
    ind = sqlite3_bind_parameter_index(stmt, ":hash");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'hash'" << endl;
      return false;
    }
    if (sqlite3_bind_int64(stmt, ind, Hash) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'hash'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":gameid");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'gameid'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, GameID) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'gameid'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":gamename");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'gamename'" << endl;
      return false;
    }
    if (sqlite3_bind_text(stmt, ind, GameName.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'gamename'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":developer");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'developer'" << endl;
      return false;
    }
    if (sqlite3_bind_text(stmt, ind, Developer.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'developer'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":romsize");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'romsize'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, RomSize) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'romsize'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":savesize");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'savesize'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, SaveSize) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'savesize'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":minsystem");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'minsystem'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, MinSystem) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'minsystem'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":mapperversion");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'mapperversion'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, MapperVersion) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'mapperversion'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":rtc");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'rtc'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, RTC) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'rtc'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":checksum");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'checksum'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, Checksum) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'checksum'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":flags");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'flags'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, Flags) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'flags'" << endl;
      return false;
    }
    break;
    
  default:
    // do nothing
    break;
  }
  
  return true;
}

}
