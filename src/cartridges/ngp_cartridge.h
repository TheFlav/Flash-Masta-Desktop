#ifndef __FLASHMASTA_CARTRIDGES_NGP_CARTRIDGE_H__
#define __FLASHMASTA_CARTRIDGES_NGP_CARTRIDGE_H__

#include "mc_cartridge.h"
#include <vector>

class ngp_cartridges: public mc_cartridge
{
public:
  
  class chip: public mc_cartridge::chip
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
    
    chip(unsigned int size,
                 address base_address,
                 unsigned int num_blocks,
                 const std::vector<block>& blocks);
    chip(const chip& other);
    
    unsigned int num_blocks() const;
    const block& get_block(block_index block) const;
    
  protected:
    const unsigned int m_num_blocks;
    const std::vector<block> m_blocks;
    
  };
  
  ngp_cartridges(unsigned int size,
                    address base_address,
                    unsigned int num_chips,
                    const std::vector<chip>& chips);
  ngp_cartridges(const ngp_cartridges& other);
  
  const chip& get_chip(chip_index chip) const;
  system_type system() const;
  
private:
  const std::vector<chip> m_chips;
};

#endif
