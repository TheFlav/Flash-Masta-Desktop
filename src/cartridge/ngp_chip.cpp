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



ngp_chip::ngp_chip(linkmasta_device* linkmasta_device, unsigned int chip_num)
  : m_is_reset(true), m_is_in_autoselect(false), m_is_in_bypass(false),
    m_is_in_erase(false), m_is_in_erase_suspend(false),
    m_supports_bypass(false),
    m_linkmasta(linkmasta_device), m_chip_num(chip_num)
{
  // Nothing else to do
}

ngp_chip::~ngp_chip()
{
  // Nothing to do
}



unsigned char ngp_chip::read(unsigned int address)
{
  return m_linkmasta->read_byte(m_chip_num, address);
}

void ngp_chip::write(unsigned int address, unsigned char data)
{
  return m_linkmasta->write_byte(m_chip_num, address, data);
}



void ngp_chip::reset()
{
  if (m_is_in_bypass)
  {
    write(ADDR_DONTCARE, 0x90);
    write(ADDR_DONTCARE, 0x00);
  }
  else
  {
    write(ADDR_DONTCARE, 0xF0);
  }
  
  m_is_reset = true;
  m_is_in_autoselect = false;
  m_is_in_bypass = false;
}

unsigned int ngp_chip::get_manufacturer_id()
{
  if (m_linkmasta->supports_read_manufacturer_id())
  {
    if (!m_is_reset)
    {
      reset();
    }
    
    return m_linkmasta->read_manufacturer_id(m_chip_num);
  }
  else
  {
    if (!m_is_in_autoselect)
    {
      enter_autoselect();
    }
    
    return read(0x0000);
  }
}

unsigned int ngp_chip::get_device_id()
{
  if (m_linkmasta->supports_read_device_id())
  {
    if (!m_is_reset)
    {
      reset();
    }
    
    return m_linkmasta->read_device_id(m_chip_num);
  }
  else
  {
    if (!m_is_in_autoselect)
    {
      enter_autoselect();
    }
    
    return read(0x0001);
  }
}

unsigned int ngp_chip::get_block_protection(unsigned int sector_address)
{
  if (m_linkmasta->supports_read_block_protection())
  {
    // Ensure the chip's been reset before passing control to Linkmasta
    if (!m_is_reset)
    {
      reset();
    }
    
    return m_linkmasta->read_block_protection(m_chip_num, sector_address);
  }
  else
  {
    if (!m_is_in_autoselect)
    {
      enter_autoselect();
    }
    
    return read((sector_address & MASK_SECTOR) | 0x00000002);
  }
}

void ngp_chip::program_byte(unsigned int address, unsigned char data)
{
  // Reset if in autoselect mode
  if (!m_is_in_bypass && !m_is_reset)
  {
    reset();
  }
  
  // Write prefix based on whether or not in bypass mode
  if (m_is_in_bypass)
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
  // Ensure that we actually support bypass mode before doing anything
  if (!supports_bypass())
  {
    return;
  }
  
  // Reset the chip if necessary
  if (!m_is_reset)
  {
    reset();
  }
  
  // Unlock bypass mode and update flags
  if (!m_is_in_bypass)
  {
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write(ADDR_COMMAND3, 0x20);
    
    m_is_reset = false;
    m_is_in_bypass = true;
  }
}

void ngp_chip::erase_chip()
{
  // Ensure chip has been reset
  if (!m_is_reset)
  {
    reset();
  }
  
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
  
  m_is_in_erase = true;
  m_is_in_erase_suspend = false;
}

void ngp_chip::erase_block(unsigned int block_address)
{
  // Ensure chip has been reset
  if (!m_is_reset)
  {
    reset();
  }
  
  if (m_linkmasta->supports_erase_chip())
  {
    m_linkmasta->erase_chip(m_chip_num);
  }
  else
  {
    // Send the sector nuke command sequence to chip
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write(ADDR_COMMAND3, 0x80);
    write(ADDR_COMMAND1, 0xAA);
    write(ADDR_COMMAND2, 0x55);
    write((block_address & MASK_SECTOR), 0x30);
  }
  
  m_is_in_erase = true;
  m_is_in_erase_suspend = false;
}



bool ngp_chip::supports_bypass() const
{
  return m_supports_bypass;
}

bool ngp_chip::test_bypass_support()
{
  // Test against manufacturer id, device id, and some potentially custom data
  if (!m_is_in_autoselect)
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
  return m_is_in_erase;
}

bool ngp_chip::test_erasing()
{
  if (!m_is_in_erase)
  {
    return false;
  }
  
  unsigned char result = read(ADDR_DONTCARE);
  
  m_is_in_erase = (result != 0xFF);
  
  if (!is_erasing())
  {
    m_is_reset = true;
  }
  
  return is_erasing();
}

void ngp_chip::program_bytes(unsigned int address, unsigned char* data, unsigned int num_bytes)
{
  if (m_linkmasta->supports_program_bytes())
  {
    // Ensure we're in default mode before passing command along to Linkmasta
    if (!m_is_reset)
    {
      reset();
    }
    
    // Use Linkmasta's built-in support for batch programming
    m_linkmasta->program_bytes(m_chip_num, address, data, num_bytes, supports_bypass());
  }
  else
  {
    // Linkmasta does not support batch programming; do it manually
    
    // First, ensure we're in the correct mode
    if (supports_bypass() && !m_is_in_bypass)
    {
      unlock_bypass();
    }
    else if (!supports_bypass() && !m_is_reset)
    {
      reset();
    }
    
    // Send byte of data one at a time
    for (unsigned int i = 0; i < num_bytes; ++i, ++address)
    {
      program_byte(address, data[i]);
    }
  }
}

void ngp_chip::read_bytes(unsigned int address, unsigned char* data, unsigned int num_bytes)
{
  // Ensure we're in read mode
  if (!m_is_reset)
  {
    reset();
  }
  
  if (m_linkmasta->supports_read_bytes())
  {
    // Use Linkmasta's built-in support for batch reads
    m_linkmasta->read_bytes(m_chip_num, address, data, num_bytes);
  }
  else
  {
    // Linkmasta does not support batch reading; to it manually
    for (unsigned int i = 0; i < num_bytes; ++i, ++address)
    {
      data[i] = read(address);
    }
  }
}



void ngp_chip::enter_autoselect()
{
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0x90);
  m_is_in_autoselect = true;
  m_is_reset = false;
}

