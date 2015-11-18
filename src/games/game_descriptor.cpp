#include "game_descriptor.h"
#include <cstring>

game_descriptor::game_descriptor(const char* name, const char* developer_name)
  : name(new char [strlen(name)]), developer_name(new char [strlen(developer_name)])
{
  strcpy(this->name, name);
  strcpy(this->developer_name, developer_name);
}

game_descriptor::game_descriptor(const game_descriptor& other)
  : game_descriptor(other.name, other.developer_name)
{
  system = other.system;
}

game_descriptor::~game_descriptor()
{
  if (name != nullptr)
  {
    delete [] name;
  }
  if (developer_name != nullptr)
  {
    delete [] developer_name;
  }
}
