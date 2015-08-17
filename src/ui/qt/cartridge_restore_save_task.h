#ifndef CARTRIDGERESTORESAVETASK_H
#define CARTRIDGERESTORESAVETASK_H

#include "cartridge_task.h"
#include <iosfwd>

class CartridgeRestoreSaveTask: public CartridgeTask
{
public:
  explicit CartridgeRestoreSaveTask(QWidget* parent = 0);
  ~CartridgeRestoreSaveTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // CARTRIDGERESTORESAVETASK_H
