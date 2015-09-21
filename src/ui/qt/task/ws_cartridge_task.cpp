#include "ws_cartridge_task.h"
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <fstream>
#include <limits>
#include "cartridge/ws_cartridge.h"
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ws_linkmasta_device.h"
#include "libusb-1.0/libusb.h"

using namespace usb;

WsCartridgeTask::WsCartridgeTask(QWidget *parent, cartridge* cart) 
  : QObject(parent), task_controller(), m_cartridge(cart),
    m_mutex(new std::mutex()), m_progress(nullptr), m_progress_label()
{
  // Nothing else to do
}

WsCartridgeTask::~WsCartridgeTask()
{
  delete m_mutex;
}

void WsCartridgeTask::go()
{
  // Begin task
  try
  {
    this->run_task();
  }
  catch (std::exception& ex)
  {
    (void) ex;
    
    // Only display error message if task wasn't cancelled
    if (!is_task_cancelled())
    {
      QMessageBox msgBox((QWidget*) this->parent());
      msgBox.setText(QString("An error occured during operation.\n\n") + ex.what());
      msgBox.exec();
    }
  }
  
  // Cleanup
  if (m_progress != nullptr)
  {
    m_progress->close();
    delete m_progress;
  }
}



void WsCartridgeTask::on_task_start(int work_expected)
{
  m_mutex->lock();
  task_controller::on_task_start(work_expected);
  
  // Create progress bar
  m_progress = new QProgressDialog(m_progress_label, "Cancel", 0, get_task_expected_work(), (QWidget*) this->parent());
  m_progress->setAutoClose(false);
  m_progress->setAutoReset(false);
  m_progress->setWindowModality(Qt::WindowModal);
  m_progress->setMinimumDuration(0);
  m_mutex->unlock();
}

void WsCartridgeTask::on_task_update(task_status status, int work_progress)
{
  m_mutex->lock();
  task_controller::on_task_update(status, work_progress);
  m_progress->setValue(get_task_work_progress());
  QApplication::processEvents();
  m_mutex->unlock();
}

void WsCartridgeTask::on_task_end(task_status status, int work_total)
{
  m_mutex->lock();
  task_controller::on_task_end(status, work_total);
  m_mutex->unlock();
}

bool WsCartridgeTask::is_task_cancelled() const
{
  m_mutex->lock();
  auto r = m_progress->wasCanceled();
  m_mutex->unlock();
  return r;
}



QString WsCartridgeTask::get_progress_label() const
{
  return m_progress_label;
}

void WsCartridgeTask::set_progress_label(QString label)
{
  m_progress_label = label;
}
