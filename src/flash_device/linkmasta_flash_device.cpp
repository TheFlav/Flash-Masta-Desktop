//
//  linkmasta_flash_device.cpp
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "linkmasta_flash_device.h"
#include "usb_device/usb_device.h"

linkmasta_flash_device::linkmasta_flash_device(usb_device* usb_device)
  : flash_device(), m_usb_device(usb_device),
    m_was_init(false), m_is_open(false)
{
  // Nothing else to do
}

linkmasta_flash_device::~linkmasta_flash_device()
{
  if (m_is_open)
  {
    close();
  }
  
  if (m_was_init)
  {
    delete m_usb_device;
  }
}



void linkmasta_flash_device::init()
{
  if (m_was_init)
  {
    return;
  }
  
  // TODO
  
  m_was_init = true;
}



void linkmasta_flash_device::open()
{
  if (m_is_open)
  {
    return;
  }
  
  // TODO
  
  m_is_open = true;
}

void linkmasta_flash_device::close()
{
  if (!m_is_open)
  {
    return;
  }
  
  // TODO
  
  m_is_open = false;
}

