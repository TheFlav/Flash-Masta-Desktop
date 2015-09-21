#ifndef __NGP_CARTRIDGE_BACKUP_SAVE_TASK_H__
#define __NGP_CARTRIDGE_BACKUP_SAVE_TASK_H__

#include "ngp_cartridge_task.h"
#include <iosfwd>

class NgpCartridgeBackupSaveTask : public NgpCartridgeTask
{
public:
  explicit NgpCartridgeBackupSaveTask(QWidget *parent, cartridge* cart);
  ~NgpCartridgeBackupSaveTask();
  
protected:
  void run_task();
  
private:
  std::ofstream* m_fout;
};

#endif // __NGP_CARTRIDGE_BACKUP_SAVE_TASK_H__
