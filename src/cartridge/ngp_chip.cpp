/*! \file
 *  \brief File containing the implementation of \ref ngp_chip.
 *  
 *  File containing the implementation of \ref ngp_chip.
 *  
 *  See corrensponding header file to view documentation for class, its
 *  methods, and its member variables.
 *  
 *  \see ngp_chip
 *  
 *  \author Daniel Andrus
 *  \date 2015-07-29
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "ngp_chip.h"
#include "linkmasta/linkmasta_device.h"
#include "task/task_controller.h"
#include "task/forwarding_task_controller.h"



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

typedef ngp_chip::data_t         data_t;
typedef ngp_chip::word_t         word_t;
typedef ngp_chip::chip_index_t   chip_index_t;
typedef ngp_chip::manufact_id_t  manufact_id_t;
typedef ngp_chip::device_id_t    device_id_t;
typedef ngp_chip::factory_prot_t factory_prot_t;
typedef ngp_chip::protect_t      protect_t;
typedef ngp_chip::address_t      address_t;



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



word_t ngp_chip::read(address_t address)
{
  return m_linkmasta->read_word(m_chip_num, address);
}

void ngp_chip::write(address_t address, word_t data)
{
  return m_linkmasta->write_word(m_chip_num, address, data);
}



void ngp_chip::reset()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip is busy erasing");
  }
  
  if (current_mode() == BYPASS)
  {
    // If we're in bypass mode, do something special to exit it
    write(ADDR_DONTCARE, 0x90);
    write(ADDR_DONTCARE, 0x00);
  }
  
  // Send the full command
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0xF0);
  
  // Update the cached mode
  m_mode = READ;
}

manufact_id_t ngp_chip::get_manufacturer_id()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip is busy erasing");
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
    throw std::runtime_error("Chip is busy erasing");
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

factory_prot_t ngp_chip::get_factory_prot()
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip is busy erasing");
  }
  
  if (current_mode() != AUTOSELECT)
  {
    enter_autoselect();
  }
  
  return read(0x0003);
}

protect_t ngp_chip::get_block_protection(address_t sector_address)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip is busy erasing");
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
    
    return (read((sector_address & MASK_SECTOR) | 0x00000002) != 0);
  }
}

void ngp_chip::program_byte(address_t address, data_t data)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip is busy erasing");
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
    throw std::runtime_error("Chip is busy erasing");
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
    throw std::runtime_error("Chip is busy erasing");
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
    throw std::runtime_error("Chip is busy erasing");
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
    throw std::runtime_error("Chip is busy erasing");
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

unsigned int ngp_chip::read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip is busy erasing");
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

unsigned int ngp_chip::program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller)
{
  if (is_erasing())
  {
    // We can only reset when we're not erasing
    throw std::runtime_error("Chip is busy erasing");
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
    
    // First, ensure we're in the correct mode
    if (supports_bypass() && current_mode() != BYPASS)
    {
      unlock_bypass();
    }
    else if (!supports_bypass() && current_mode() != READ)
    {
      reset();
    }
    
    // Inform controller of task start
    if (controller != nullptr)
    {
      controller->on_task_start(num_bytes);
    }
    
    // Send byte of data one at a time
    unsigned int i;
    for (i = 0; i < num_bytes && (controller == nullptr || !controller->is_task_cancelled()); ++i, ++address)
    {
      try
      {
        program_byte(address, data[i]);
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



void ngp_chip::enter_autoselect()
{
  write(ADDR_COMMAND1, 0xAA);
  write(ADDR_COMMAND2, 0x55);
  write(ADDR_COMMAND3, 0x90);
  m_mode = AUTOSELECT;
}
