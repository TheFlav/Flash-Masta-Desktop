#include "ngp_cartridge.h"

// Conveniences (and only local to this file)
typedef ngp_cartridge::chip_index chip_index;
typedef ngp_cartridge::chip chip;
typedef ngp_cartridge::chip::block_index block_index;
using namespace std;

ngp_cartridge::ngp_cartridge()
  : mc_cartridge(), m_chips(0)
{
  // Nothing else to do
}

ngp_cartridge::ngp_cartridge(const ngp_cartridge& other)
  : mc_cartridge(other), m_chips(other.m_chips)
{
  // Nothing else to do
}

const chip* ngp_cartridge::get_chip(chip_index chip) const
{
  return &m_chips[chip];
}

cartridge::system_type ngp_cartridge::system() const
{
  return NEO_GEO_POCKET;
}



////////////////////////////////////////////////////////////////////////////////
//                          ngp_cartridge::chip                               //
////////////////////////////////////////////////////////////////////////////////


ngp_cartridge::chip::chip()
  : mc_cartridge::chip(), m_manufacturer_id(0), m_device_id(0),
    m_num_blocks(0), m_blocks(0)
{
  // Nothing else to do
}

ngp_cartridge::chip::chip(const chip& other)
  : mc_cartridge::chip(other),
    m_manufacturer_id(other.m_manufacturer_id), m_device_id(other.m_device_id),
    m_num_blocks(other.m_num_blocks), m_blocks(other.m_blocks)
{
  // Nothing else to do
}

unsigned int ngp_cartridge::chip::manufacturer_id() const
{
  return m_manufacturer_id;
}

unsigned int ngp_cartridge::chip::device_id() const
{
  return m_device_id;
}

unsigned int ngp_cartridge::chip::num_blocks() const
{
  return m_num_blocks;
}

const ngp_cartridge::chip::block& ngp_cartridge::chip::get_block(
  block_index block) const
{
  return m_blocks[block];
}



////////////////////////////////////////////////////////////////////////////////
//                       ngp_cartridge::chip::block                           //
////////////////////////////////////////////////////////////////////////////////


ngp_cartridge::chip::block::block()
  : m_size(0), m_base_address(0), m_is_protected(0)
{
  // Nothing else to do
}

ngp_cartridge::chip::block::block(const block& other)
  : m_size(other.m_size), m_base_address(other.m_base_address),
    m_is_protected(other.m_is_protected)
{
  // Nothing else to do
}

unsigned int ngp_cartridge::chip::block::size() const
{
  return m_size;
}

address_t ngp_cartridge::chip::block::base_address() const
{
  return m_base_address;
}

bool ngp_cartridge::chip::block::is_protected() const
{
  return m_is_protected;
}
