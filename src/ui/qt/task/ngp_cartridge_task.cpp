#include "ngp_cartridge_task.h"
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <fstream>
#include <limits>
#include "cartridge/ngp_cartridge.h"
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ngp_linkmasta_device.h"
#include "libusb-1.0/libusb.h"

using namespace usb;

NgpCartridgeTask::NgpCartridgeTask(QWidget *parent, cartridge* cart, int slot) 
  : QObject(parent), task_controller(), m_cartridge(cart), m_slot(slot),
    m_mutex(new std::mutex()), m_progress(nullptr), m_progress_label()
{
  // Nothing else to do
}

NgpCartridgeTask::~NgpCartridgeTask()
{
  delete m_mutex;
}

void NgpCartridgeTask::go()
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



void NgpCartridgeTask::on_task_start(int work_expected)
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

void NgpCartridgeTask::on_task_update(task_status status, int work_progress)
{
  m_mutex->lock();
  task_controller::on_task_update(status, work_progress);
  m_progress->setValue(get_task_work_progress());
  QApplication::processEvents();
  m_mutex->unlock();
}

void NgpCartridgeTask::on_task_end(task_status status, int work_total)
{
  m_mutex->lock();
  task_controller::on_task_end(status, work_total);
  m_mutex->unlock();
}

bool NgpCartridgeTask::is_task_cancelled() const
{
  m_mutex->lock();
  auto r = m_progress->wasCanceled();
  m_mutex->unlock();
  return r;
}



QString NgpCartridgeTask::get_progress_label() const
{
  return m_progress_label;
}

void NgpCartridgeTask::set_progress_label(QString label)
{
  m_progress_label = label;
}
