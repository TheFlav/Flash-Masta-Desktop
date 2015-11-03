#ifndef __NGP_CARTRIDGE_FLASH_TASK_H__
#define __NGP_CARTRIDGE_FLASH_TASK_H__

#include "ngp_cartridge_task.h"
#include <iosfwd>

class NgpCartridgeFlashTask: public NgpCartridgeTask
{
public:
  explicit NgpCartridgeFlashTask(QWidget* parent, cartridge* cart, int slot = -1);
  ~NgpCartridgeFlashTask();
  
public:
  void go();
  
protected:
  void run_task();
  
private:
  std::ifstream* m_fin;
};

#endif // __NGP_CARTRIDGE_FLASH_TASK_H__
