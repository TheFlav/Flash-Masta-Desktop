#ifndef __NGP_GAMES_ROW__
#define __NGP_GAMES_ROW__

namespace ngp
{
class ngp_games_row;
}

#include "games-row.h"

namespace ngp
{
const string schema_file_name = "ngpschema.sql";
const string data_file_name = "ngpgames.xml";
const string db_file_name = "ngpgames.db";

class ngp_games_row : public games_row
{
public:
              ~ngp_games_row() {};
  std::string insert_query() const;
  bool        parse_xml(const node_t* node, sqlite3* db);
  bool        bind_to_stmt(sqlite3_stmt* stmt, int query);
  
  int ID;
  long long Hash;
  int GameID;
  int GameVersion;
  long StartupAddress;
  int MinSystem;
  string License;
  string CartName;
  string GameName;
};

}

#endif
