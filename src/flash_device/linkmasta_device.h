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

class linkmasta_device
{
public:
  typedef unsigned char    data_t;
  typedef int              timeout_t;
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
  virtual unsigned int     read(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes) = 0;
  virtual unsigned int     read(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, timeout_t timeout) = 0;
  virtual unsigned int     write(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes) = 0;
  virtual unsigned int     write(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, timeout_t timeout) = 0;
};

#endif /* defined(__LINKMASTSA_DEVICE_H__) */
