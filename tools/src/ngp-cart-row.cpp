#include "ngp-cart-row.h"
#include <sstream>

namespace ngpcart
{

string ngp_cart_row::insert_query() const
{
  stringstream query;
  
  query << "UPDATE Games SET "
    << "CartChips=:cartchips,"
    << "CartSize=:cartsize"
    << " WHERE "
    << "ID = :gameid;";
  
  if (!SaveBlocks.empty())
  {
    query << "INSERT INTO SaveBlocks ("
      << "GameID,"
      << "BlockNumber,"
      << "Chip,"
      << "Address,"
      << "Size"
      << ") VALUES (";

    for (int i = 0; i < SaveBlocks.size(); i++)
    {
      query << "?,"
        << "?,"
        << "?,"
        << "?,"
        << "?"
        << ")";

      if (i < SaveBlocks.size() - 1)
      {
        query << ",(";
      }
      else
      {
        query << ";";
      }
    }
  }
  
  return query.str();
}

bool ngp_cart_row::parse_xml(const node_t* node, sqlite3* db)
{
  bool success = true;
  node_t* n;
  
  // Cartridge size
  if (success)
  {
    node_t* n_ = node->first_node("CART_MBITS");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element CART_MBITS" << endl;
      success = false;
    }
    else CartSize = std::strtol(n_->value(), 0, 10);
  }
  
  // Number of chips
  if (success)
  {
    node_t* n_ = node->first_node("CART_NUM_CHIPS");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element CART_NUM_CHIPS" << endl;
      success = false;
    }
    else CartChips = std::strtol(n_->value(), 0, 10);
  }
  
  // Chips
  if (success)
  {
    SaveBlocks.clear();
    for (node_t* n_ = node->first_node("CHIP");
         success && n_ != nullptr;
         n_ = n_->next_sibling("CHIP"))
    {
      int chip_index = std::strtol(n_->first_attribute("INDEX")->value(), 0, 10);
      
      // Unprotected blocks
      for (node_t* n__ = n_->first_node("UNPROTECTED_BLOCK");
           success && n__ != nullptr;
           n__ = n__->next_sibling("UNPROTECTED_BLOCK"))
      {
        SaveBlock block;
        block.Chip = chip_index;
        
        if (success)
        {
          node_t* n___ = n__->first_node("NUM");
          if (n___ == nullptr)
          {
            cerr << "Missing XML element NUM" << endl;
            success = false;
          }
          else block.BlockNumber = std::strtol(n___->value(), 0, 10);
        }
        
        if (success)
        {
          node_t* n___ = n__->first_node("ADDRESS");
          if (n___ == nullptr)
          {
            cerr << "Missing XML element ADDRESS" << endl;
            success = false;
          }
          else block.Address = std::strtol(n___->value(), 0, 16);
        }
        
        if (success)
        {
          node_t* n___ = n__->first_node("SIZE");
          if (n___ == nullptr)
          {
            cerr << "Missing XML element SIZE" << endl;
            success = false;
          }
          else block.Size = std::strtol(n___->value(), 0, 16);
        }
        
        if (success)
        {
          SaveBlocks.push_back(block);
        }
      }
    }
  }
  
  
  
  // Cartridge game ID
  int game_id;
  if (success)
  {
    node_t* n_ = node->first_node("GAME_ID");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element GAME_ID" << endl;
      success = false;
    }
    else game_id = std::strtol(n_->value(), 0, 16) & 0xFFFF;
  }
  
  // Cartridge game version
  int game_ver;
  if (success)
  {
    node_t* n_ = node->first_node("GAME_VER");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element GAME_VER" << endl;
      success = false;
    }
    else game_ver = std::strtol(n_->value(), 0, 16) & 0xFF;
  }
  
  // Game ID
  if (success)
  {
    int ind;
    sqlite3_stmt* stmt;
    
    string query = "SELECT ID FROM Games WHERE GameID=:gameid AND GameVersion=:gamever LIMIT 1";
    if (success) if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
      cerr << "Unable to prepare statement" << endl;
      success = false;
    }
    
    ind = sqlite3_bind_parameter_index(stmt, ":gameid");
    if (success) if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'gameid'" << endl;
      success = false;
    }
    if (success) if (sqlite3_bind_int(stmt, ind, game_id) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'gameid'" << endl;
      success = false;
    }
    
    ind = sqlite3_bind_parameter_index(stmt, ":gamever");
    if (success) if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'gamever'" << endl;
      success = false;
    }
    if (success) if (sqlite3_bind_int(stmt, ind, game_ver) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'gamever'" << endl;
      success = false;
    }
    
    if (success) if (sqlite3_step(stmt) != SQLITE_ROW)
    {
      cerr << "No results for game_id " << game_id << " and game_ver " << game_ver << endl;
      success = false;
    }
    
    if (success) GameId = sqlite3_column_int(stmt, 0);
    
    sqlite3_finalize(stmt);
  }
  
  return success;
}

bool ngp_cart_row::bind_to_stmt(sqlite3_stmt* stmt, int query)
{
  int ind;
  string param;
  
  switch (query)
  {
  case 0:
    ind = sqlite3_bind_parameter_index(stmt, ":cartchips");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'cartchips'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, CartChips) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'cartchips'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":cartsize");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'cartsize'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, CartSize) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'cartsize'" << endl;
      return false;
    }

    ind = sqlite3_bind_parameter_index(stmt, ":gameid");
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter 'gameid'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, GameId) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter 'gameid'" << endl;
      return false;
    }
    break;
    
  case 1:
    ind = 0;
    for (int i = 0; i < SaveBlocks.size(); i++)
    {
      if (sqlite3_bind_int(stmt, ++ind, GameId) != SQLITE_OK)
      {
        cerr << "Unable to bind GameId to parameter " << ind << endl;
        return false;
      }

      // block number
      if (sqlite3_bind_int(stmt, ++ind, SaveBlocks[i].BlockNumber) != SQLITE_OK)
      {
        cerr << "Unable to bind BlockNumber to parameter " << ind << endl;
        return false;
      }

      // chip
      if (sqlite3_bind_int(stmt, ++ind, SaveBlocks[i].Chip) != SQLITE_OK)
      {
        cerr << "Unable to bind Chip to parameter " << ind << endl;
        return false;
      }

      // address
      if (sqlite3_bind_int(stmt, ++ind, SaveBlocks[i].Address) != SQLITE_OK)
      {
        cerr << "Unable to bind Address to parameter " << ind << endl;
        return false;
      }

      // size
      if (sqlite3_bind_int(stmt, ++ind, SaveBlocks[i].Size) != SQLITE_OK)
      {
        cerr << "Unable to bind Size to parameter " << ind << endl;
        return false;
      }
    }
    break;
    
  default:
    // do nothing
    break;
  }
  
  return true;
}

}
