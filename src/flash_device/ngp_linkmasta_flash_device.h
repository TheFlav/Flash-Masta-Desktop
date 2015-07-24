//
//  ngp_linkmasta_flash_device.h
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __NGP_LINKMASTA_FLASH_DEVICE_H__
#define __NGP_LINKMASTA_FLASH_DEVICE_H__

#include "flash_device.h"

class usb_device;

class ngp_linkmasta_flash_device: public flash_device
{
public:
  
  // Constructors, initializers, and destructors
  /*constructor*/       ngp_linkmasta_flash_device(usb_device* usb_device);
  /*destructor */       ~ngp_linkmasta_flash_device();
  void                  init();
  
  // Getters
  bool                  is_open() const;
  timeout_t             timeout() const;
  
  // Setters
  void                  set_timeout(timeout_t timeout);
  
  // Operations
  void                  open();
  void                  close();
  
  
private:
  
  // Resources
  usb_device* const     m_usb_device;
  
  // Status flags
  bool                  m_was_init;
  bool                  m_is_open;
};

#endif /* defined(__ngp_linkmasta_flash_device_H__) */
