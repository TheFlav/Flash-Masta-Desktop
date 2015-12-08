#ifndef __GAME_DESCRIPTOR_H__
#define __GAME_DESCRIPTOR_H__

struct game_descriptor
{
  enum game_system
  {
    UNKNOWN,
    NEO_GEO_POCKET,
    WONDERSWAN
  };
  
  game_descriptor(const char* name, const char* developer_name);
  game_descriptor(const game_descriptor& other);
  ~game_descriptor();
  
  char* const name;
  char* const developer_name;
  game_system system;
};

#endif // defined(__GAME_DESCRIPTOR_H__)
