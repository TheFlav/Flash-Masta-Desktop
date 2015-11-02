#ifndef __NGP_CARTRIDGE_VERIFY_TASK_H__
#define __NGP_CARTRIDGE_VERIFY_TASK_H__

#include "ngp_cartridge_task.h"
#include <iosfwd>

class NgpCartridgeVerifyTask : public NgpCartridgeTask
{
public:
  explicit NgpCartridgeVerifyTask(QWidget *parent, cartridge* cart, int slot = -1);
  ~NgpCartridgeVerifyTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // __NGP_CARTRIDGE_VERIFY_TASK_H__
