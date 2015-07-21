#include "mc_cartridge.h"

// Conveniences (and only local to this file)
typedef mc_cartridge::chip_index chip_index;

mc_cartridge::mc_cartridge()
  : cartridge(), m_num_chips(0)
{
  // Nothing else to do
}

mc_cartridge::mc_cartridge(const mc_cartridge& other)
  : cartridge(other), m_num_chips(other.m_num_chips)
{
  // Nothing else to do
}

mc_cartridge::~mc_cartridge()
{
  // Nothing else to do
}

inline unsigned int mc_cartridge::num_chips() const
{
  return m_num_chips;
}



mc_cartridge::chip::chip()
  : m_size(0), m_base_address(0)
{
  // Nothing else to do
}

mc_cartridge::chip::chip(const chip& other)
  : m_size(other.m_size), m_base_address(other.m_base_address)
{
  // Nothing else to do
}

mc_cartridge::chip::~chip()
{
  // Nothing else to do
}

inline unsigned int mc_cartridge::chip::size() const
{
  return m_size;
}

inline address_t mc_cartridge::chip::base_address() const
{
  return m_base_address;
}
