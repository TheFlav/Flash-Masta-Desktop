
#include <iostream>
#include <iomanip>
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
  
  // Add games to database
  if (!add_games_to_db(db, &games_xml))
  {
    cerr << "An error occured while adding games to database" << endl;
  }
  
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
    if (n == nullptr)
    {
      cerr << "Missing XML element NAME" << endl;
      success = false;
    }
    else row->GameName = string(n->value());
  }
  
  // Metadata and hash
  if (success)
  {
    row->Hash = 0;
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
  
  // Developer ID
  if (success)
  {
    node_t* n_ = n->first_node("HEX0");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX0" << endl;
      success = false;
    }
    else row->Hash |= (std::strtol(n_->value(), 0, 16) & 0xFF) << (7*8);
  }
  
  // Minimum system
  if (success)
  {
    node_t* n_ = n->first_node("HEX1");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX1" << endl;
      success = false;
    }
    else
    {
      row->MinSystem = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      row->Hash |= ((long long) row->MinSystem) << (6*8);
    }
  }
  
  // Game ID
  if (success)
  {
    node_t* n_ = n->first_node("HEX2");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX2" << endl;
      success = false;
    }
    else
    {
      row->GameID = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      row->Hash |= ((long long) row->GameID) << (5*8);
    }
  }
  
  // Mapper version
  if (success)
  {
    node_t* n_ = n->first_node("HEX3");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX3" << endl;
      success = false;
    }
    else row->MapperVersion = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
  }
  
  // ROM size
  if (success)
  {
    node_t* n_ = n->first_node("HEX4");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX4" << endl;
      success = false;
    }
    else
    {
      row->RomSize = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      row->Hash |= ((long long) row->RomSize) << (4*8);
    }
  }
  
  // Save size/type
  if (success)
  {
    node_t* n_ = n->first_node("HEX5");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX5" << endl;
      success = false;
    }
    else
    {
      row->SaveSize = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      row->Hash |= ((long long) row->SaveSize) << (3*8);
    }
  }
  
  // Flags
  if (success)
  {
    node_t* n_ = n->first_node("HEX6");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX6" << endl;
      success = false;
    }
    else
    {
      row->Flags = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      row->Hash |= ((long long) row->Flags) << (2*8);
    }
  }
  
  // RTC
  if (success)
  {
    node_t* n_ = n->first_node("HEX7");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX7" << endl;
      success = false;
    }
    else row->RTC = (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
  }
  
  // Checksum 1
  if (success)
  {
    node_t* n_ = n->first_node("HEX8");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX8" << endl;
      success = false;
    }
    else row->Checksum = (int) (std::strtol(n_->value(), 0, 16) & 0xFF) << 8;
  }
  
  // Checksum 2
  if (success)
  {
    node_t* n_ = n->first_node("HEX9");
    if (n_ == nullptr)
    {
      cerr << "Missing XML element HEX9" << endl;
      success = false;
    }
    else
    {
      row->Checksum |= (int) (std::strtol(n_->value(), 0, 16) & 0xFF);
      row->Hash |= (long long) row->Checksum;
    }
  }
  
  // Developer name
  if (success)
  {
    n = node->first_node("DEVELOPER_ID");
    if (n == nullptr)
    {
      cerr << "Missing XML element DEVELOPER_ID" << endl;
      success = false;
    }
    else row->Developer = string(n->value());
  }
  
  
  // Clean up in case of error
  if (!success)
  {
    delete row;
    row = nullptr;
  }
  
  return row;
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
    ":mapperversion,"
    ":rtc,"
    ":checksum,"
    ":flags"
    ")");
}

bool games_row::bind_to_stmt(sqlite3_stmt* stmt)
{
  int ind;
  
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
  
  ind = sqlite3_bind_parameter_index(stmt, ":developer");
  if (ind == 0)
  {
    cerr << "Unable to find index of parameter 'developer'" << endl;
    return false;
  }
  if (sqlite3_bind_text(stmt, ind, Developer.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
  {
    cerr << "Unable to bind parameter 'developer'" << endl;
    return false;
  }
  
  ind = sqlite3_bind_parameter_index(stmt, ":romsize");
  if (ind == 0)
  {
    cerr << "Unable to find index of parameter 'romsize'" << endl;
    return false;
  }
  if (sqlite3_bind_int(stmt, ind, RomSize) != SQLITE_OK)
  {
    cerr << "Unable to bind parameter 'romsize'" << endl;
    return false;
  }
  
  ind = sqlite3_bind_parameter_index(stmt, ":savesize");
  if (ind == 0)
  {
    cerr << "Unable to find index of parameter 'savesize'" << endl;
    return false;
  }
  if (sqlite3_bind_int(stmt, ind, SaveSize) != SQLITE_OK)
  {
    cerr << "Unable to bind parameter 'savesize'" << endl;
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
  
  ind = sqlite3_bind_parameter_index(stmt, ":mapperversion");
  if (ind == 0)
  {
    cerr << "Unable to find index of parameter 'mapperversion'" << endl;
    return false;
  }
  if (sqlite3_bind_int(stmt, ind, MapperVersion) != SQLITE_OK)
  {
    cerr << "Unable to bind parameter 'mapperversion'" << endl;
    return false;
  }
  
  ind = sqlite3_bind_parameter_index(stmt, ":rtc");
  if (ind == 0)
  {
    cerr << "Unable to find index of parameter 'rtc'" << endl;
    return false;
  }
  if (sqlite3_bind_int(stmt, ind, RTC) != SQLITE_OK)
  {
    cerr << "Unable to bind parameter 'rtc'" << endl;
    return false;
  }
  
  ind = sqlite3_bind_parameter_index(stmt, ":checksum");
  if (ind == 0)
  {
    cerr << "Unable to find index of parameter 'checksum'" << endl;
    return false;
  }
  if (sqlite3_bind_int(stmt, ind, Checksum) != SQLITE_OK)
  {
    cerr << "Unable to bind parameter 'checksum'" << endl;
    return false;
  }
  
  ind = sqlite3_bind_parameter_index(stmt, ":flags");
  if (ind == 0)
  {
    cerr << "Unable to find index of parameter 'flags'" << endl;
    return false;
  }
  if (sqlite3_bind_int(stmt, ind, Flags) != SQLITE_OK)
  {
    cerr << "Unable to bind parameter 'flags'" << endl;
    return false;
  }
  
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
      cerr << "An error occured while parsing XML" << endl;
      success = false;
    }
    /* else
    {
      ios::fmtflags f(cout.flags());
      cout << "0x" << hex << setfill('0') << setw(16) << row->Hash << endl;
      cout.flags(f);
    } */
    
    // Reset statement status so we can rebind parameters
    if (success) sqlite3_reset(stmt);
    
    // Bind parameters to statement
    if (success) if (!row->bind_to_stmt(stmt))
    {
      cerr << sqlite3_errcode(db) << ": " << sqlite3_errmsg(db) << endl;
      cerr << "An error occured while binding parameters to statement" << endl;
      success = false;
    }
    
    // Insert into database
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
