#ifndef __NGP_CARTRIDGE_H__
#define __NGP_CARTRIDGE_H__

#include "cartridge.h"
#include <vector>

class linkmasta_device;

/**
 * Class representing a Neo Geo Pocket game cartridge. Stores information
 * relevant to cartridge storage and hardware information, such as the number
 * of onboard chips and exactly how the chips are divided into blocks.
 */
class ngp_cartridge: public cartridge
{
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
//void                  restore_cartridge_from_file(std::ifstream& fin);
//void                  backup_cartridge_to_file(std::ofstream& fout);
  
protected:
  void                  build_destriptor();
  
private:
  bool                  m_was_init;
  
  linkmasta_device*     m_linkmasta;
  cartridge_descriptor* m_descriptor;
};

#endif // __NGP_CARTRIDGE_H__
