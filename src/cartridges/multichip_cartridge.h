#ifndef __FLASHMASTA_MULTICHIP_CARTRIDGE_H__
#define __FLASHMASTA_MULTICHIP_CARTRIDGE_H__

#include "cartridge.h"

class multichip_cartridge: public cartridge
{
public:
  typedef unsigned int chip_index;
  
  multichip_cartridge(chip_index num_chips, const address* size_chips);
  ~multichip_cartridge();
  
  virtual chip_index num_chips() const;
  virtual address size_chip(chip_index chip) const;
  virtual address base_address_chip(chip_index chip) const;
  
protected:
  const chip_index size_chips;
  address* size_bytes_chips;
  address* base_address_chips;
};

#endif
