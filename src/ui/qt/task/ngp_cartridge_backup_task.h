#ifndef __NGP_CARTRIDGE_BACKUP_TASK_H__
#define __NGP_CARTRIDGE_BACKUP_TASK_H__

#include "ngp_cartridge_task.h"
#include <iosfwd>

class NgpCartridgeBackupTask : public NgpCartridgeTask
{
public:
  explicit NgpCartridgeBackupTask(QWidget *parent, cartridge* cart);
  ~NgpCartridgeBackupTask();
  
protected:
  void run_task();
  
private:
  std::ofstream* m_fout;
};

#endif // __NGP_CARTRIDGE_BACKUP_TASK_H__
