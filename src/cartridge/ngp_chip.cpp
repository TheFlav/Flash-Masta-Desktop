//
//  ngp_chip.cpp
//  FlashMasta
//
//  Created by Dan on 7/29/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ngp_chip.h"
#include "linkmasta_device/linkmasta_device.h"



#define ADDR_DONTCARE 0x00000000
#define ADDR_COMMAND1 0x00005555
#define ADDR_COMMAND2 0x00002AAA
#define ADDR_COMMAND3 0x00005555

#define MASK_SECTOR   0x001FE000

const int BYPASS_SUPPORTERS[3] = {
  0x83, /* NGP Flashmasta */
  0x85, /* WS Flashmasta */
  -1    /* sentinal value (end of array) */
};

typedef ngp_chip::data_t        data_t;
typedef ngp_chip::chip_index_t  chip_index_t;
typedef ngp_chip::manufact_id_t manufact_id_t;
typedef ngp_chip::device_id_t   device_id_t;
typedef ngp_chip::protect_t     protect_t;
typedef ngp_chip::address_t     address_t;



ngp_chip::ngp_chip(linkmasta_device* linkmasta_device, chip_index_t chip_num)
  : m_mode(READ), m_last_erased_addr(0), m_supports_bypass(false),
    m_linkmasta(linkmasta_device), m_chip_num(chip_num)
{
  // Nothing else to do
}

ngp_chip::~ngp_chip()
{
  // Nothing to do
}



data_t ngp_chip::read(address_t address)
{
  return m_linkmasta->read_byte(m_chip_num, address);
}

void ngp_chip::write(address_t address, data_t data)
{
  return m_linkmasta->write_byte(m_chip_num, address, data);
}



void ngp_chip::reset()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  if (current_mode() == BYPASS)
  {
    // If we're in bypass mode, do something special to exit it
    write(ADDR_DONTCARE, 0x90);
    write(ADDR_DONTCARE, 0x00);
  }
  else
  {
    // Send the reset command
    write(ADDR_DONTCARE, 0xF0);
  }
  
  // Update the cached mode
  m_mode = READ;
}

manufact_id_t ngp_chip::get_manufacturer_id()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  if (m_linkmasta->supports_read_manufacturer_id())
  {
    if (current_mode() != READ)
    {
      reset();
    }
    
    return m_linkmasta->read_manufacturer_id(m_chip_num);
  }
  else
  {
    if (current_mode() != AUTOSELECT)
    {
      enter_autoselect();
    }
    
    return read(0x0000);
  }
}

device_id_t ngp_chip::get_device_id()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  if (m_linkmasta->supports_read_device_id())
  {
    if (current_mode() != READ)
    {
      reset();
    }
    
    return m_linkmasta->read_device_id(m_chip_num);
  }
  else
  {
    if (current_mode() != AUTOSELECT)
    {
      enter_autoselect();
    }
    
    return read(0x0001);
  }
}

protect_t ngp_chip::get_block_protection(address_t sector_address)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  if (m_linkmasta->supports_read_block_protection())
  {
    // Ensure the chip's been reset before passing control to Linkmasta
    if (current_mode() != READ)
    {
      reset();
    }
    
    return m_linkmasta->read_block_protection(m_chip_num, sector_address);
  }
  else
  {
    if (current_mode() != AUTOSELECT)
    {
      enter_autoselect();
    }
    
    return read((sector_address & MASK_SECTOR) | 0x00000002);
  }
}

void ngp_chip::program_byte(address_t address, data_t data)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Reset if in autoselect mode
  if (current_mode() != BYPASS && current_mode() != READ)
  {
    reset();
  }
  
  // Write prefix based on whether or not in bypass mode
  if (current_mode() == BYPASS)
  {
    write(ADDR_DONTCARE, 0xA0);
  }
  else
  {
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write(ADDR_COMMAND3, 0xA0);
  }
  
  write(address, data);
}

void ngp_chip::unlock_bypass()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Ensure that we actually support bypass mode before doing anything
  if (!supports_bypass())
  {
    return;
  }
  
  // Reset the chip if necessary
  if (current_mode() != READ)
  {
    reset();
  }
  
  // Unlock bypass mode and update flags
  if (current_mode() != BYPASS)
  {
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write(ADDR_COMMAND3, 0x20);
    
    m_mode = BYPASS;
  }
}

void ngp_chip::erase_chip()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Ensure chip has been reset
  if (current_mode() != READ)
  {
    reset();
  }
  
  m_last_erased_addr = 0;
  
  if (m_linkmasta->supports_erase_chip())
  {
    m_linkmasta->erase_chip(m_chip_num);
  }
  else
  {
    // Send the nuke command sequence to chip
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write(ADDR_COMMAND3, 0x80);
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write(ADDR_COMMAND3, 0x10);
  }
  
  m_mode = ERASE;
}

void ngp_chip::erase_block(address_t block_address)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Ensure chip has been reset
  if (current_mode() != READ)
  {
    reset();
  }
  
  m_last_erased_addr = block_address;
  
  if (m_linkmasta->supports_erase_chip())
  {
    m_linkmasta->erase_chip(m_chip_num);
  }
  else
  {
    // Send the sector erase command sequence to chip
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write(ADDR_COMMAND3, 0x80);
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write((block_address & MASK_SECTOR), 0x30);
  }
  
  m_mode = ERASE;
}



ngp_chip::chip_mode ngp_chip::current_mode() const
{
  return m_mode;
}

bool ngp_chip::supports_bypass() const
{
  return m_supports_bypass;
}

bool ngp_chip::test_bypass_support()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Test against manufacturer id, device id, and some potentially custom data
  if (current_mode() != AUTOSELECT)
  {
    enter_autoselect();
  }
  
  unsigned char result = read(0x03);
  
  m_supports_bypass = false;
  for (unsigned int i = 0; BYPASS_SUPPORTERS[i] != -1; ++i)
  {
    if ((int) result == BYPASS_SUPPORTERS[i])
    {
      m_supports_bypass = true;
      break;
    }
  }
  
  return supports_bypass();
}

bool ngp_chip::is_erasing() const
{
  return (current_mode() == ERASE);
}

bool ngp_chip::test_erasing()
{
  if (current_mode() != ERASE)
  {
    return false;
  }
  
  unsigned char result = read(m_last_erased_addr);
  
  m_mode = (result == 0xFF ? READ : ERASE);
  
  return is_erasing();
}

unsigned int ngp_chip::program_bytes(address_t address, const data_t* data, unsigned int num_bytes)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  if (m_linkmasta->supports_program_bytes())
  {
    // Ensure we're in default mode before passing command along to Linkmasta
    if (current_mode() != READ)
    {
      reset();
    }
    
    // Use Linkmasta's built-in support for batch programming
    return m_linkmasta->program_bytes(m_chip_num, address, data, num_bytes, supports_bypass());
  }
  else
  {
    // Linkmasta does not support batch programming; do it manually
    
    // First, ensure we're in the correct mode
    if (supports_bypass() && current_mode() != BYPASS)
    {
      unlock_bypass();
    }
    else if (!supports_bypass() && current_mode() != READ)
    {
      reset();
    }
    
    // Send byte of data one at a time
    for (unsigned int i = 0; i < num_bytes; ++i, ++address)
    {
      program_byte(address, data[i]);
    }
    
    return num_bytes;
  }
}

unsigned int ngp_chip::read_bytes(address_t address, data_t* data, unsigned int num_bytes)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Ensure we're in read mode
  if (current_mode() != READ)
  {
    reset();
  }
  
  if (m_linkmasta->supports_read_bytes())
  {
    // Use Linkmasta's built-in support for batch reads
    return m_linkmasta->read_bytes(m_chip_num, address, data, num_bytes);
  }
  else
  {
    // Linkmasta does not support batch reading; to it manually
    for (unsigned int i = 0; i < num_bytes; ++i, ++address)
    {
      data[i] = read(address);
    }
    
    return num_bytes;
  }
}



void ngp_chip::enter_autoselect()
{
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0x90);
  m_mode = AUTOSELECT;
}

