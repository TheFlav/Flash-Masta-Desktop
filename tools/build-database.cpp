
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

bool build_tables_from_schema(sqlite3* db, xml_document<>& xml);

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
  
  if (!build_tables_from_schema(db, schema_xml)) return 2;
  
  // Close database connection
  sqlite3_close(db);
  return 0;
}



// Declare some handy little structs
struct table_column {
  table_column(): not_null(false), unique(false), auto_increment(false) {}
  string name;
  string type;
  string default_value;
  bool not_null;
  bool unique;
  bool auto_increment;
};

struct table_index {
  vector<table_column*> columns;
  string order;
};

struct table_key {
  table_key(): primary(false) {}
  vector<table_column*> columns;
  bool primary;
};

struct table {
  table(): primary_key(nullptr) {}
  ~table() {
    for (auto column : columns) if (column != nullptr) delete column;
    for (auto index  : indexes) if (index  != nullptr) delete index;
    for (auto key    : keys)    if (key    != nullptr) delete key;
  }
  string name;
  vector<table_column*> columns;
  vector<table_index*> indexes;
  vector<table_key*> keys;
  table_key* primary_key;
};

struct foreign_key {
  foreign_key(): referencing_table(nullptr), referenced_table(nullptr),
    referencing_column(nullptr), referenced_column(nullptr),
    on_update_cascade(false), on_delete_cascade(false) {}
  string name;
  table* referencing_table;
  table* referenced_table;
  table_column* referencing_column;
  table_column* referenced_column;
  bool on_update_cascade;
  bool on_delete_cascade;
};



// HELPER FUNCTIONS //

inline bool contains_whitespace(const char* str)
{
  for (int i = 0; str[i] != '\0'; i++)
  {
    if (isspace(str[i])) return true;
  }
  return false;
}

template<typename T> inline bool contains_gaps(const vector<T>& list)
{
  static_assert(std::is_pointer<T>::value, "Template function contains_gaps() only applicable to pointer types");
  for (auto item : list) if (item == nullptr) return true; return false;
}

inline table_column* find_column_in_table(string c, const table* t)
{
  for (table_column* col : t->columns) if (col->name == c) return col; return nullptr;
}

/*!
 *  \brief Parses and validates a 'column' property of an index or key, printing
 *         an error to the console and returning -1 if a symantic error is
 *         encountered, otherwise returns the index that the current column is
 *         intended to be stored and adds the new column refernce to the vector.
 */
inline int parse_column_index_and_validate(const node_t* n, const table* t, vector<table_column*>& l)
{
  table_column* target_column = nullptr;
  int target_index;
  bool cancel = false;
  
  // Process index attribute if it exists, otherwise choose first available spot
  if (!cancel) if (n->first_attribute("index") != nullptr)
  {
    xml_attribute<>* attribute = n->first_attribute("index");

    // Parse as int
    char* c;
    target_index = strtol(attribute->value(), &c, 10);

    // Check if parse worked for whole string
    if (!cancel) if (*c != 0 || target_index < 0)
    {
      cerr << "Value of 'index' attribute must be a positive integer" << endl;
      cancel = true;
    }

    // Make sure column at given index is not already defined
    if (!cancel) if (target_index < l.size() && l[target_index] != nullptr)
    {
      cerr << "Index column number " << target_index << " already defined" << endl;
      cancel = true;
    }
  }
  else for (target_index = 0; target_index < l.size() && l[target_index] != nullptr; target_index++);

  // Search for column with matching name in table
  if (!cancel) target_column = find_column_in_table(string(n->value()), t);

  if (!cancel) if (target_column == nullptr)
  {
    cerr << "Reference to undefined column '" << n->value()
         << "' table '" << t->name << "'" << endl;
    cancel = true;
  }

  // Check if column already included in index
  if (!cancel) for (table_column* col : l)
  {
    if (col == target_column)
    {
      cerr << "Duplicate reference to column '" << target_column->name
           << "' in table '" << t->name << "'" << endl;
      cancel = true;
      break;
    }
  }
  
  // Insert into vector
  if (!cancel) if (target_index >= l.size())
  {
    l.resize(target_index + 1, nullptr);
    l[target_index] = target_column;
  }
  
  // Clean up in case of error
  if (cancel)
  {
    target_index = -1;
  }
  
  return target_index;
}

//////////////////////



// Function to build sqlite tables from schema xml
bool build_tables_from_schema(sqlite3* db, xml_document<>& schema_xml)
{
  typedef node_t node_t;
  
  vector<node_t*> table_nodes;
  vector<node_t*> fk_nodes;
  
  vector<table*> tables;
  vector<foreign_key*> foreign_keys;
  
  // Separate table nodes from foreign key nodes so we can process them individually
  for (node_t* curr_node = schema_xml.first_node()->first_node();
       curr_node != nullptr;
       curr_node = curr_node->next_sibling())
  {
    if (strcmp(curr_node->name(), "table") == 0)
    {
      table_nodes.push_back(curr_node);
    }
    else if (strcmp(curr_node->name(), "foreign_key") == 0)
    {
      fk_nodes.push_back(curr_node);
    }
    else
    {
      cerr << "Unknown element schema::" << curr_node->name() << endl;
      return false;
    }
  }
  
  // Process table nodes
  for (node_t* curr_node : table_nodes)
  {
    table curr_table;
    vector<node_t*> column_nodes;
    vector<node_t*> index_nodes;
    vector<node_t*> key_nodes;
    
    // Sort nodes into buckets to be individually processed
    for (curr_node = curr_node->first_node();
         curr_node != nullptr;
         curr_node = curr_node->next_sibling())
    {
      if (strcmp(curr_node->name(), "name") == 0)
      {
        if (strcmp(curr_node->value(), "") == 0)
        {
          cerr << "Table name cannot be empty" << endl;
          return false;
        }
        else if (!curr_table.name.empty())
        {
          cerr << "Table name cannot be redefined" << endl;
          return false;
        }
        else
        {
          curr_table.name = string(curr_node->value());
        }
      }
      else if (strcmp(curr_node->name(), "column") == 0)
      {
        column_nodes.push_back(curr_node);
      }
      else if (strcmp(curr_node->name(), "index") == 0)
      {
        index_nodes.push_back(curr_node);
      }
      else if (strcmp(curr_node->name(), "key") == 0)
      {
        key_nodes.push_back(curr_node);
      }
    }
    
    // Process column buckets
    for (unsigned int i = 0; i < column_nodes.size(); i++)
    {
      curr_node = column_nodes[i];
      
    }
  }
  
}

table* build_table_from_xml(const node_t* table_node)
{
  
}

/*!
 *  \brief Constructs a \ref table_column struct from an XML node.
 *  
 *  Allocates and fills a new instance of \ref table_column with values
 *  parsed from an XML node. If any symantic errors are encountered, prints
 *  an error to \ref std::cerr.
 *  
 *  \param [in] column_node XML element from which to construct the struct.
 *  
 *  \returns A pointer to a valid \ref table_column struct if the operation
 *           was successful or nullptr if the operation failed for any reason.
 */
table_column* build_column_from_xml(const node_t* column_node, const table* parent)
{
  table_column* column = new table_column;
  bool cancel = false;
  
  // Handle each subnode individually, catching symantic errors along the way
  for (node_t* subnode = column_node->first_node();
       !cancel && subnode != nullptr;
       subnode = subnode->next_sibling())
  {
    if (strcmp(subnode->name(), "name") == 0)
    {
      if (!column->name.empty())
      {
        cerr << "Column property 'name' already defined" << endl;
        cancel = true;
      }
      else if (strcmp(subnode->value(), "") == 0)
      {
        cerr << "Column property 'name' cannot be empty" << endl;
        cancel = true;
      }
      else if (contains_whitespace(subnode->value()))
      {
        cerr << "Column property 'name' cannot contain whitespace" << endl;
        cancel = true;
      }
      else for (table_column* col : parent->columns) if (col->name == string(subnode->value()))
      {
        cerr << "Column with name '" << subnode->value()
             << "' already declared" << endl;
        cancel = true;
      }
      
      if (!cancel)
      {
        column->name = string(subnode->value());
      }
    }
    else if (strcmp(subnode->name(), "type") == 0)
    {
      if (!column->type.empty())
      {
        cerr << "Column property 'type' already defined" << endl;
        cancel = true;
      }
      else if (strcmp(subnode->value(), "") == 0)
      {
        cerr << "Column property 'type' cannot be empty" << endl;
        cancel = true;
      }
      else if (contains_whitespace(subnode->value()))
      {
        cerr << "Column property 'type' cannot contain whitespace" << endl;
        cancel = true;
      }
      else
      {
        column->type = string(subnode->value());
      }
    }
    else if (strcmp(subnode->name(), "default") == 0)
    {
      if (!column->default_value.empty())
      {
        cerr << "Column property 'default' already defined" << endl;
        cancel = true;
      }
      else if (strcmp(subnode->value(), "") == 0)
      {
        cerr << "Column property 'default' cannot be empty" << endl;
        cancel = true;
      }
      else if (contains_whitespace(subnode->value()))
      {
        cerr << "Column property 'default' cannot contain whitespace" << endl;
        cancel = true;
      }
      else
      {
        column->default_value = string(subnode->value());
      }
    }
    else if (strcmp(subnode->name(), "not_null") == 0)
    {
      if (column->not_null)
      {
        cerr << "Column property 'not_null' aready defined" << endl;
        cancel = true;
      }
      else
      {
        column->not_null = true;
      }
    }
    else if (strcmp(subnode->name(), "unique") == 0)
    {
      if (column->unique)
      {
        cerr << "Column property 'unique' already defined" << endl;
        cancel = true;
      }
      else
      {
        column->unique = true;
      }
    }
    else if (strcmp(subnode->name(), "auto_increment") == 0)
    {
      if (column->auto_increment)
      {
        cerr << "Column property 'auto_increment' already defined" << endl;
        cancel = true;
      }
      else
      {
        column->auto_increment = true;
      }
    }
    else
    {
      cerr << "Unrecognized column property '" << subnode->name() << "'" << endl;
      cancel = true;
    }
  }
  
  // Ensure all required fields have been defined
  if (!cancel)
  {
    if (column->name.empty())
    {
      cerr << "Column property 'name' undefined" << endl;
      cancel = true;
    }
    if (column->type.empty())
    {
      cerr << "Column property 'type' undefined" << endl;
      cancel = true;
    }
  }
  
  // Clean up if error
  if (cancel)
  {
    delete column;
    column = nullptr;
  }
  
  return column;
}

/*!
 *  \brief Constructs a \ref table_index struct from an XML node.
 *  
 *  Allocates and fills a new instance of \ref table_index with values parsed
 *  from an XML node. If any symantic errors are encountered, prints an error to
 *  \ref std::cerr.
 *  
 *  Even though this function takes a \ref table struct as an argument, it does
 *  insert itself into the table. It is the responsibility of the caller to do
 *  what is necessary to add the created struct to the table.
 *  
 *  \param [in] index_node XML element from which to build the struct.
 *  \param [in] parent Parent table to which this index belongs. Used for
 *              validation and linking to other objects in the table.
 *  
 *  \returns A pointer to a valid \ref table_index struct if the operation was
 *           successful or nullptr if the operation failed for any reason.
 */
table_index* build_index_from_xml(const node_t* index_node, const table* parent)
{
  table_index* index = new table_index;
  bool cancel = false;
  
  // Handle each subnode individually, checking for errors along the way
  for (node_t* subnode = index_node->first_node();
       !cancel && subnode != nullptr;
       subnode = subnode->next_sibling())
  {
    if (strcmp(subnode->name(), "column") == 0)
    {
      // Properly handle the index attribute
      if (parse_column_index_and_validate(subnode, parent, index->columns) == -1)
      {
        cancel = true;
      }
    }
    else if (strcmp(subnode->name(), "order") == 0)
    {
      if (!index->order.empty())
      {
        cerr << "Index property 'order' already defined" << endl;
        cancel = true;
      }
      else
      {
        index->order = string(subnode->value());
      }
    }
    else
    {
      cerr << "Unknown index property '" << subnode->name() << "'" << endl;
      cancel = true;
    }
  }
  
  // Verify that all column spaces have been filled (no gaps)
  if (!cancel) if (contains_gaps<table_column*>(index->columns))
  {
    cerr << "Gaps exist in index definition in table '" << parent->name << "'" << endl;
    cancel = true;
  }
  
  // Clean up in case of errors
  if (cancel)
  {
    delete index;
    index = nullptr;
  }
  
  return index;
}

table_key* build_key_from_xml(const node_t* key_node, const table* parent)
{
  table_key* key = new table_key;
  bool cancel = false;
  
  // First handle "type" attribute
  xml_attribute<>* attr_type = key_node->first_attribute("type");
  if (!cancel) if (attr_type != nullptr)
  {
    // Process primary keys
    if (strcmp(attr_type->value(), "primary") == 0)
    {
      // Make sure a primary key doesn't already exist in the table
      if (parent->primary_key != nullptr)
      {
        cerr << "Primary key for table " << parent->name
             << " already exists" << endl;
        cancel = true;
      }
      else
      {
        key->primary = true;
      }
    }
    else
    {
      cerr << "Unknown value '" << attr_type->value() << "' for key type" << endl;
      cancel = true;
    }
  }
  
  // Process each subnode individually, checking for errors along the way
  for (node_t* subnode = key_node->first_node();
       !cancel && subnode != nullptr;
       subnode = subnode->next_sibling())
  {
    if (strcmp(subnode->name(), "column") == 0)
    {
      if (parse_column_index_and_validate(subnode, parent, key->columns) == -1)
      {
        cancel = true;
      }
    }
    else
    {
      cerr << "Unknown key property '" << subnode->name() << "'" << endl;
      cancel = true;
    }
  }
  
  // Verify that all column spaces have been filled (no gaps)
  if (!cancel) if (contains_gaps<table_column*>(key->columns))
  {
    cerr << "Gaps exist in key definition in table '" << parent->name << "'" << endl;
    cancel = true;
  }
  
  // Clean up in case of errors
  if (cancel)
  {
    delete key;
    key = nullptr;
  }
  
  return key;
}

foreign_key* build_fk_from_xml(const node_t* fk_node, const vector<table*>& tables, const vector<foreign_key*>& foreign_keys)
{
  foreign_key* fk = new foreign_key;
  bool cancel = false;
  
  fk->referencing_table = nullptr;
  fk->referenced_table = nullptr;
  fk->referencing_column = nullptr;
  fk->referenced_column = nullptr;
  
  // Process each subnode individually, checking for errors along the way
  for (node_t* subnode = fk_node->first_node();
       !cancel && subnode != nullptr;
       subnode = subnode->next_sibling())
  {
    if (strcmp(subnode->name(), "name") == 0)
    {
      if (!fk->name.empty())
      {
        cerr << "Foregin key property 'name' already defined" << endl;
        cancel = true;
      }
      else if (strcmp(subnode->value(), "") == 0)
      {
        cerr << "Foreign key property 'name' cannot be empty" << endl;
        cancel = true;
      }
      else if (contains_whitespace(subnode->value()))
      {
        cerr << "Foreign key property 'name' cannot contain whitespace" << endl;
        cancel = true;
      }
      else for (foreign_key* fkey : foreign_keys) if (fkey->name == string(subnode->value()))
      {
        cerr << "Foreign key with name '" << subnode->value()
             << "' already declared" << endl;
        cancel = true;
      }
        
      if (!cancel)
      {
        fk->name = string(subnode->value());
      }
    }
    else if (strcmp(subnode->name(), "referencing_table") == 0)
    {
      table* target_table = nullptr;
      
      if (fk->referencing_table != nullptr)
      {
        cerr << "Foreign key property 'referencing_table' already defined" << endl;
        cancel = true;
      }
      
      // Find table with matching name
      if (!cancel) for (table* t : tables) if (t->name == string(subnode->value()))
      {
        target_table = t;
        break;
      }
      
      if (!cancel) if (target_table == nullptr)
      {
        cerr << "Undefined table '" << subnode->value()
             << "' referenced by foreign key" << endl;
        cancel = true;
      }
      else
      {
        fk->referencing_table = target_table;
      }
    }
    else if (strcmp(subnode->name(), "referenced_table") == 0)
    {
      table* target_table = nullptr;
      
      if (fk->referenced_table != nullptr)
      {
        cerr << "Foreign key property 'referenced_table' already defined" << endl;
        cancel = true;
      }
      
      // Find table with matching name
      if (!cancel) for (table* t : tables) if (t->name == string(subnode->value()))
      {
        target_table = t;
        break;
      }
      
      if (!cancel) if (target_table == nullptr)
      {
        cerr << "Undefined table '" << subnode->value()
             << "' referenced by foreign key" << endl;
        cancel = true;
      }
      else
      {
        fk->referenced_table = target_table;
      }
    }
    else if (strcmp(subnode->name(), "referencing_column") == 0)
    {
      table_column* target_column;
      
      if (fk->referencing_column != nullptr)
      {
        cerr << "Foreign key property 'referencing_column' already defined" << endl;
        cancel = true;
      }
      else if (fk->referencing_table == nullptr)
      {
        cerr << "Foreign key property 'referencing_column' declared before 'referencing_table' defined" << endl;
        cancel = true;
      }
      
      // Find column with matching name
      if (!cancel) target_column = find_column_in_table(string(subnode->value()), fk->referencing_table);
      
      if (!cancel) if (target_column == nullptr)
      {
        cerr << "Reference to undefined column '" << subnode->value()
             << "' in foreign key" << endl;
        cancel = true;
      }
      else
      {
        fk->referencing_column = target_column;
      }
    }
    else if (strcmp(subnode->name(), "referenced_column") == 0)
    {
      table_column* target_column;
      
      if (fk->referenced_column != nullptr)
      {
        cerr << "Foreign key property 'referenced_column' already defined" << endl;
        cancel = true;
      }
      else if (fk->referenced_table == nullptr)
      {
        cerr << "Foreign key property 'referenced_column' declared before 'referenced_table' defined" << endl;
        cancel = true;
      }
      
      // Find column with matching name
      if (!cancel) target_column = find_column_in_table(string(subnode->value()), fk->referenced_table);
      
      if (!cancel) if (target_column == nullptr)
      {
        cerr << "Reference to undefined column '" << subnode->value()
             << "' in foreign key" << endl;
        cancel = true;
      }
      else
      {
        fk->referenced_column = target_column;
      }
    }
    else if (strcmp(subnode->name(), "update_cascade") == 0)
    {
      if (fk->on_update_cascade)
      {
        cerr << "Foreign key property 'update_cascade' already defined" << endl;
        cancel = true;
      }
      else
      {
        fk->on_update_cascade = true;
      }
    }
    else if (strcmp(subnode->name(), "delete_cascade") == 0)
    {
      if (fk->on_delete_cascade)
      {
        cerr << "Foreign key property 'delete_cascade' already defined" << endl;
        cancel = true;
      }
      else
      {
        fk->on_delete_cascade = true;
      }
    }
    else
    {
      cerr << "Unknown foreign key property '" << subnode->name() << "'" << endl;
      cancel = true;
    }
  }
  
  // Ensure all required fields have been defined
  if (!cancel)
  {
    if (fk->name.empty())
    {
      cerr << "Foreign key property 'name' undefined" << endl;
      cancel = true;
    }
    if (fk->referencing_table == nullptr)
    {
      cerr << "Foreign key property 'referencing_table' undefined" << endl;
      cancel = true;
    }
    if (fk->referenced_table == nullptr)
    {
      cerr << "Foreign key property 'referenced_table' undefined" << endl;
      cancel = true;
    }
    if (fk->referencing_column == nullptr)
    {
      cerr << "Foreign key property 'referencing_column' undefined" << endl;
      cancel = true;
    }
    if (fk->referenced_column == nullptr)
    {
      cerr << "Foreign key property 'referenced_column' undefined" << endl;
      cancel = true;
    }
    if (fk->referencing_table  != nullptr && fk->referenced_table  != nullptr &&
        fk->referencing_column != nullptr && fk->referenced_column != nullptr &&
        fk->referencing_table  == fk->referenced_table &&
        fk->referencing_column == fk->referenced_column)
    {
      cerr << "Foreign key cannot cause column to reference itself" << endl;
      cancel = true;
    }
  }
  
  // Clean up in case of error
  if (cancel)
  {
    delete fk;
    fk = nullptr;
  }
  
  return fk;
}
