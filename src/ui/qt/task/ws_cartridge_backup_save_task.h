#ifndef __WS_CARTRIDGE_BACKUP_SAVE_TASK_H__
#define __WS_CARTRIDGE_BACKUP_SAVE_TASK_H__

#include "ws_cartridge_task.h"
#include <iosfwd>

class WsCartridgeBackupSaveTask: public WsCartridgeTask
{
public:
  explicit WsCartridgeBackupSaveTask(QWidget* parent, cartridge* cart);
  ~WsCartridgeBackupSaveTask();
  
protected:
  void run_task();
  
private:
  std::ofstream* m_fout;
};


#endif // __WS_CARTRIDGE_BACKUP_SAVE_TASK_H__
