#ifndef __WS_CARTRIDGE_BACKUP_TASK_H__
#define __WS_CARTRIDGE_BACKUP_TASK_H__

#include "ws_cartridge_task.h"
#include <iosfwd>

class WsCartridgeBackupTask : public WsCartridgeTask
{
public:
  explicit WsCartridgeBackupTask(QWidget *parent, cartridge* cart);
  ~WsCartridgeBackupTask();
  
protected:
  void run_task();
  
private:
  std::ofstream* m_fout;
};

#endif // __WS_CARTRIDGE_BACKUP_TASK_H__
