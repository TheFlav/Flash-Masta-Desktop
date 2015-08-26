//
//  cartridge_descriptor.cpp
//  FlashMasta
//
//  Created by Dan on 7/29/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "types.h"
#include "cartridge_descriptor.h"

cartridge_descriptor::cartridge_descriptor(unsigned int num_chips)
  : num_chips(num_chips),
    chips(num_chips > 0 ? new chip_descriptor*[num_chips] : nullptr)
{
  for (unsigned int i = 0; i < num_chips; ++i)
  {
    chips[i] = nullptr;
  }
}

cartridge_descriptor::cartridge_descriptor(const cartridge_descriptor& other)
  : type(other.type), num_bytes(other.num_bytes),
    num_chips(other.num_chips),
    chips(other.num_chips > 0 ? new chip_descriptor*[other.num_chips] : nullptr)
{
  for (unsigned int i = 0; i < num_chips; ++i)
  {
    if (other.chips[i] != nullptr)
    {
      chips[i] = new chip_descriptor(*other.chips[i]);
    }
  }
}

cartridge_descriptor::~cartridge_descriptor()
{
  for (unsigned int i = 0;i < num_chips; ++i)
  {
    if (chips[i] != nullptr)
    {
      delete chips[i];
    }
  }
  
  if (chips != nullptr)
  {
    delete [] chips;
  }
}



cartridge_descriptor::chip_descriptor::chip_descriptor(unsigned int num_blocks)
  : num_blocks(num_blocks),
    blocks(num_blocks > 0 ? new block_descriptor*[num_blocks] : nullptr)
{
  for (unsigned int i = 0; i < num_blocks; ++i)
  {
    blocks[i] = nullptr;
  }
}

cartridge_descriptor::chip_descriptor::chip_descriptor(const chip_descriptor& other)
  : chip_num(other.chip_num), manufacturer_id(other.manufacturer_id),
    device_id(other.device_id), num_bytes(other.num_bytes),
    num_blocks(other.num_blocks),
    blocks(other.num_blocks > 0 ? new block_descriptor*[other.num_blocks] : nullptr)
{
  for (unsigned int i = 0; i < num_blocks; ++i)
  {
    if (other.blocks[i] != nullptr)
    {
      blocks[i] = new block_descriptor(*other.blocks[i]);
    }
  }
}

cartridge_descriptor::chip_descriptor::~chip_descriptor()
{
  for (unsigned int i = 0; i < num_blocks; ++i)
  {
    if (blocks[i] != nullptr)
    {
      delete blocks[i];
    }
  }
  
  if (blocks != nullptr)
  {
    delete [] blocks;
  }
}



cartridge_descriptor::chip_descriptor::block_descriptor::block_descriptor()
{
  // Nothing else to do
}

cartridge_descriptor::chip_descriptor::block_descriptor::block_descriptor(const block_descriptor& other)
  : block_num(other.block_num), base_address(other.base_address),
    num_bytes(other.num_bytes), is_protected(other.is_protected)
{
  // Nothing else to do
}

cartridge_descriptor::chip_descriptor::block_descriptor::~block_descriptor()
{
  // Nothing else to do
}


