#include "multi_chip_cartridge.h"

typedef multi_chip_cartridge::address address;
typedef multi_chip_cartridge::chip_index chip_index;

multi_chip_cartridge::multi_chip_cartridge(unsigned int size,
    address base_address, unsigned int num_chips)
  : cartridge(size, base_address), m_num_chips(num_chips)
{
  // Nothing else to do
}

inline unsigned int multi_chip_cartridge::num_chips() const
{
  return m_num_chips;
}

multi_chip_cartridge::chip::chip(unsigned int size, address base_address)
  : m_size(size), m_base_address(base_address)
{
  // Nothing else to do
}

inline unsigned int multi_chip_cartridge::chip::size() const
{
  return m_size;
}

inline address multi_chip_cartridge::chip::base_address() const
{
  return m_base_address;
}
