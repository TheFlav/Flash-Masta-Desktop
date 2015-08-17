#ifndef __NGP_CARTRIDGE_H__
#define __NGP_CARTRIDGE_H__

#include "cartridge.h"

class linkmasta_device;
class ngp_chip;

/**
 * Class representing a Neo Geo Pocket game cartridge. Stores information
 * relevant to cartridge storage and hardware information, such as the number
 * of onboard chips and exactly how the chips are divided into blocks.
 */
class ngp_cartridge: public cartridge
{
private:
  static const unsigned int MAX_NUM_CHIPS = 2;

public:
  /**
   * Designated constructor for the ngp_cartridge. Initializes properties
   * with default values of each type.
   */
  /* constructor */     ngp_cartridge(linkmasta_device* linkmasta);
  /* destructor  */     ~ngp_cartridge();
  
  // Note: documentation contained in super class cartridge
  system_type           system() const;
  const cartridge_descriptor* descriptor() const;
  
  void                  init();
  void                  backup_cartridge_game_data(std::ostream& fout, task_controller* controller = nullptr);
  void                  restore_cartridge_game_data(std::istream& fin, task_controller* controller = nullptr);
  bool                  compare_cartridge_game_data(std::istream& fin, task_controller* controller = nullptr);
  void                  backup_cartridge_save_data(std::ostream& fout, task_controller* controller = nullptr);
  void                  restore_cartridge_save_data(std::istream& fin, task_controller* controller = nullptr);
  bool                  compare_cartridge_save_data(std::istream& fin, task_controller* controller = nullptr);
  
  static bool           test_for_cartridge(linkmasta_device* linkmasta);
  
protected:
  void                  build_cartridge_destriptor();
  void                  build_chip_descriptor(unsigned int chip_i);
  void                  build_block_descriptor(unsigned int chip_i, unsigned int block_i);
  
private:  
  bool                  m_was_init;
  
  linkmasta_device*     m_linkmasta;
  cartridge_descriptor* m_descriptor;
  unsigned int          m_num_chips;
  ngp_chip*             m_chips[MAX_NUM_CHIPS];
};

#endif // __NGP_CARTRIDGE_H__