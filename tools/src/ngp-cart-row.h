#ifndef __NGP_CART_ROW__
#define __NGP_CART_ROW__

namespace ngpcart
{
class ngp_cart_row;
}

#include "games-row.h"
#include <vector>

namespace ngpcart
{
const string schema_file_name = "ngpcartschema.sql";
const string data_file_name = "ngpcartinfo.xml";
const string db_file_name = "ngpgames.db";

class ngp_cart_row : public games_row
{
public:
              ~ngp_cart_row() {};
  std::string insert_query() const;
  bool        parse_xml(const node_t* node);
  bool        bind_to_stmt(sqlite3_stmt* stmt);
  
  struct SaveBlock {
    int  BlockNumber;
    int  Chip;
    long Address;
    long Size;
  };
  
  long GameId;
  long CartChips;
  long CartSize;
  std::vector<SaveBlock> SaveBlocks;
};

}

#endif
