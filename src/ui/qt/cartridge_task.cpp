#include "cartridge_task.h"
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <fstream>
#include <limits>
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ngp_linkmasta_device.h"
#include "libusb-1.0/libusb.h"

using namespace usb;

CartridgeTask::CartridgeTask(QWidget *parent) 
  : QObject(parent), task_controller(), m_mutex(new std::mutex()),
    m_progress(nullptr), m_progress_label()
{
  // Nothing else to do
}

CartridgeTask::~CartridgeTask()
{
  delete m_mutex;
}

void CartridgeTask::go()
{
  // Initialize libusb
  if (libusb_init(&m_libusb) != 0)
  {
    QMessageBox msgBox;
    msgBox.setText("Unable to initialize usb library.");
    msgBox.exec();
    return;
  }
  
  // Get handle to USB device
  m_handle = libusb_open_device_with_vid_pid(m_libusb, 0x20A0, 0x4178);
  if (m_handle == nullptr)
  {
    libusb_exit(m_libusb);
    QMessageBox msgBox;
    msgBox.setText("Unable to find device. Make sure it's connected.");
    msgBox.exec();
    return;
  }
  else
  {
    m_device = libusb_get_device(m_handle);
    libusb_ref_device(m_device);
    libusb_close(m_handle);
  }
  
  // Initialize usb device
  try
  {
    m_usb = new libusb_usb_device(m_device);
    m_usb->init();
  }
  catch (std::exception& ex)
  {
    libusb_unref_device(m_device);
    libusb_exit(m_libusb);
    QMessageBox msgBox;
    msgBox.setText(QString("An error occured while initializing usb device.\n\n") + ex.what());
    msgBox.exec();
    return;
  }
  
  // Initialize linkmasta device
  try
  {
    m_linkmasta = new ngp_linkmasta_device(m_usb);
    m_linkmasta->init();
  }
  catch (std::exception& ex)
  {
    delete m_linkmasta;
    libusb_unref_device(m_device);
    libusb_exit(m_libusb);
    QMessageBox msgBox;
    msgBox.setText(QString("An error occured while initalizing linkmasta device.\n\n") + ex.what());
    msgBox.exec();
    return;
  }
  
  // Test for cartridge
  try
  {
    if (!ngp_cartridge::test_for_cartridge(m_linkmasta))
    {
      delete m_linkmasta;
      libusb_unref_device(m_device);
      libusb_exit(m_libusb);
      QMessageBox msgBox;
      msgBox.setText(QString("No cartridge detected.\n\n"));
      msgBox.exec();
      return;
    }
  }
  catch (std::exception& ex)
  {
    delete m_linkmasta;
    libusb_unref_device(m_device);
    libusb_exit(m_libusb);
    QMessageBox msgBox;
    msgBox.setText(QString("An error occured while testing for cartridge.\n\n") + ex.what());
    msgBox.exec();
    return;
  }
  
  // Initialize cartridge
  try
  {
    m_cartridge = new ngp_cartridge(m_linkmasta);
    m_cartridge->init();
  }
  catch (std::exception& ex)
  {
    delete m_cartridge;
    delete m_linkmasta;
    libusb_unref_device(m_device);
    libusb_exit(m_libusb);
    QMessageBox msgBox;
    msgBox.setText(QString("An error occured while initializing cartridge.\n\n") + ex.what());
    msgBox.exec();
    return;
  }
  
  // Begin task
  bool success = true;
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
    success = false;
  }
  
  // Cleanup
  delete m_cartridge;
  delete m_linkmasta;
  libusb_unref_device(m_device);
  libusb_exit(m_libusb);
  if (m_progress != nullptr)
  {
    m_progress->close();
    delete m_progress;
  }
}



void CartridgeTask::on_task_start(int work_expected)
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

void CartridgeTask::on_task_update(task_status status, int work_progress)
{
  m_mutex->lock();
  task_controller::on_task_update(status, work_progress);
  m_progress->setValue(get_task_work_progress());
  QApplication::processEvents();
  m_mutex->unlock();
}

void CartridgeTask::on_task_end(task_status status, int work_total)
{
  m_mutex->lock();
  task_controller::on_task_end(status, work_total);
  m_mutex->unlock();
}

bool CartridgeTask::is_task_cancelled() const
{
  m_mutex->lock();
  auto r = m_progress->wasCanceled();
  m_mutex->unlock();
  return r;
}



QString CartridgeTask::get_progress_label() const
{
  return m_progress_label;
}

void CartridgeTask::set_progress_label(QString label)
{
  m_progress_label = label;
}
