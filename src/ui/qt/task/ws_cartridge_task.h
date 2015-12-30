#ifndef __WS_CARTRIDGE_TASK_H__
#define __WS_CARTRIDGE_TASK_H__

#include <QObject>
#include <mutex>
#include "tasks/task_controller.h"
#include "usb/usbfwd.h"

class cartridge;
class QProgressDialog;
class linkmasta_device;
struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

class WsCartridgeTask : public QObject, public task_controller
{
  Q_OBJECT
public:
  explicit              WsCartridgeTask(QWidget *parent, cartridge* cart, int slot = -1);
  virtual               ~WsCartridgeTask();
  
  virtual void          go();
  
  virtual void          on_task_start(int work_expected);
  virtual void          on_task_update(task_status status, int work_progress);
  virtual void          on_task_end(task_status status, int work_total);
  virtual bool          is_task_cancelled() const;
  
protected:
  virtual void          run_task() = 0;
  virtual QString       get_progress_label() const;
  virtual void          set_progress_label(QString label);
  
protected:
  cartridge*            m_cartridge;
  int                   m_slot;
  
private:
  std::mutex*           m_mutex;
  QProgressDialog*      m_progress;
  QString               m_progress_label;
};

#endif // __WS_CARTRIDGE_TASK_H__
