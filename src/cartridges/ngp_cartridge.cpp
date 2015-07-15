#include "ngp_cartridge.h"

// Conveniences (and only local to this file)
typedef ngp_cartridge::address address;
typedef ngp_cartridge::chip_index chip_index;
typedef ngp_cartridge::chip neo_geo_chip;
typedef neo_geo_chip::block_index block_index;
using namespace std;

ngp_cartridge::ngp_cartridge(unsigned int size, address base_address,
  unsigned int num_chips, const vector<chip>& chips)
  : mc_cartridge(size, base_address, num_chips),
    m_chips(chips)
{
  // Nothing else to do
}

ngp_cartridge::ngp_cartridge(const ngp_cartridge& other)
  : mc_cartridge(other.m_size, other.m_base_address, other.m_num_chips),
    m_chips(other.m_chips)
{
  // Nothing else to do
}

const neo_geo_chip& ngp_cartridge::get_chip(chip_index chip) const
{
  return m_chips[chip];
}

cartridge::system_type ngp_cartridge::system() const
{
  return NEO_GEO_POCKET;
}



////////////////////////////////////////////////////////////////////////////////
//                          ngp_cartridge::chip                               //
////////////////////////////////////////////////////////////////////////////////


ngp_cartridge::chip::chip(unsigned int size,
  address base_address, unsigned int num_blocks, const vector<block>& blocks)
  : mc_cartridge::chip(size, base_address), m_num_blocks(num_blocks),
    m_blocks(blocks)
{
  // Nothing else to do
}

ngp_cartridge::chip::chip(const chip& other)
  : mc_cartridge::chip(other.m_size, other.m_base_address),
    m_num_blocks(other.m_num_blocks), m_blocks(other.m_blocks)
{
  // Nothing else to do
}

inline unsigned int ngp_cartridge::chip::num_blocks() const
{
  return m_num_blocks;
}

inline const ngp_cartridge::chip::block& ngp_cartridge::chip::get_block(
  block_index block) const
{
  return m_blocks[block];
}



////////////////////////////////////////////////////////////////////////////////
//                       ngp_cartridge::chip::block                           //
////////////////////////////////////////////////////////////////////////////////


ngp_cartridge::chip::block::block(unsigned int size,
  address base_address, bool is_protected)
  : m_size(size), m_base_address(base_address), m_is_protected(is_protected)
{
  // Nothing else to do
}

ngp_cartridge::chip::block::block(const block& other)
  : m_size(other.m_size), m_base_address(other.m_base_address),
    m_is_protected(other.m_is_protected)
{
  // Nothing else to do
}

inline unsigned int ngp_cartridge::chip::block::size() const
{
  return m_size;
}

inline address ngp_cartridge::chip::block::base_address() const
{
  return m_base_address;
}

inline bool ngp_cartridge::chip::block::is_protected() const
{
  return m_is_protected;
}
