#include "build-database.h"
#include "ws-games-row.h"
#include "ngp-games-row.h"

int main(const int argc, const char** const argv)
{
  (void) argc;
  (void) argv;
  
  /////////////////////////////////////
  // Initialize operation parameters //
  /////////////////////////////////////
  
  // Number of databases to operate on
  const int num_databases = 2;
  
  // SQLite database file
  const string db_file_name[] = {
    ws::db_file_name,
    ngp::db_file_name
  };
  
  // SQLite table creation query file
  const string schema_file_name[] = {
    ws::schema_file_name,
    ngp::schema_file_name
  };
  
  // Data XML files
  const string data_file_name[] = {
    ws::data_file_name,
    ngp::data_file_name
  };
  
  // Table row objects
  games_row* const data_row[] = {
    new ws::ws_games_row,
    new ngp::ngp_games_row
  };
  
  // Build each database using parameters above
  int code = 0;
  for (int i = 0; code == 0 && i < num_databases; i++)
  {
    code = build_database(db_file_name[i], schema_file_name[i], data_file_name[i], data_row[i]);
    if (code != 0)
    {
      cerr << "Failed to build database " << db_file_name[i] << endl;
    }
  }
  
  // Clean up dynamically allocated objects
  for (int i = 0; i < num_databases; i++)
  {
    delete data_row[i];
  }
  
  return code;
}



int build_database(const string& db_file_name, const string& schema_file_name, const string& data_file_name, games_row* const data_row)
{
  // Declare some variables we're going to be using a lot of
  sqlite3* db;
  doc_t games_xml;
  
  // Open connection to database file
  sqlite3_open_v2(db_file_name.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  int e = sqlite3_errcode(db);
  if (e != SQLITE_OK && e != SQLITE_ROW && e != SQLITE_DONE)
  {
    cerr << sqlite3_errmsg(db) << endl;
    sqlite3_close(db);
    return e;
  }
  
  // Open and execute schema sql file
  ifstream f_schema(schema_file_name.c_str());
  if (!f_schema)
  {
    cerr << "Unable to open schema file '" << schema_file_name << "'" << endl;
    sqlite3_close(db);
    return 1;
  }
  if (!execute_file(f_schema, db))
  {
    sqlite3_close(db);
    return 1;
  }
  f_schema.close();
  
  // Open and parse game information xml
  file<> games_file(data_file_name.c_str());
  games_xml.parse<0>(games_file.data());
  
  // Add games to database
  if (!add_games_to_db(db, &games_xml, data_row))
  {
    cerr << "An error occured while adding games to database" << endl;
  }
  
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

bool add_games_to_db(sqlite3* db, const doc_t* games_xml, games_row* row)
{
  bool success = true;
  
  // Construct statement that uses bound parameters
  string query = row->insert_query();
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
    if (!row->parse_xml(rominfo_node))
    {
      cerr << "An error occured while parsing XML" << endl;
      success = false;
    }
    
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
    
    // Move to next node
    if (success) rominfo_node = rominfo_node->next_sibling();
  }
  
  // Clean up prepared statement
  sqlite3_finalize(stmt);
  
  return success;
}
