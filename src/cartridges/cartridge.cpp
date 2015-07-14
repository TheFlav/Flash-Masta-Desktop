#include "cartridge.h"

typedef cartridge::address address;

cartridge::cartridge(unsigned int size, address base_address)
  : m_size(size), m_base_address(base_address)
{
  // Nothing else to do
}

inline unsigned int cartridge::size() const
{
  return m_size;
}

inline address cartridge::base_address() const
{
  return m_base_address;
}
