//
//  linkmasta_device.h
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __LINKMASTSA_DEVICE_H__
#define __LINKMASTSA_DEVICE_H__

#include "common/types.h"
#include <string>

class cartridge;
class task_controller;

enum linkmasta_type
{
  LINKMASTA_UNKNOWN,
  LINKMASTA_NEO_GEO_POCKET,
  LINKMASTA_WONDERSWAN
};

class linkmasta_device
{
public:
  typedef unsigned char    data_t;
  typedef unsigned int     word_t;
  typedef unsigned int     timeout_t;
  typedef std::string      version_t;
  typedef unsigned int     chip_index;
  
  virtual                  ~linkmasta_device() {};
  virtual void             init() = 0;
  
  virtual bool             is_open() const = 0;
  virtual timeout_t        timeout() const = 0;
  virtual version_t        firmware_version() = 0;
  
  virtual void             set_timeout(timeout_t timeout) = 0;
  
  virtual void             open() = 0;
  virtual void             close() = 0;
  virtual word_t           read_word(chip_index chip, address_t address) = 0;
  virtual void             write_word(chip_index chip, address_t address, word_t data) = 0;
  
  virtual linkmasta_type   type() const;
  virtual bool             supports_read_bytes() const;
  virtual bool             supports_program_bytes() const;
  virtual bool             supports_erase_chip() const;
  virtual bool             supports_erase_chip_block() const;
  virtual bool             supports_read_manufacturer_id() const;
  virtual bool             supports_read_device_id() const;
  virtual bool             supports_read_block_protection() const;
  virtual bool             supports_read_num_slots() const;
  virtual bool             supports_read_slot_size() const;
  virtual bool             supports_switch_slot() const;
  
  virtual unsigned int     read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr);
  virtual unsigned int     program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr);
  virtual void             erase_chip(chip_index chip);
  virtual void             erase_chip_block(chip_index chip, address_t block_address);
  virtual unsigned int     read_manufacturer_id(chip_index chip);
  virtual unsigned int     read_device_id(chip_index chip);
  virtual bool             read_block_protection(chip_index chip, address_t block_address);
  virtual unsigned int     read_num_slots();
  virtual unsigned int     read_slot_size(unsigned int slot_num);
  virtual bool             switch_slot(unsigned int slot_num);
};

#endif /* defined(__LINKMASTSA_DEVICE_H__) */
