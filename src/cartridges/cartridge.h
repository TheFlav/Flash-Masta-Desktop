//
//  cartridge.h
//  FlashMasta
//
//  Created by Dan on 7/14/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

class cartridge
{
  
  
  
public:
  virtual unsigned int manufacturer_id() const = 0;
  virtual unsigned int product_id() const = 0;
  virtual unsigned int size() const = 0;
  virtual unsigned int chips() const = 0;
  virtual unsigned int chip_size(unsigned int chip = 0) const = 0;
};

#endif
