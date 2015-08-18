#ifndef __WS_CARTRIDGE_VERIFY_TASK_H__
#define __WS_CARTRIDGE_VERIFY_TASK_H__

#include "ws_cartridge_task.h"
#include <iosfwd>

class WsCartridgeVerifyTask : public WsCartridgeTask
{
public:
  explicit WsCartridgeVerifyTask(QWidget *parent = 0);
  ~WsCartridgeVerifyTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // __WS_CARTRIDGE_VERIFY_TASK_H__
