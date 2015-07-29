//
//  ngp_chip.h
//  FlashMasta
//
//  Created by Dan on 7/29/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __NGP_CHIP_H__
#define __NGP_CHIP_H__

class linkmasta_device;

class ngp_chip
{
public:
  /* constructor */       ngp_chip(linkmasta_device* linkmasta_device, unsigned int chip_num);
  /* destructor  */       ~ngp_chip();
  
  unsigned char           read(unsigned int address);
  void                    write(unsigned int address, unsigned char data);
  
  void                    reset();
  unsigned int            get_manufacturer_id();
  unsigned int            get_device_id();
  unsigned int            get_block_protection(unsigned int sector_address);
  void                    program_byte(unsigned int address, unsigned char data);
  void                    unlock_bypass();
  void                    erase_chip();
  void                    erase_block(unsigned int block_address);
  
  bool                    supports_bypass() const;
  bool                    test_bypass_support();
  bool                    is_erasing() const;
  bool                    test_erasing();
  void                    program_bytes(unsigned int address, unsigned char* data, unsigned int num_bytes);
  void                    read_bytes(unsigned int address, unsigned char* data, unsigned int num_bytes);
  
private:
  void                    enter_autoselect();
  
  bool                    m_is_reset;
  bool                    m_is_in_autoselect;
  bool                    m_is_in_bypass;
  bool                    m_is_in_erase;
  bool                    m_is_in_erase_suspend;
  
  bool                    m_supports_bypass;
  
  linkmasta_device* const m_linkmasta;
  unsigned int const      m_chip_num;
};

#endif /* defined(__NGP_CHIP_H__) */
