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


MainWindow::MainWindow(QWidget *parent) 
  : QMainWindow(parent), ui(new Ui::MainWindow),
    m_target_system(system_type::UNKNOWN)
{
  ui->setupUi(this);
  
  ui->deviceListWidget->addItem("Neo Geo Pocket Linkmasta");
  ui->deviceListWidget->addItem("WonderSwan FlashMasta");
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
