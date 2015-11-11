//
//  ws_linkmasta_device.h
//  FlashMasta
//
//  Created by Dan on 8/14/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __WS_LINKMASTA_DEVICE_H__
#define __WS_LINKMASTA_DEVICE_H__

#include "linkmasta_device.h"
#include "usb/usbfwd.h"

class ws_linkmasta_device: public linkmasta_device
{
public:
  // Constructors, initializers, and destructors
  /*constructor*/  ws_linkmasta_device(usb::usb_device* usb_device);
  /*destructor */  ~ws_linkmasta_device();
  void             init();
  
  // Getters
  bool             is_open() const;
  timeout_t        timeout() const;
  version_t        firmware_version();
  bool             is_integrated_with_cartridge() const;
  
  // Setters
  void             set_timeout(timeout_t timeout);
  
  // Operations
  void             open();
  void             close();
  word_t           read_word(chip_index chip, address_t address);
  void             write_word(chip_index chip, address_t address, word_t data);
  bool             test_for_cartridge();
  cartridge*       build_cartridge();
  
  linkmasta_type   type() const;
  bool             supports_read_bytes() const;
  bool             supports_program_bytes() const;
  bool             supports_read_num_slots() const;
  bool             supports_read_slot_size() const;
  bool             supports_switch_slot() const;
  
  unsigned int     read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr);
  unsigned int     program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr);
  unsigned int     read_num_slots();
  unsigned int     read_slot_size(unsigned int slot_num);
  bool             switch_slot(unsigned int slot_num);
  
private:
  void             fetch_firmware_version();
  void             fetch_slot_info();
  
  // Resources
  usb::usb_device* const m_usb_device;
  
  // Status flags
  bool             m_was_init;
  bool             m_is_open;
  bool             m_firmware_version_set;
  bool             m_slot_info_set;
  
  // Cached values
  unsigned int     m_firmware_major_version;
  unsigned int     m_firmware_minor_version;
  bool             m_static_num_slots;
  bool             m_static_slot_sizes;
  unsigned int     m_num_slots;
  unsigned int     m_slot_size;
};

#endif /* defined(__WS_LINKMASTA_DEVICE_H__) */
