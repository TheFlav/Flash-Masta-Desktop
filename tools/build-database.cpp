
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>

#include "../src/sqlite/sqlite3.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

typedef xml_node<> node_t;
typedef xml_document<> doc_t;

#define CHECK_ERROR(database) \
{\
  int error = sqlite3_errcode(database);\
  if (error != SQLITE_OK && error != SQLITE_ROW && error != SQLITE_DONE)\
  {\
    cout << sqlite3_errmsg(database) << endl;\
    sqlite3_close(database);\
    exit(error);\
  }\
}

bool execute_file(ifstream& fin, sqlite3* db);
bool add_games_to_db(sqlite3* db, const doc_t* games_xml);

int main(const int argc, const char** const argv)
{
  // Declare some variables we're going to be using a lot of
  sqlite3* db;
  doc_t games_xml;
  
  // Open connection to database file
  sqlite3_open_v2("wsgames.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  CHECK_ERROR(db);
  
  // Open and execute schema sql file
  ifstream f_schema("wsschema.sql");
  if (!f_schema)
  {
    cerr << "Unable to open schema file 'wsschema.sql'" << endl;
    return 1;
  }
  if (!execute_file(f_schema, db))
  {
    return 1;
  }
  f_schema.close();
  
  // Open and parse game information xml
  file<> games_file("wsgames.xml");
  games_xml.parse<0>(games_file.data());
  
  // Close database connection
  sqlite3_close(db);
  return 0;
}



struct games_row
{
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
  
  static games_row* from_xml(const node_t* node);
  static string insert_query();
  bool   bind_to_stmt(sqlite3_stmt* stmt);
};

games_row* games_row::from_xml(const node_t* node)
{
  bool success = true;
  games_row* row = new games_row;
  node_t* n;
  
  // Game name
  if (success)
  {
    n = node->first_node("NAME");
    if (n == nullptr) success = false;
    else row->GameName = string(n->value());
  }
  
  // Clean up in case of error
  if (!success)
  {
    delete row;
    row = nullptr;
  }
  
  return row; // TODO
}

string games_row::insert_query()
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
    ":mapperverion,"
    ":rtc,"
    ":checksum,"
    ":flags"
    ")");
}

bool games_row::bind_to_stmt(sqlite3_stmt* stmt)
{
  int ind;
  
  ind = sqlite3_bind_parameter_index(stmt, "hash");
  if (ind == 0) return false;
  if (sqlite3_bind_int64(stmt, ind, Hash) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "gameid");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, GameID) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "gamename");
  if (ind == 0) return false;
  if (sqlite3_bind_text(stmt, ind, GameName.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_DONE) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "developer");
  if (ind == 0) return false;
  if (sqlite3_bind_text(stmt, ind, Developer.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_DONE) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "romsize");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, RomSize) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "savesize");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, SaveSize) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "minsystem");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, MinSystem) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "mapperversion");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, MapperVersion) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "rtc");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, RTC) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "checksum");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, Checksum) != SQLITE_OK) return false;
  
  ind = sqlite3_bind_parameter_index(stmt, "flags");
  if (ind == 0) return false;
  if (sqlite3_bind_int(stmt, ind, Flags) != SQLITE_OK) return false;
  
  return true;
}



bool execute_file(ifstream& fin, sqlite3* db)
{
  bool success = true;
  string query;
  
  // Read file into query string
  fin.seekg(0, ios::end);
  query.resize(fin.tellg());
  fin.seekg(0, ios::beg);
  fin.read(&query[0], query.size());
  
  // Execute string until all queries are complete
  const char* c = &(query.c_str()[0]);
  while (success && c != &(query.c_str()[query.size()]))
  {
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(db, c, -1, &statement, &c);
    sqlite3_step(statement);
    
    // Check for query errors
    int errcode = sqlite3_errcode(db);
    if (errcode != SQLITE_OK && errcode != SQLITE_ROW && errcode != SQLITE_DONE)
    {
      cerr << sqlite3_errcode(db) << ": " << sqlite3_errmsg(db) << endl;
      success = false;
    }
    
    sqlite3_finalize(statement);
  }
  
  return success;
}

bool add_games_to_db(sqlite3* db, const doc_t* games_xml)
{
  bool success = true;
  
  // Construct statement that uses bound parameters
  string query = games_row::insert_query();
  sqlite3_stmt* stmt;
  sqlite3_prepare(db, query.c_str(), -1, &stmt, nullptr);
  
  // Check for statement errors
  int errcode = sqlite3_errcode(db);
  if (errcode != SQLITE_OK && errcode != SQLITE_ROW && errcode != SQLITE_DONE)
  {
    cerr << sqlite3_errcode(db) << ": " << sqlite3_errmsg(db) << endl;
    success = false;
  }
  
  // Parse each ROMINFO node and add to table
  node_t* rominfo_node = games_xml->first_node()->first_node();
  while (success && rominfo_node != nullptr)
  {
    // Create struct from xml data
    games_row* row = games_row::from_xml(rominfo_node);
    if (row == nullptr)
    {
      success = false;
    }
    
    // Bind parameters to statement
    if (success) if (!row->bind_to_stmt(stmt))
    {
      success = false;
    }
    
    // Insert into database
    if (success) sqlite3_reset(stmt);
    if (success) sqlite3_step(stmt);
    
    // Check for query errors
    int errcode = sqlite3_errcode(db);
    if (success) if (errcode != SQLITE_OK && errcode != SQLITE_ROW && errcode != SQLITE_DONE)
    {
      cerr << sqlite3_errcode(db) << ": " << sqlite3_errmsg(db) << endl;
      success = false;
    }
    
    // Clean up
    if (row != nullptr) delete row;
    
    // Move to next node
    if (success) rominfo_node = rominfo_node->next_sibling();
  }
  
  // Clean up prepared statement
  sqlite3_finalize(stmt);
  
  return success;
}
