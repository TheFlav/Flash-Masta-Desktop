#include <iostream>
#include <fstream>
#include <vector>

#include "../src/sqlite/sqlite3.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

void build_tables_from_schema(sqlite3* db, xml_document<>& xml);

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

int main(const int argc, const char** const argv)
{
  // Declare some variables we're going to be using a lot of
  sqlite3* db;
  xml_document<> schema_xml;
  xml_document<> games_xml;
  
  // Open connection to database file
  sqlite3_open_v2("wsgames.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  CHECK_ERROR(db);
  
  // Open and parse schema xml
  file<> schema_file("wsschema.xml");
  schema_xml.parse<0>(schema_file.data());
  
  // Open and parse game information xml
  file<> games_file("wsgames.xml");
  games_xml.parse<0>(games_file.data());
  
  build_tables_from_schema(db, schema_xml);
  
  // Close database connection
  sqlite3_close(db);
  return 0;
}

// Function to build sqlite tables from schema xml
void build_tables_from_schema(sqlite3* db, xml_document<>& schema_xml)
{
  cerr << schema_xml.first_node()->name() << endl;
  
  // Declare some handy little structs
  struct column; // TODO
  struct index;  // TODO
  struct table;  // TODO
  
  // Loop over tables
  for (xml_node<>* curr_node = schema_xml.first_node()->first_node();
       curr_node != nullptr;
       curr_node = curr_node->next_sibling())
  {
    
  }
  
}
