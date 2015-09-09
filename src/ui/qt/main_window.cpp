#include "main_window.h"
#include "ui_mainwindow.h"

#include <vector>
#include <QString>
#include <string>
#include "flash_masta.h"
#include "device_manager.h"
#include "task/ngp_cartridge_backup_task.h"
#include "task/ngp_cartridge_verify_task.h"
#include "task/ngp_cartridge_flash_task.h"
#include "task/ngp_cartridge_backup_save_task.h"
#include "task/ngp_cartridge_restore_save_task.h"
#include "task/ws_cartridge_backup_task.h"
#include "task/ws_cartridge_verify_task.h"
#include "task/ws_cartridge_flash_task.h"
#include "task/ws_cartridge_backup_save_task.h"
#include "task/ws_cartridge_restore_save_task.h"
#include "libusb-1.0/libusb.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent) 
  : QMainWindow(parent), ui(new Ui::MainWindow),
    m_target_system(system_type::UNKNOWN), m_timer(this), m_device_ids()
{
  ui->setupUi(this);
  
  // Start the automatic list refresh timer
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_refreshDeviceList_timeout()));
  m_timer.start(10);
}

MainWindow::~MainWindow()
{
  delete ui;
}



void MainWindow::on_actionBackupROM_triggered()
{
  switch (ui->deviceListWidget->currentRow())
  {
  case 0:
    NgpCartridgeBackupTask(this).go();
    break;
    
  case 1:
    WsCartridgeBackupTask(this).go();
    break;
  }
}

void MainWindow::on_actionRestoreROM_triggered()
{
  switch (ui->deviceListWidget->currentRow())
  {
  case 0:
    NgpCartridgeFlashTask(this).go();
    break;
    
  case 1:
    WsCartridgeFlashTask(this).go();
    break;
  }
}

void MainWindow::on_actionVerifyROM_triggered()
{
  switch (ui->deviceListWidget->currentRow())
  {
  case 0:
    NgpCartridgeVerifyTask(this).go();
    break;
    
  case 1:
    WsCartridgeVerifyTask(this).go();
    break;
  }
}

void MainWindow::on_actionBackupSave_triggered()
{
  switch (ui->deviceListWidget->currentRow())
  {
  case 0:
    NgpCartridgeBackupSaveTask(this).go();
    break;
    
  case 1:
    WsCartridgeBackupSaveTask(this).go();
    break;
  }
}

void MainWindow::on_actionRestoreSave_triggered()
{
  switch (ui->deviceListWidget->currentRow())
  {
  case 0:
    NgpCartridgeRestoreSaveTask(this).go();
    break;
    
  case 1:
    WsCartridgeRestoreSaveTask(this).go();
    break;
  }
}

void MainWindow::on_refreshDeviceList_timeout()
{
  vector<unsigned int> devices;
  
  if (FlashMasta::get_instance()->get_device_manager()->try_get_connected_devices(devices))
  {
    // Compare known devices with new devices and update list
    for (int i = 0, j = 0; i < m_device_ids.size() || j < devices.size();)
    {
      if (i < m_device_ids.size() && j < devices.size())
      {
        if (m_device_ids[i] < devices[j])
        {
          // Device has been disconnected
          delete ui->deviceListWidget->takeItem(i);
          m_device_ids.erase(m_device_ids.begin() + i);
        }
        else if (m_device_ids[i] > devices[j])
        {
          // Devices was skipped/added in the middle
          ui->deviceListWidget->insertItem(i, QString::number(devices[j]));
          m_device_ids.insert(m_device_ids.begin() + i, devices[j]);
          
          ++i;
          ++j;
        }
        else
        {
          ++i;
          ++j;
        }
      }
      else if (i < m_device_ids.size())
      {
        // Device was disconnected
        delete ui->deviceListWidget->takeItem(i);
        m_device_ids.erase(m_device_ids.begin() + i);
      }
      else if (j < devices.size())
      {
        // Device was connected
        QListWidgetItem *item = new QListWidgetItem(QString::number(devices[j]));
        auto size = item->sizeHint();
        size.setHeight(40);
        item->setSizeHint(size);
        ui->deviceListWidget->insertItem(i, item);
        m_device_ids.insert(m_device_ids.begin() + i, devices[j]);
        
        ++i;
        ++j;
      }
    }
  }
  
  m_timer.start(10);
}


