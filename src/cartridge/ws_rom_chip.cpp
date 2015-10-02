//
//  ws_rom_chip.cpp
//  FlashMasta
//
//  Created by Dan on 8/17/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ws_rom_chip.h"
#include "linkmasta_device/linkmasta_device.h"
#include "tasks/task_controller.h"
#include "tasks/forwarding_task_controller.h"



#define CHIP_INDEX    0

#define ADDR_DONTCARE 0x00000000
#define ADDR_COMMAND1 0x00000AAA
#define ADDR_COMMAND2 0x00000555
#define ADDR_COMMAND3 0x00000AAA

#define MASK_SECTOR   0xFFFE0000

typedef ws_rom_chip::data_t        data_t;
typedef ws_rom_chip::word_t        word_t;
typedef ws_rom_chip::chip_index_t  chip_index_t;
typedef ws_rom_chip::manufact_id_t manufact_id_t;
typedef ws_rom_chip::device_id_t   device_id_t;
typedef ws_rom_chip::protect_t     protect_t;
typedef ws_rom_chip::address_t     address_t;



ws_rom_chip::ws_rom_chip(linkmasta_device* linkmasta_device)
  : m_mode(READ), m_last_erased_addr(0),
    m_linkmasta(linkmasta_device), m_chip_num(CHIP_INDEX)
{
  // Nothing else to do
}

ws_rom_chip::~ws_rom_chip()
{
  // Nothing to do
}



word_t ws_rom_chip::read(address_t address)
{
  return (word_t) m_linkmasta->read_word(m_chip_num, address);
}

void ws_rom_chip::write(address_t address, word_t data)
{
  return m_linkmasta->write_word(m_chip_num, address, data);
}



void ws_rom_chip::reset()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Send the full command
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0xF0);
  
  // Update the cached mode
  m_mode = READ;
}

manufact_id_t ws_rom_chip::get_manufacturer_id()
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

device_id_t ws_rom_chip::get_device_id()
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
    
    return read(0x0002);
  }
}

protect_t ws_rom_chip::get_block_protection(address_t sector_address)
{
  (void) sector_address;
  return 0;
}

void ws_rom_chip::program_word(address_t address, word_t data)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Reset if in autoselect mode
  if (current_mode() != READ)
  {
    reset();
  }
  
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0xA0);
  write(address, data);
}

void ws_rom_chip::erase_chip()
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

void ws_rom_chip::erase_block(address_t block_address)
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
  
  block_address &= MASK_SECTOR;
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
    write(m_last_erased_addr, 0x30);
  }
  
  m_mode = ERASE;
}



ws_rom_chip::chip_mode ws_rom_chip::current_mode() const
{
  return m_mode;
}

bool ws_rom_chip::is_erasing() const
{
  return (current_mode() == ERASE);
}

bool ws_rom_chip::test_erasing()
{
  if (current_mode() != ERASE)
  {
    return false;
  }
  
  // Send BLANK CHECK SETUP command sequence
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(m_last_erased_addr, 0xEB);
  write(m_last_erased_addr, 0x76);
  write(m_last_erased_addr, 0x00);
  write(m_last_erased_addr, 0x00);
  write(m_last_erased_addr, 0x29);
  
  unsigned char result = read(m_last_erased_addr);
  
  m_mode = (result == 0 ? ERASE : READ);
  
  return is_erasing();
}

unsigned int ws_rom_chip::read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller)
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
    if (controller == nullptr)
    {
      return m_linkmasta->read_bytes(m_chip_num, address, data, num_bytes);
    }
    else
    {
      // Create temporary forwarding controller to pass to linkmasta
      forwarding_task_controller fwd_controller(controller);
      fwd_controller.scale_work_to(num_bytes);
      controller->on_task_start(num_bytes);
      
      unsigned int result = 0;
      
      // Request read from linkmasta, forwarding task progress updates
      if (!controller->is_task_cancelled())
      {
        try
        {
          result = m_linkmasta->read_bytes(m_chip_num, address, data, num_bytes, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Inform controller that task has ended
      controller->on_task_end(controller->is_task_cancelled() && result < num_bytes ? task_status::CANCELLED : task_status::COMPLETED, result);
      return result;
    }
  }
  else
  {
    // Inform controller that task has started
    if (controller != nullptr)
    {
      controller->on_task_start(num_bytes);
    }
    
    // Linkmasta does not support batch reading; to it manually
    unsigned int i;
    for (i = 0; i < num_bytes && (controller == nullptr || !controller->is_task_cancelled()); ++i, ++address)
    {
      try
      {
        data[i] = read(address);
      }
      catch (std::exception& ex)
      {
        // Inform controller that an error has occured and pass exception up
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, i);
        }
        throw;
      }
      
      // Update controller on task progress
      if (controller != nullptr)
      {
        controller->on_task_update(task_status::RUNNING, 1);
      }
    }
    
    // Inform controller that task is complete
    if (controller != nullptr)
    {
      controller->on_task_end(controller->is_task_cancelled() && i < num_bytes ? task_status::CANCELLED : task_status::COMPLETED, num_bytes);
    }
    return i;
  }
}

unsigned int ws_rom_chip::program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller)
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
    if (controller == nullptr)
    {
      return m_linkmasta->program_bytes(m_chip_num, address, data, num_bytes, false);
    }
    else
    {
      // Create temporary forwarding controller to pass to linkmasta
      forwarding_task_controller fwd_controller(controller);
      fwd_controller.scale_work_to(num_bytes);
      controller->on_task_start(num_bytes);
      
      unsigned int result = 0;
      
      // Request program from linkmasta, forwarding task progress updates
      if (!controller->is_task_cancelled())
      {
        try
        {
          result = m_linkmasta->program_bytes(m_chip_num, address, data, num_bytes, false,  &fwd_controller);
        }
        catch (std::exception& ex)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Inform controller of task end
      controller->on_task_end(controller->is_task_cancelled() && result < num_bytes ? task_status::CANCELLED :  task_status::COMPLETED, result);
      return result;
    }
  }
  else
  {
    // Linkmasta does not support batch programming; do it manually
    
    // TODO: Make use of chip's buffer commands
    
    // First, ensure we're in the correct mode
    if (current_mode() != READ)
    {
      reset();
    }
    
    // Inform controller of task start
    if (controller != nullptr)
    {
      controller->on_task_start(num_bytes);
    }
    
    // Program word of data one at a time
    unsigned int i;
    for (i = 0; i < num_bytes && (controller == nullptr || !controller->is_task_cancelled()); i+=2, address+=2)
    {
      // Combine 2 bytes into 1 word
      word_t d = 0;
      d |= data[i];
      d <<= (sizeof(data[i]) * 8);
      d |= data[i+1];
      
      try
      {
        program_word(address, d);
      }
      catch (std::exception& ex)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, i);
        }
        throw;
      }
      
      // Inform controller of task progress
      if (controller != nullptr)
      {
        controller->on_task_update(task_status::RUNNING, 1);
      }
    }
    
    // Inform controller of task end
    if (controller != nullptr)
    {
      controller->on_task_end(controller->is_task_cancelled() && i < num_bytes ? task_status::CANCELLED : task_status::COMPLETED, num_bytes);
    }
    return num_bytes;
  }
}



void ws_rom_chip::enter_autoselect()
{
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0x90);
  m_mode = AUTOSELECT;
}


