#include "neo_geo_cartridge.h"

typedef neo_geo_cartridge::address address;
typedef neo_geo_cartridge::chip_index chip_index;
typedef neo_geo_cartridge::neo_geo_chip neo_geo_chip;
typedef neo_geo_chip::block_index block_index;

using namespace std;

neo_geo_cartridge::neo_geo_cartridge(unsigned int size, address base_address,
  unsigned int num_chips, const vector<neo_geo_chip>& chips)
  : multi_chip_cartridge(size, base_address, num_chips),
    m_chips(chips)
{
  // Nothing else to do
}

neo_geo_cartridge::neo_geo_cartridge(const neo_geo_cartridge& other)
  : multi_chip_cartridge(other.m_size, other.m_base_address, other.m_num_chips),
    m_chips(other.m_chips)
{
  // Nothing else to do
}

const neo_geo_chip& neo_geo_cartridge::get_chip(chip_index chip) const
{
  return m_chips[chip];
}

cartridge::system_type neo_geo_cartridge::system() const
{
  return NEO_GEO;
}



neo_geo_cartridge::neo_geo_chip::neo_geo_chip(unsigned int size,
  address base_address, unsigned int num_blocks, const vector<block>& blocks)
  : chip(size, base_address), m_num_blocks(num_blocks),
    m_blocks(blocks)
{
  // Nothing else to do
}

neo_geo_cartridge::neo_geo_chip::neo_geo_chip(const neo_geo_chip& other)
  : chip(other.m_size, other.m_base_address), m_num_blocks(other.m_num_blocks),
    m_blocks(other.m_blocks)
{
  // Nothing else to do
}

inline unsigned int neo_geo_cartridge::neo_geo_chip::num_blocks() const
{
  return m_num_blocks;
}

inline const neo_geo_cartridge::neo_geo_chip::block& neo_geo_cartridge::neo_geo_chip::get_block(
  block_index block) const
{
  return m_blocks[block];
}




neo_geo_cartridge::neo_geo_chip::block::block(unsigned int size,
  address base_address, bool is_protected)
  : m_size(size), m_base_address(base_address), m_is_protected(is_protected)
{
  // Nothing else to do
}

neo_geo_cartridge::neo_geo_chip::block::block(const block& other)
  : m_size(other.m_size), m_base_address(other.m_base_address),
    m_is_protected(other.m_is_protected)
{
  // Nothing else to do
}

inline unsigned int neo_geo_cartridge::neo_geo_chip::block::size() const
{
  return m_size;
}

inline address neo_geo_cartridge::neo_geo_chip::block::base_address() const
{
  return m_base_address;
}

inline bool neo_geo_cartridge::neo_geo_chip::block::is_protected() const
{
  return m_is_protected;
}
