/*! \file
 *  \brief File containing the declaration of the \ref ws_cartridge class.
 *  
 *  File containing the header information and declaration of the
 *  \ref ws_cartridge class. This file includes the minimal number of files
 *  necessary to use any instance of the \ref ws_cartridge class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-07-28
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __WS_CARTRIDGE_H__
#define __WS_CARTRIDGE_H__

#include "cartridge.h"
#include <vector>

class linkmasta_device;
class ws_rom_chip;
class ws_sram_chip;

/*! \class ws_cartridge
 *  \brief Class representing a WonderSwan game cartridge.
 *  
 *  Class representing a WonderSwan game cartridge. Stores information relevant
 *  to cartridge storage and hardware information, such as the number of onboard
 *  chips and exactly how the chips are divided into blocks. Implements
 *  functions for performing high-level operations on a cartridge.
 *  
 *  This class does not support copying or the copy assignment operator (=).
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class ws_cartridge: public cartridge
{
public:
                        ws_cartridge(linkmasta_device* linkmasta);
                        ~ws_cartridge();
  
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
  std::string           fetch_game_name(int slot);
  

protected:
  void                  build_cartridge_destriptor();
  void                  build_chip_descriptor(unsigned int chip_i);
  void                  build_block_descriptor(unsigned int chip_i, unsigned int block_i);
  void                  build_slots_layout();
  
private:  
  bool                  m_was_init;
  
  linkmasta_device*     m_linkmasta;
  cartridge_descriptor* m_descriptor;
  ws_rom_chip*          m_rom_chip;
  ws_sram_chip*         m_sram_chip;
  
  std::vector<unsigned int> m_slots;
};

#endif /* defined(__WS_CARTRIDGE_H__) */
