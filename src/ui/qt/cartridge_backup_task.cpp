#include "cartridge_backup_task.h"
#include <fstream>
#include <thread>
#include <QMessageBox>
#include <QProgressDialog>
#include <QFileDialog>
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ngp_linkmasta_device.h"
#include "cartridge/ngp_cartridge.h"
#include "libusb-1.0/libusb.h"
#include <functional>
#include <QApplication>

using namespace usb;

CartridgeBackupTask::CartridgeBackupTask(QWidget *parent) : QObject(parent), task_controller(), m_mutex(new std::mutex())
{
  // Nothing else to do
}

CartridgeBackupTask::~CartridgeBackupTask()
{
  delete m_mutex;
}

void CartridgeBackupTask::go()
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
  
  // Get destination file from user
  QString filename = QFileDialog::getSaveFileName(
        (QWidget*) this->parent(), tr("Save File"), "backup.ngp",
        tr("Neo Geo Pocket (*.ngp)"));
  if (filename == QString::null)
  {
    // Quietly fail
    delete m_linkmasta;
    libusb_unref_device(m_device);
    libusb_exit(m_libusb);
    return;
  }
  
  // Initialize output file
  m_fout = new std::ofstream(filename.toStdString().c_str(), std::ios::binary);
  if (!m_fout->is_open())
  {
    delete m_fout;
    delete m_linkmasta;
    libusb_unref_device(m_device);
    libusb_exit(m_libusb);
    QMessageBox msgBox;
    msgBox.setText("Unable to open destination file");
    msgBox.exec();
    return;
  }
  
  // create progress bar dialog
  m_progress = new QProgressDialog("Backing up cartridge", "Cancel", 0, 1, (QWidget*) this->parent());
  m_progress->setAutoClose(false);
  m_progress->setWindowModality(Qt::WindowModal);
  m_progress->show();
  
  // Begin task
  m_cartridge->write_cartridge_to_file(*m_fout, this);
  
  // Cleanup
  m_fout->close();
  delete m_fout;
  delete m_cartridge;
  delete m_linkmasta;
  libusb_unref_device(m_device);
  libusb_exit(m_libusb);
  if (get_task_status() == task_status::ERROR)
  {
    QMessageBox msgBox(m_progress);
    msgBox.setText("An error occured while backing up cartridge.");
    msgBox.exec();
  }
  m_progress->close();  
  
  /*
  // start up new thread
  std::thread t([this]()
  {
    this->m_cartridge->write_cartridge_to_file(*(this->m_fout), this);
  });
  t.detach();
  */
}



// on_task_start
//   update min/max value of progress bar

void CartridgeBackupTask::on_task_start(int work_expected)
{
  m_mutex->lock();
  task_controller::on_task_start(work_expected);
  m_progress->setMinimum(0);
  m_progress->setMaximum(get_task_expected_work());
  m_mutex->unlock();
}



// on_task_update
//   update value of loading bar

void CartridgeBackupTask::on_task_update(task_status status, int work_progress)
{
  m_mutex->lock();
  task_controller::on_task_update(status, work_progress);
  m_progress->setValue(get_task_work_progress());
  QApplication::processEvents();
  m_mutex->unlock();
}



// on_task_end
//   close everything
//   delete progress bar
//   delete self

void CartridgeBackupTask::on_task_end(task_status status, int work_total)
{
  m_mutex->lock();
  task_controller::on_task_end(status, work_total);
  m_mutex->unlock();
}



// is_task_cancelled
//   forward request to progress dialog

bool CartridgeBackupTask::is_task_cancelled() const
{
  m_mutex->lock();
  auto r = m_progress->wasCanceled();
  m_mutex->unlock();
  return r;
}
