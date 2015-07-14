#include "cartridge.h"

typedef cartridge::address address;

cartridge::cartridge(address size_bytes)
  : size_bytes(size_bytes)
{
  // Do nothing else
}

inline address cartridge::size() const
{
  return size_bytes;
}
