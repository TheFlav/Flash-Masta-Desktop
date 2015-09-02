#include "main_window.h"
#include "ui_mainwindow.h"
#include <qfontdatabase.h>
#include <qfiledialog.h>
#include "../../hardware/PC-App-CLI/NeoLinkmasta.h"
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


MainWindow::MainWindow(QWidget *parent) 
  : QMainWindow(parent), ui(new Ui::MainWindow),
    m_target_system(system_type::UNKNOWN), m_timer(this)
{
  ui->setupUi(this);
  
  ui->deviceListWidget->addItem("Neo Geo Pocket Linkmasta");
  ui->deviceListWidget->addItem("WonderSwan FlashMasta");
  
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_checkDevices_timer()));
  m_timer.start(1000);
}

MainWindow::~MainWindow()
{
  m_timer.stop();
  
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



void MainWindow::on_checkDevices_timer()
{
  int ngp_count = 0;
  int ws_count = 0;
  
  libusb_context* libusb;
  libusb_device** devices;
  
  libusb_init(&libusb);
  int num_devices = libusb_get_device_list(libusb, &devices);
  
  for (int i = 0; i < num_devices; ++i)
  {
    libusb_device_descriptor desc;
    libusb_get_device_descriptor(devices[i], &desc);
    if (desc.idVendor == 0x20A0 && desc.idProduct == 0x4178)
    {
      ++ngp_count;
    }
    if (desc.idVendor == 0x20A0 && desc.idProduct == 0x4252)
    {
      ++ws_count;
    }
  }
  
  libusb_free_device_list(devices, 1);
  libusb_exit(libusb);
  
  if (ui->deviceListWidget->count() != (ngp_count + ws_count))
  {
    ui->deviceListWidget->clear();
    for (int i = 0; i < ngp_count; ++i)
    {
      ui->deviceListWidget->addItem("NGP");
    }
    for (int i = 0; i < ws_count; ++i)
    {
      ui->deviceListWidget->addItem("WS");
    }
  }
  
  m_timer.start(1000);
}


