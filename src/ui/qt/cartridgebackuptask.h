#ifndef CARTRIDGEBACKUPTASK_H
#define CARTRIDGEBACKUPTASK_H

#include <QObject>
#include <mutex>
#include <iosfwd>
#include "tasks/task_controller.h"
#include "usb/usbfwd.h"

class cartridge;
class QProgressDialog;
class linkmasta_device;
struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

class CartridgeBackupTask : public QObject, public task_controller
{
  Q_OBJECT
public:
  explicit CartridgeBackupTask(QWidget *parent = 0);
  ~CartridgeBackupTask();
  void go();
  
  void on_task_start(int work_expected);
  void on_task_update(task_status status, int work_progress);
  void on_task_end(task_status status, int work_total);
  
  bool is_task_cancelled() const;
  
signals:
  
public slots:

private:
  QProgressDialog*      m_progress;
  
  std::mutex*           m_mutex;
  std::ofstream*        m_fout;
  
  usb::usb_device*      m_usb;
  linkmasta_device*     m_linkmasta;
  cartridge*            m_cartridge;
  
  libusb_context*       m_libusb;
  libusb_device*        m_device;
  libusb_device_handle* m_handle;
};

#endif // CARTRIDGEBACKUPTASK_H
