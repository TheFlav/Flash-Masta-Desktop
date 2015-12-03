
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

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

int main(const int argc, const char** const argv)
{
  // Declare some variables we're going to be using a lot of
  sqlite3* db;
  xml_document<> games_xml;
  
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



bool execute_file(ifstream& fin, sqlite3* db)
{
  bool success = true;
  string query;
  
  // Read file into query string
  fin.seekg(0, ios::end);
  query.resize(fin.tellg());
  fin.seekg(0, ios::beg);
  fin.read(&query[0], query.size());
  
  // Trim whitespace from end
  
  
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
