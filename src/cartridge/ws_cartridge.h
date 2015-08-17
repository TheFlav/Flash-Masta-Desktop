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
class ws_game_chip;

class ws_cartridge: public cartridge
{
public:
  /* constructor */     ws_cartridge(linkmasta_device* linkmasta);
  /* destructor  */     ~ws_cartridge();
  
  system_type           system() const;
  const cartridge_descriptor* descriptor() const;
  
  void                  init();
  bool                  compare_file_to_cartridge(std::ifstream& fin, task_controller* controller = nullptr);
  void                  write_file_to_cartridge(std::ifstream& fin, task_controller* controller = nullptr);
  void                  write_cartridge_to_file(std::ofstream& fout, task_controller* controller = nullptr);

protected:
  void                  build_cartridge_destriptor();
  void                  build_chip_descriptor(unsigned int chip_i);
  void                  build_block_descriptor(unsigned int chip_i, unsigned int block_i);
  
private:  
  bool                  m_was_init;
  
  linkmasta_device*     m_linkmasta;
  cartridge_descriptor* m_descriptor;
  ws_game_chip*         m_game_chip;
};

#endif /* defined(__WS_CARTRIDGE_H__) */
