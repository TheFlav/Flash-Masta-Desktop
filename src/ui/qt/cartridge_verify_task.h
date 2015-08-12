#ifndef CARTRIDGEVERIFYTASK_H
#define CARTRIDGEVERIFYTASK_H

#include "cartridge_task.h"
#include <iosfwd>

class CartridgeVerifyTask : public CartridgeTask
{
public:
  explicit CartridgeVerifyTask(QWidget *parent = 0);
  ~CartridgeVerifyTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // CARTRIDGEVERIFYTASK_H
