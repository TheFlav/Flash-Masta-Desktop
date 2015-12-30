#ifndef __NGP_CARTRIDGE_VERIFY_SAVE_TASK_H__
#define __NGP_CARTRIDGE_VERIFY_SAVE_TASK_H__

#include "ngp_cartridge_task.h"
#include <iosfwd>

class NgpCartridgeVerifySaveTask : public NgpCartridgeTask
{
public:
  explicit NgpCartridgeVerifySaveTask(QWidget *parent, cartridge* cart, int slot = -1);
  ~NgpCartridgeVerifySaveTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // __NGP_CARTRIDGE_VERIFY_SAVE_TASK_H__
