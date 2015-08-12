#ifndef CARTRIDGEBACKUPTASK_H
#define CARTRIDGEBACKUPTASK_H

#include "cartridge_task.h"
#include <iosfwd>

class CartridgeBackupTask : public CartridgeTask
{
public:
  explicit CartridgeBackupTask(QWidget *parent = 0);
  ~CartridgeBackupTask();
  
protected:
  void run_task();
  
private:
  std::ofstream* m_fout;
};

#endif // CARTRIDGEBACKUPTASK_H
