#ifndef __GAMES_ROW_H__
#define __GAMES_ROW_H__

class games_row;

#include "build-database.h"

class games_row
{
public:
  virtual             ~games_row() {};
  virtual std::string insert_query() const = 0;
  virtual bool        parse_xml(const node_t* node, sqlite3* db) = 0;
  virtual bool        bind_to_stmt(sqlite3_stmt* stmt, int query) = 0;
};

#endif
