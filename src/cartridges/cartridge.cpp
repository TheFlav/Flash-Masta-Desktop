#include "cartridge.h"

cartridge::cartridge(unsigned int size_bytes)
  : size_bytes(size_bytes)
{
  // Do nothing else
}

inline unsigned int cartridge::size() const
{
  return size_bytes;
}
