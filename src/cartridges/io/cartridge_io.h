//
//  cartridge_io.h
//  FlashMasta
//
//  Created by Dan on 7/16/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __FLASHMASTA_CARTRIDGES_IO_CARTRIDGE_IO_H__
#define __FLASHMASTA_CARTRIDGES_IO_CARTRIDGE_IO_H__

#include <iostream>
#include "../cartridge.h"

class cartridge_io
{
public:
  virtual unsigned int read_bytes(unsigned int address, char* buffer, unsigned int n) = 0;
  virtual unsigned int write_bytes(unsigned int address, const char* buffer, unsigned int n) = 0;
  
  virtual unsigned int flash_from_stream(std::istream& in) = 0;
  virtual unsigned int flash_to_stream(std::ostream& out) = 0;
  
  virtual const cartridge* get_cartridge_info() = 0;
};

#endif
