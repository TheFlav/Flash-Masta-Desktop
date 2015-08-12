#ifndef CARTRIDGEFLASHTASK_H
#define CARTRIDGEFLASHTASK_H

#include "cartridge_task.h"
#include <iosfwd>

class CartridgeFlashTask: public CartridgeTask
{
public:
  explicit CartridgeFlashTask(QWidget* parent = 0);
  ~CartridgeFlashTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // CARTRIDGEFLASHTASK_H
