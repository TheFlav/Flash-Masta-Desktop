#ifndef __WS_CARTRIDGE_FLASH_TASK_H__
#define __WS_CARTRIDGE_FLASH_TASK_H__

#include "ws_cartridge_task.h"
#include <iosfwd>

class WsCartridgeFlashTask: public WsCartridgeTask
{
public:
  explicit WsCartridgeFlashTask(QWidget* parent, cartridge* cart, int slot = -1);
  ~WsCartridgeFlashTask();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // __WS_CARTRIDGE_FLASH_TASK_H__
