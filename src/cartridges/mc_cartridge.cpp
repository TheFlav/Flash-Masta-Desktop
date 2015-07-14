#include "mc_cartridge.h"

typedef mc_cartridge::address address;
typedef mc_cartridge::chip_index chip_index;

mc_cartridge::mc_cartridge(unsigned int size,
    address base_address, unsigned int num_chips)
  : cartridge(size, base_address), m_num_chips(num_chips)
{
  // Nothing else to do
}

inline unsigned int mc_cartridge::num_chips() const
{
  return m_num_chips;
}

mc_cartridge::chip::chip(unsigned int size, address base_address)
  : m_size(size), m_base_address(base_address)
{
  // Nothing else to do
}

inline unsigned int mc_cartridge::chip::size() const
{
  return m_size;
}

inline address mc_cartridge::chip::base_address() const
{
  return m_base_address;
}
