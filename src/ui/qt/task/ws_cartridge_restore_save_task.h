#ifndef __WS_CARTRIDGE_RESTORE_SAVE_TASK_H__
#define __WS_CARTRIDGE_RESTORE_SAVE_TASK_H__

#include "ws_cartridge_task.h"
#include <iosfwd>

class WsCartridgeRestoreSaveTask: public WsCartridgeTask
{
public:
  explicit WsCartridgeRestoreSaveTask(QWidget* parent = 0);
  ~WsCartridgeRestoreSaveTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // __WS_CARTRIDGE_RESTORE_SAVE_TASK_H__
