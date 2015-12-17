#ifndef __NGP_GAME_CATALOG_H__
#define __NGP_GAME_CATALOG_H__

#include "game_catalog.h"

struct sqlite3;

class ngp_game_catalog : public game_catalog
{
public:
  ngp_game_catalog(const char* db_file_name);
  ~ws_game_catalog();
  
  const game_descriptor* identify_game(cartridge* cart, int slot_num =-1);
  
private:
  sqlite3* m_sqlite;
};

#endif // defined(__NGP_GAME_CATALOG_H__)
