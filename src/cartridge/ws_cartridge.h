//
//  ws_cartridge.h
//  FlashMasta
//
//  Created by Dan on 8/17/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __WS_CARTRIDGE_H__
#define __WS_CARTRIDGE_H__

#include "cartridge.h"

class linkmasta_device;
class ws_rom_chip;
class ws_sram_chip;

class ws_cartridge: public cartridge
{
public:
  /* constructor */     ws_cartridge(linkmasta_device* linkmasta);
  /* destructor  */     ~ws_cartridge();
  
  system_type           system() const;
  const cartridge_descriptor* descriptor() const;
  
  void                  init();
  bool                  compare_cartridge_game_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  void                  restore_cartridge_game_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  void                  backup_cartridge_game_data(std::ostream& fout, int slot = SLOT_ALL, task_controller* controller = nullptr);
  void                  backup_cartridge_save_data(std::ostream& fout, int slot = SLOT_ALL, task_controller* controller = nullptr);
  void                  restore_cartridge_save_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  bool                  compare_cartridge_save_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  unsigned int          num_slots() const;
  unsigned int          slot_size(int slot) const;

protected:
  void                  build_cartridge_destriptor();
  void                  build_chip_descriptor(unsigned int chip_i);
  void                  build_block_descriptor(unsigned int chip_i, unsigned int block_i);
  
private:  
  bool                  m_was_init;
  
  linkmasta_device*     m_linkmasta;
  cartridge_descriptor* m_descriptor;
  ws_rom_chip*          m_rom_chip;
  ws_sram_chip*         m_sram_chip;
};

#endif /* defined(__WS_CARTRIDGE_H__) */
