#ifndef __FLASHMASTA_CARTRIDGES_NEO_GEO_CARTRIDGE_H__
#define __FLASHMASTA_CARTRIDGES_NEO_GEO_CARTRIDGE_H__

#include "multi_chip_cartridge.h"
#include <vector>

class neo_geo_cartridge: public multi_chip_cartridge
{
public:
  
  class neo_geo_chip: public multi_chip_cartridge::chip
  {
  public:
    typedef unsigned int block_index;
    
    class block
    {
    public:
      block(unsigned int size,
            address base_address,
            bool is_protected);
      block(const block& other);
      
      unsigned int size() const;
      address base_address() const;
      bool is_protected() const;
      
    protected:
      const unsigned int m_size;
      const address m_base_address;
      const bool m_is_protected;
    };
    
    neo_geo_chip(unsigned int size,
                 address base_address,
                 unsigned int num_blocks,
                 const std::vector<block>& blocks);
    neo_geo_chip(const neo_geo_chip& other);
    
    unsigned int num_blocks() const;
    const block& get_block(block_index block) const;
    
  protected:
    const unsigned int m_num_blocks;
    const std::vector<block> m_blocks;
    
  };
  
  neo_geo_cartridge(unsigned int size,
                    address base_address,
                    unsigned int num_chips,
                    const std::vector<neo_geo_chip>& chips);
  neo_geo_cartridge(const neo_geo_cartridge& other);
  
  const neo_geo_chip& get_chip(chip_index chip) const;
  system_type system() const;
  
private:
  const std::vector<neo_geo_chip> m_chips;
};

#endif
