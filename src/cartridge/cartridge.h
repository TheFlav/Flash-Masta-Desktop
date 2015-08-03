#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include "../common/types.h"
#include "cartridge_descriptor.h"
#include <iosfwd>



/**
 * Abstract class representing a generic game cartridge for an unknown game
 * system. Contains information relevant to storage capacticy and hardware.
 */
class cartridge
{
public:
  /**
   * Destructor for the cartridge class. Does nothing except exist for extendibility.
   */
  virtual /* destructor */ ~cartridge() {};
  
  virtual system_type      system() const = 0;
  virtual const cartridge_descriptor* descriptor() const = 0;
  
  virtual void             init() = 0;
  virtual bool             compare_file_to_cartridge(std::ifstream& fin) = 0;
  virtual void             write_file_to_cartridge(std::ifstream& fin) = 0;
  virtual void             write_cartridge_to_file(std::ofstream& fout) = 0;
};

#endif // __CARTRIDGE_H__
