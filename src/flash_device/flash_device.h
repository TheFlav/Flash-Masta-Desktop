//
//  flash_device.h
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __FLASH_DEVICE_H__
#define __FLASH_DEVICE_H__

#include "common/types.h"
#include <string>

class flash_device
{
public:
  typedef unsigned char data_t;
  
                        flash_device() {};
  virtual               ~flash_device() {};
  
  virtual void          init() = 0;
  virtual bool          test_cartridge() = 0;
  virtual unsigned int  get_timeout() const = 0;
  virtual int           get_vendor_id() = 0;
  virtual int           get_product_id() = 0;
  virtual std::string   get_firmware_version() = 0;
  
  virtual void          set_timeout(unsigned int timeout) = 0;
  
  virtual void          open() = 0;
  virtual void          close() = 0;
  virtual unsigned int  read(address_t start_address, data_t* buffer, unsigned int num_bytes) = 0;
  virtual unsigned int  read(address_t start_address, data_t* buffer, unsigned int num_bytes, unsigned int timeout) = 0;
  virtual unsigned int  write(address_t start_address, data_t* buffer, unsigned int num_bytes) = 0;
  virtual unsigned int  write(address_t start_address, data_t* buffer, unsigned int num_bytes,  unsigned int timeout) = 0;
};

#endif /* defined(__FLASH_DEVICE_H__) */
