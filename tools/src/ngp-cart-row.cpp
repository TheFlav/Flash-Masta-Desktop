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
  // TODO
  (void) stmt;
  return false;
}

}
