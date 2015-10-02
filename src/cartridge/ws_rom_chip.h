//
//  ws_rom_chip.h
//  FlashMasta
//
//  Created by Dan on 8/17/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __WS_ROM_CHIP_H__
#define __WS_ROM_CHIP_H__

class linkmasta_device;
class task_controller;

class ws_rom_chip
{
public:
  typedef unsigned char   data_t;
  typedef unsigned char   word_t;
  typedef unsigned int    chip_index_t;
  typedef unsigned int    manufact_id_t;
  typedef unsigned int    device_id_t;
  typedef bool            protect_t;
  typedef unsigned int    address_t;
  
  enum chip_mode
  {
    READ,
    AUTOSELECT,
    ERASE
  };
  
  /* constructor */       ws_rom_chip(linkmasta_device* linkmasta_device);
  /* destructor  */       ~ws_rom_chip();
  
  word_t                  read(address_t address);
  void                    write(address_t address, word_t data);
  
  void                    reset();
  manufact_id_t           get_manufacturer_id();
  device_id_t             get_device_id();
  protect_t               get_block_protection(address_t sector_address);
  void                    program_word(address_t address, word_t data);
  void                    unlock_bypass();
  void                    erase_chip();
  void                    erase_block(address_t block_address);
  
  chip_mode               current_mode() const;
  bool                    supports_bypass() const;
  bool                    test_bypass_support();
  bool                    is_erasing() const;
  bool                    test_erasing();
  unsigned int            read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  unsigned int            program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  
private:
  void                    enter_autoselect();
  
  chip_mode               m_mode;
  address_t               m_last_erased_addr;
  
  linkmasta_device* const m_linkmasta;
  chip_index_t const      m_chip_num;
};

#endif /* defined(__WS_ROM_CHIP_H__) */
