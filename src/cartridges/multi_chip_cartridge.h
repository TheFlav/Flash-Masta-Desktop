#ifndef __FLASHMASTA_MULTI_CHIP_CARTRIDGE_H__
#define __FLASHMASTA_MULTI_CHIP_CARTRIDGE_H__

#include "cartridge.h"

class multi_chip_cartridge: public cartridge
{
public:
  typedef unsigned int chip_index;
  
  class chip
  {
  public:
    chip(unsigned int size, address base_address);
    
    unsigned int size() const;
    address base_address() const;
    
  protected:
    const unsigned int m_size;
    const address m_base_address;
  };
  
  multi_chip_cartridge(unsigned int size,
                       address base_address,
                       unsigned int num_chips);
  
  unsigned int num_chips() const;
  virtual const chip& get_chip(chip_index chip) const = 0;
  
protected:
  const unsigned int m_num_chips;
  
};

#endif
