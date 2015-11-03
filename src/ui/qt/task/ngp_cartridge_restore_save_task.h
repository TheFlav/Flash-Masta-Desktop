#ifndef __NGP_CARTRIDGE_RESTORE_SAVE_TASK_H__
#define __NGP_CARTRIDGE_RESTORE_SAVE_TASK_H__

#include "ngp_cartridge_task.h"
#include <iosfwd>

class NgpCartridgeRestoreSaveTask: public NgpCartridgeTask
{
public:
  explicit NgpCartridgeRestoreSaveTask(QWidget* parent, cartridge* cart, int slot = -1);
  ~NgpCartridgeRestoreSaveTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // __NGP_CARTRIDGE_RESTORE_SAVE_TASK_H__
