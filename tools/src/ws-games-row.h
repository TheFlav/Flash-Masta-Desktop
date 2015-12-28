#ifndef __WS_GAMES_ROW_H__
#define __WS_GAMES_ROW_H__

namespace ws
{
class ws_games_row;
}

#include "games-row.h"

namespace ws
{
const string schema_file_name = "wsschema.sql";
const string data_file_name = "wsgames.xml";
const string db_file_name = "wsgames.db";

class ws_games_row : public games_row
{
public:
              ~ws_games_row() {};
  std::string insert_query() const;
  bool        parse_xml(const node_t* node, sqlite3* db);
  bool        bind_to_stmt(sqlite3_stmt* stmt, int query);
  
  int ID;
  long long Hash;
  int GameID;
  string GameName;
  string Developer;
  int RomSize;
  int SaveSize;
  int MinSystem;
  int MapperVersion;
  int RTC;
  int Checksum;
  int Flags;
};

}

#endif
