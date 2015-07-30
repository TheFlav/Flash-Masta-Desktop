//
//  cartridge_descriptor.h
//  FlashMasta
//
//  Created by Dan on 7/28/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __CARTRIDGE_DESCRIPTOR_H__
#define __CARTRIDGE_DESCRIPTOR_H__



/**
 * Enumeration representing a system to which a game cartridge belongs.
 * Can be used to change how a segment of code processes a game cartridge.
 */
enum system_type
{
  UNKNOWN,          //<! Unknown cartridge type
  NEO_GEO_POCKET    //<! Cartridge made for the Neo Geo Pocket
};



struct cartridge_descriptor
{
  struct                    chip_descriptor;
  
  /* constructor      */    cartridge_descriptor(unsigned int num_chips);
  /* copy constructor */    cartridge_descriptor(const cartridge_descriptor& other);
  /* destructor       */    ~cartridge_descriptor();
  
  system_type               type;
  unsigned int              num_bytes;
  const unsigned int        num_chips;
  chip_descriptor** const   chips;
};



struct cartridge_descriptor::chip_descriptor
{
  struct                    block_descriptor;
  
  /* constructor      */    chip_descriptor(unsigned int num_blocks);
  /* copy constructor */    chip_descriptor(const chip_descriptor& other);
  /* destructor       */    ~chip_descriptor();
  
  unsigned int              chip_num;
  unsigned int              manufacturer_id;
  unsigned int              device_id;
  unsigned int              num_bytes;
  const unsigned int        num_blocks;
  block_descriptor** const  blocks;
};



struct cartridge_descriptor::chip_descriptor::block_descriptor
{
  /* constructor      */    block_descriptor();
  /* copy constructor */    block_descriptor(const block_descriptor& other);
  /* destructor       */    ~block_descriptor();
  
  unsigned int              block_num;
  unsigned int              base_address;
  unsigned int              num_bytes;
  bool                      is_protected;
};

#endif  /* defined(__CARTRIDGE_DESCRIPTOR_H__) */
