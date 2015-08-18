#ifndef __NGP_CARTRIDGE_TASK_H__
#define __NGP_CARTRIDGE_TASK_H__

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

class NgpCartridgeTask : public QObject, public task_controller
{
  Q_OBJECT
public:
  explicit              NgpCartridgeTask(QWidget *parent = 0);
  virtual               ~NgpCartridgeTask();
  
  virtual void          go();
  
  virtual void          on_task_start(int work_expected);
  virtual void          on_task_update(task_status status, int work_progress);
  virtual void          on_task_end(task_status status, int work_total);
  virtual bool          is_task_cancelled() const;
  
protected:
  virtual void          run_task() = 0;
  virtual QString       get_progress_label() const;
  virtual void          set_progress_label(QString label);
  cartridge*            m_cartridge;
  
  static const unsigned short target_vendor_id = 0x20A0;
  static const unsigned short target_device_id = 0x4178;
  
private:
  std::mutex*           m_mutex;
  QProgressDialog*      m_progress;
  QString               m_progress_label;
  
  usb::usb_device*      m_usb;
  linkmasta_device*     m_linkmasta;
  
  libusb_context*       m_libusb;
  libusb_device*        m_device;
  libusb_device_handle* m_handle;
};

#endif // __NGP_CARTRIDGE_TASK_H__
