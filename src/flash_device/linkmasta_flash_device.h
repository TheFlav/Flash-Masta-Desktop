//
//  linkmasta_flash_device.h
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __LINKMASTA_FLASH_DEVICE_H__
#define __LINKMASTA_FLASH_DEVICE_H__

#include "flash_device.h"

class usb_device;

class linkmasta_flash_device: public flash_device
{
public:
  
  // Constructors, initializers, and destructors
  /*constructor*/       linkmasta_flash_device(usb_device* usb_device);
  /*destructor */       ~linkmasta_flash_device();
  void                  init();
  
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

#endif /* defined(__LINKMASTA_FLASH_DEVICE_H__) */
