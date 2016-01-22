/*! \file
 *  \brief File containing the implementation of \ref ws_sram_chip.
 *  
 *  File containing the implementation of \ref ws_sram_chip.
 *  
 *  See corrensponding header file to view documentation for class, its
 *  methods, and its member variables.
 *  
 *  \see ws_sram_chip
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-25
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "ws_sram_chip.h"
#include "linkmasta/linkmasta_device.h"
#include "task/task_controller.h"
#include "task/forwarding_task_controller.h"



#define CHIP_INDEX    1

typedef ws_sram_chip::data_t data_t;
typedef ws_sram_chip::word_t word_t;
typedef ws_sram_chip::address_t address_t;

ws_sram_chip::ws_sram_chip(linkmasta_device* linkmasta)
  : m_linkmasta(linkmasta), m_chip_num(CHIP_INDEX)
{
  // Nothing else to do
}

ws_sram_chip::~ws_sram_chip()
{
  // Nothing else to do
}

word_t ws_sram_chip::read(address_t address)
{
  return m_linkmasta->read_word(m_chip_num, address);
}

void ws_sram_chip::write(address_t address, word_t data)
{
  return m_linkmasta->write_word(m_chip_num, address, data);
}

unsigned int ws_sram_chip::read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller)
{
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

unsigned int ws_sram_chip::program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller)
{
  if (m_linkmasta->supports_program_bytes())
  {
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
        write(address, data[i]);
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
