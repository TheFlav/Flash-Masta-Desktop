#include "ngp-games-row.h"

namespace ngp
{

string ngp_games_row::insert_query() const
{
  return string("INSERT INTO Games ("
    "`Hash`,"
    "GameID,"
    "GameVersion,"
    "StartupAddress,"
    "MinSystem,"
    "License,"
    "CartName,"
    "GameName"
    ") VALUES ("
    ":hash,"
    ":gameid,"
    ":gameversion,"
    ":startupaddress,"
    ":minsystem,"
    ":license,"
    ":cartname,"
    ":gamename"
    ")");
}

bool ngp_games_row::parse_xml(const node_t* node, sqlite3* db)
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
  
  // Metadata & hash
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
  
  // Cartridge name text
  if (success)
  {
    node_t* n_ = node->first_node("ROM_NAME");
    if (n_ != nullptr && n_->first_node() != nullptr) n_ = n_->first_node(); // handles fields with CDATA
    if (n_ == nullptr)
    {
      cerr << "Missing XML element ROM_NAME" << endl;
      success = false;
    }
    else CartName = string(n_->value());
  }
  
  // License text
  if (success)
  {
    node_t* n_ = node->first_node("LICENSE_TEXT");
    if (n_ != nullptr && n_->first_node() != nullptr) n_ = n_->first_node(); // handles fields with CDATA
    if (n_ == nullptr)
    {
      cerr << "Missing XML element LICENSE_TEXT" << endl;
      success = false;
    }
    else License = string(n_->value());
  }
  
  // Starting address 1
  if (success)
  {
    node_t* n_ = n->first_node("HEX28");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX28" << endl;
      success = false;
    }
    else StartupAddress = (std::strtol(n_->value(), 0, 16) & 0xFF);
  }
  
  // Starting address 2
  if (success)
  {
    node_t* n_ = n->first_node("HEX29");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX29" << endl;
      success = false;
    }
    else StartupAddress |= (std::strtol(n_->value(), 0, 16) & 0xFF) << 8;
  }
  
  // Starting address 3
  if (success)
  {
    node_t* n_ = n->first_node("HEX30");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX30" << endl;
      success = false;
    }
    else StartupAddress |= (std::strtol(n_->value(), 0, 16) & 0xFF) << (8 * 2);
  }
  
  // Starting address 4 & hash
  if (success)
  {
    node_t* n_ = n->first_node("HEX31");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX31" << endl;
      success = false;
    }
    else
    {
      StartupAddress |= (std::strtol(n_->value(), 0, 16) & 0xFF) << (8 * 3);
      Hash |= (((long long) StartupAddress) & 0xFFFFFFFF) << (8 * 4);
    }
  }
  
  // Game ID 1
  if (success)
  {
    node_t* n_ = n->first_node("HEX32");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX32" << endl;
      success = false;
    }
    else GameID = (std::strtol(n_->value(), 0, 16) & 0xFF);
  }
  
  // Game ID 2 & hash
  if (success)
  {
    node_t* n_ = n->first_node("HEX33");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX33" << endl;
      success = false;
    }
    else
    {
      GameID |= (std::strtol(n_->value(), 0, 16) & 0xFF) << 8;
      Hash |= (((long long) GameID) & 0xFFFF) << (8 * 2);
    }
  }
  
  // Game version & hash
  if (success)
  {
    node_t* n_ = n->first_node("HEX34");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX34" << endl;
      success = false;
    }
    else
    {
      GameVersion = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= (((long long) GameVersion) & 0xFF) << 8;
    }
  }
  
  // Minimum system & hash
  if (success)
  {
    node_t* n_ = n->first_node("HEX35");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX35" << endl;
      success = false;
    }
    else
    {
      MinSystem = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      Hash |= ((long long) MinSystem) & 0xFF;
    }
  }
  
  return success;
}

bool ngp_games_row::bind_to_stmt(sqlite3_stmt* stmt, int query)
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

    ind = sqlite3_bind_parameter_index(stmt, ":gameversion");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'gameversion'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, GameVersion) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'gameversion'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":startupaddress");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'startupaddress'" << endl;
      return false;
    }
    if (sqlite3_bind_int64(stmt, ind, (long long) StartupAddress) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'startupaddress'" << endl;
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

    ind = sqlite3_bind_parameter_index(stmt, ":license");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'license'" << endl;
      return false;
    }
    if (sqlite3_bind_text(stmt, ind, License.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'license'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":cartname");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'cartname'" << endl;
      return false;
    }
    if (sqlite3_bind_text(stmt, ind, CartName.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'cartname'" << endl;
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
    break;
    
  default:
    // Do nothing
    break;
  }
  
  return true;
}

}
