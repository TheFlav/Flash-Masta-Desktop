//
//  cartridge.h
//  FlashMasta
//
//  Created by Dan on 7/14/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __FLASHMASTA_CARTRIDGES_CARTRIDGE_H__
#define __FLASHMASTA_CARTRIDGES_CARTRIDGE_H__

class cartridge
{
public:
  typedef unsigned int address;
  
  enum console_type
  {
    UNKNOWN,
    NEO_GEO
  };
  
  cartridge(address size_bytes);
  
  virtual address size() const;
  virtual console_type console_type() const = 0;
  
protected:
  const address size_bytes;
};

#endif
