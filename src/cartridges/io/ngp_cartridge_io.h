//
//  ngp_cartridge_io.h
//  FlashMasta
//
//  Created by Dan on 7/16/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __FLASHMASTA_CARTRIDGES_IO_NGP_CARTRIDGE_IO_H__
#define __FLASHMASTA_CARTRIDGES_IO_NGP_CARTRIDGE_IO_H__

#include "cartridge_io.h"
#include "../ngp_cartridge.h"

class ngp_cartridge_io: public cartridge_io
{
public:
  ngp_cartridge_io();
  ~ngp_cartridge_io();
  
  unsigned int read_bytes(unsigned int address, char* buffer, unsigned int n);
  unsigned int write_bytes(unsigned int address, const char* buffer, unsigned int n);
  
  unsigned int flash_from_stream(std::istream& in);
  unsigned int flash_to_stream(std::ostream& out);
  
  const ngp_cartridge* get_cartridge_info();
  
protected:
  ngp_cartridge* m_cartridge;
};

#endif
