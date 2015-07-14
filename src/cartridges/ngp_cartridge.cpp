#include "ngp_cartridge.h"

typedef ngp_cartridges::address address;
typedef ngp_cartridges::chip_index chip_index;
typedef ngp_cartridges::chip neo_geo_chip;
typedef neo_geo_chip::block_index block_index;

using namespace std;

ngp_cartridges::ngp_cartridges(unsigned int size, address base_address,
  unsigned int num_chips, const vector<chip>& chips)
  : mc_cartridge(size, base_address, num_chips),
    m_chips(chips)
{
  // Nothing else to do
}

ngp_cartridges::ngp_cartridges(const ngp_cartridges& other)
  : mc_cartridge(other.m_size, other.m_base_address, other.m_num_chips),
    m_chips(other.m_chips)
{
  // Nothing else to do
}

const neo_geo_chip& ngp_cartridges::get_chip(chip_index chip) const
{
  return m_chips[chip];
}

cartridge::system_type ngp_cartridges::system() const
{
  return NEO_GEO;
}



ngp_cartridges::chip::chip(unsigned int size,
  address base_address, unsigned int num_blocks, const vector<block>& blocks)
  : mc_cartridge::chip(size, base_address), m_num_blocks(num_blocks),
    m_blocks(blocks)
{
  // Nothing else to do
}

ngp_cartridges::chip::chip(const chip& other)
  : mc_cartridge::chip(other.m_size, other.m_base_address),
    m_num_blocks(other.m_num_blocks), m_blocks(other.m_blocks)
{
  // Nothing else to do
}

inline unsigned int ngp_cartridges::chip::num_blocks() const
{
  return m_num_blocks;
}

inline const ngp_cartridges::chip::block& ngp_cartridges::chip::get_block(
  block_index block) const
{
  return m_blocks[block];
}




ngp_cartridges::chip::block::block(unsigned int size,
  address base_address, bool is_protected)
  : m_size(size), m_base_address(base_address), m_is_protected(is_protected)
{
  // Nothing else to do
}

ngp_cartridges::chip::block::block(const block& other)
  : m_size(other.m_size), m_base_address(other.m_base_address),
    m_is_protected(other.m_is_protected)
{
  // Nothing else to do
}

inline unsigned int ngp_cartridges::chip::block::size() const
{
  return m_size;
}

inline address ngp_cartridges::chip::block::base_address() const
{
  return m_base_address;
}

inline bool ngp_cartridges::chip::block::is_protected() const
{
  return m_is_protected;
}
