/*! \file
 *  \brief File containing the implementation of \ref ws_rom_chip.
 *  
 *  File containing the implementation of \ref ws_com_chip.
 *  
 *  See corrensponding header file to view documentation for class, its
 *  methods, and its member variables.
 *  
 *  \see ws_rom_chip
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-17
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "ws_rom_chip.h"
#include "linkmasta/linkmasta_device.h"
#include "task/task_controller.h"
#include "task/forwarding_task_controller.h"



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
    m_linkmasta(linkmasta_device), m_chip_num(CHIP_INDEX),
    m_slot_index(0)
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
    throw std::runtime_error("Chip still erasing");
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
    throw std::runtime_error("Chip still erasing");
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
    throw std::runtime_error("Chip still erasing");
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
    throw std::runtime_error("Chip still erasing");
  }
  
  // Reset if in autoselect mode
  if (current_mode() != BYPASS && current_mode() != READ)
  {
    reset();
  }
  
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

void ws_rom_chip::unlock_bypass()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip still erasing");
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

void ws_rom_chip::erase_chip()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip still erasing");
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
    throw std::runtime_error("Chip still erasing");
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

bool ws_rom_chip::supports_bypass() const
{
  return m_supports_bypass;
}

bool ws_rom_chip::test_bypass_support()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip still erasing");
  }
  
  // Assume yes for this particular chip
  m_supports_bypass = true;
  
  return supports_bypass();
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
  unsigned char result1 = read(m_last_erased_addr);
  unsigned char result2 = read(m_last_erased_addr);
  
  m_mode = (result1 != result2 ? ERASE : READ);
  
  return is_erasing();
}

unsigned int ws_rom_chip::read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip still erasing");
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
          (void) ex;
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
        (void) ex;
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
    throw std::runtime_error("Chip still erasing");
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
      return m_linkmasta->program_bytes(m_chip_num, address, data, num_bytes, supports_bypass());
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
          result = m_linkmasta->program_bytes(m_chip_num, address, data, num_bytes, supports_bypass(),  &fwd_controller);
        }
        catch (std::exception& ex)
        {
          (void) ex;
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
    if (supports_bypass() && current_mode() != BYPASS)
    {
      unlock_bypass();
    }
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
    for (i = 0; i < num_bytes && (controller == nullptr || !controller->is_task_cancelled()); i++, address++)
    {
      try
      {
        program_word(address, data[i]);
      }
      catch (std::exception& ex)
      {
        (void) ex;
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

unsigned int ws_rom_chip::selected_slot() const
{
  return m_slot_index;
}

bool ws_rom_chip::select_slot(unsigned int slot)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip still erasing");
  }
  
  // Ensure chip has been reset
  if (current_mode() != READ)
  {
    reset();
  }
  
  if (m_linkmasta->supports_switch_slot())
  {
    // Use linkmasta's functionality if available
    if (m_linkmasta->switch_slot(slot))
    {
      m_slot_index = slot;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    // Only linkmastas can switch slots. Simply fail if the linkmasta can't
    return false;
  }
}



void ws_rom_chip::enter_autoselect()
{
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0x90);
  m_mode = AUTOSELECT;
}
