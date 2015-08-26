//
//  ws_sram_chip.h
//  FlashMasta
//
//  Created by Dan on 8/25/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __WS_SRAM_CHIP__
#define __WS_SRAM_CHIP__

class linkmasta_device;
class task_controller;

class ws_sram_chip
{
public:
  typedef unsigned char   data_t;
  typedef unsigned char   word_t;
  typedef unsigned int    chip_index_t;
  typedef unsigned int    address_t;
  
                          ws_sram_chip(linkmasta_device* linkmasta);
                          ~ws_sram_chip();
  
  word_t                  read(address_t address);
  void                    write(address_t address, word_t data);
  
  unsigned int            read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  unsigned int            program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  
private:
  linkmasta_device* m_linkmasta;
  chip_index_t      m_chip_num;
};

#endif /* defined(__WS_SRAM_CHIP__) */
