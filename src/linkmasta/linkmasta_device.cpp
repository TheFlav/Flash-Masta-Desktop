/*! \file
 *  \brief File containing the implementation of \ref linkmasta_device
 *  
 *  File containing the implementation of \ref linkmasta_device. See
 *  corresponding header file to view documentation for the class, its methods,
 *  and its member variables.
 *  
 *  \see linkmasta_device
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-14
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "linkmasta_device.h"
#include <stdexcept>

linkmasta_system linkmasta_device::system() const
{
  return linkmasta_system::LINKMASTA_UNKNOWN;
}

bool linkmasta_device::supports_read_bytes() const
{
  return false;
}

bool linkmasta_device::supports_program_bytes() const
{
  return false;
}

bool linkmasta_device::supports_erase_chip() const
{
  return false;
}

bool linkmasta_device::supports_erase_chip_block() const
{
  return false;
}

bool linkmasta_device::supports_read_manufacturer_id() const
{
  return false;
}

bool linkmasta_device::supports_read_device_id() const
{
  return false;
}

bool linkmasta_device::supports_read_block_protection() const
{
  return false;
}

bool linkmasta_device::supports_read_num_slots() const
{
  return false;
}

bool linkmasta_device::supports_read_slot_size() const
{
  return false;
}

bool linkmasta_device::supports_switch_slot() const
{
  return false;
}



unsigned int linkmasta_device::read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller)
{
  (void) chip;
  (void) start_address;
  (void) buffer;
  (void) num_bytes;
  (void) controller;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

unsigned int linkmasta_device::program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller)
{
  (void) chip;
  (void) start_address;
  (void) buffer;
  (void) num_bytes;
  (void) bypass_mode;
  (void) controller;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

void linkmasta_device::erase_chip(chip_index chip)
{
  (void) chip;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

void linkmasta_device::erase_chip_block(chip_index chip, address_t block_address)
{
  (void) chip;
  (void) block_address;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

unsigned int linkmasta_device::read_manufacturer_id(chip_index chip)
{
  (void) chip;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

unsigned int linkmasta_device::read_device_id(chip_index chip)
{
  (void) chip;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

bool linkmasta_device::read_block_protection(chip_index chip, address_t block_address)
{
  (void) chip;
  (void) block_address;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

unsigned int linkmasta_device::read_num_slots()
{
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

unsigned int linkmasta_device::read_slot_size(unsigned int slot_num)
{
  (void) slot_num;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

bool linkmasta_device::switch_slot(unsigned int slot_num)
{
  (void) slot_num;
  throw std::runtime_error("ERROR: NOT SUPPORTED");
}

