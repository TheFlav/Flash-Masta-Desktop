#include "cartridge.h"

cartridge::cartridge()
  : m_size(0), m_base_address(0)
{
  // Nothing else to do
}

cartridge::cartridge(const cartridge& other)
  : m_size(other.m_size), m_base_address(other.m_base_address)
{
  // Nothing else to do
}

cartridge::~cartridge()
{
  // Nothing else to do
}

inline unsigned int cartridge::size() const
{
  return m_size;
}

inline address_t cartridge::base_address() const
{
  return m_base_address;
}
