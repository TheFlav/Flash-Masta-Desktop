#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include "../common/types.h"
#include "cartridge_descriptor.h"
#include <iosfwd>

class task_controller;



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
  virtual void             backup_cartridge_game_data(std::ostream& fout, task_controller* controller = nullptr) = 0;
  virtual void             restore_cartridge_game_data(std::istream& fin, task_controller* controller = nullptr) = 0;
  virtual bool             compare_cartridge_game_data(std::istream& fin, task_controller* controller = nullptr) = 0;
  virtual void             backup_cartridge_save_data(std::ostream& fout, task_controller* controller = nullptr) = 0;
  virtual void             restore_cartridge_save_data(std::istream& fin, task_controller* controller = nullptr) = 0;
  virtual bool             compare_cartridge_save_data(std::istream& fin, task_controller* controller = nullptr) = 0;
};

#endif // __CARTRIDGE_H__
