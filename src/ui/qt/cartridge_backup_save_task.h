#ifndef CARTRIDGEBACKUPSAVETASK_H
#define CARTRIDGEBACKUPSAVETASK_H

#include "cartridge_task.h"
#include <iosfwd>

class CartridgeBackupSaveTask : public CartridgeTask
{
public:
  explicit CartridgeBackupSaveTask(QWidget *parent = 0);
  ~CartridgeBackupSaveTask();
  
protected:
  void run_task();
  
private:
  std::ofstream* m_fout;
};

#endif // CARTRIDGEBACKUPSAVETASK_H
