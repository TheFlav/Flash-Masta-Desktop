# include "ngp-cart-row.h"
#include <sstream>

namespace ngpcart
{

string ngp_cart_row::insert_query() const
{
  stringstream query;
  
  query << "UPDATE Games SET "
    << "CartChips = :cartchips,"
    << "CartSize = :cartsize"
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
      query << ":gameid" << i << ","
        << ":blocknumber" << i << ","
        << ":chip" << i << ","
        << ":address" << i << ","
        << ":size" << i
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

bool ngp_cart_row::parse_xml(const node_t* node)
{
  // TODO
  (void) node;
  return false;
}

bool ngp_cart_row::bind_to_stmt(sqlite3_stmt* stmt)
{
  int ind;
  string param;
  
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
  
  for (int i = 0; i < SaveBlocks.size(); i++)
  {
    param = string(":gameid") + to_string(i);
    ind = sqlite3_bind_parameter_index(stmt, param.c_str());
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter '" << param << "'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, GameId) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter '" << param << "'" << endl;
      return false;
    }
    
    // block number
    param = string(":blocknumber") + to_string(i);
    ind = sqlite3_bind_parameter_index(stmt, param.c_str());
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter '" << param << "'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, SaveBlocks[i].BlockNumber) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter '" << param << "'" << endl;
      return false;
    }
    
    // chip
    param = string(":chip") + to_string(i);
    ind = sqlite3_bind_parameter_index(stmt, param.c_str());
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter '" << param << "'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, SaveBlocks[i].Chip) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter '" << param << "'" << endl;
      return false;
    }
    
    // address
    param = string(":address") + to_string(i);
    ind = sqlite3_bind_parameter_index(stmt, param.c_str());
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter '" << param << "'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, SaveBlocks[i].Address) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter '" << param << "'" << endl;
      return false;
    }
    
    // size
    param = string(":size") + to_string(i);
    ind = sqlite3_bind_parameter_index(stmt, param.c_str());
    if (ind == 0)
    {
      cerr << "Unable to find index of parameter '" << param << "'" << endl;
      return false;
    }
    if (sqlite3_bind_int(stmt, ind, SaveBlocks[i].Size) != SQLITE_OK)
    {
      cerr << "Unable to bind parameter '" << param << "'" << endl;
      return false;
    }
  }
  
  return true;
}

}
