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
  
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  ui->textBrowser->setFont(fixedFont);
  ui->combobox_system_type->setCurrentIndex(0);
  
  print_to_console("");
  print_to_console("-=> NeoLinkMasta GUI v%d.%d build %d <=-", 1, 2, 1001);
  print_to_console("");
  print_to_console("     (c) Mr.Spiv & Flavor in 2011.");
  print_to_console("");
}

MainWindow::~MainWindow()
{
  delete ui;
}



void MainWindow::print_to_console(const char* message, ...)
{
  char buf[1024];
  va_list args;
  
  // Format string and arguments and store in buffer
  va_start(args, message);
  vsprintf(buf, message, args);
  
  // Append message to console
  ui->textBrowser->append(buf);
  
  va_end(args);
}



void MainWindow::on_combobox_system_type_currentIndexChanged(int index)
{
  switch (index)
  {
  case 0:
    enable_buttons(ROM_FLASH | ROM_BACKUP | ROM_VERIFY | SAVE_RESTORE | SAVE_BACKUP);
    m_target_system = system_type::NEO_GEO_POCKET;
    break;
    
  case 1:
    enable_buttons(ROM_FLASH | ROM_BACKUP | ROM_VERIFY | SAVE_RESTORE | SAVE_BACKUP);
    m_target_system = system_type::WONDERSWAN;
    break;
    
  default:
    enable_buttons(0);
    m_target_system = system_type::UNKNOWN;
    break;
  }
}

void MainWindow::on_button_backup_rom_clicked()
{
  switch (m_target_system)
  {
  case NEO_GEO_POCKET:
    NgpCartridgeBackupTask(this).go();
    break;
    
  case WONDERSWAN:    
    WsCartridgeBackupTask(this).go();
    break;
    
  default:
    break;
  }
}

void MainWindow::on_button_verify_rom_clicked()
{
  switch (m_target_system)
  {
  case NEO_GEO_POCKET:
    NgpCartridgeVerifyTask(this).go();
    break;
    
  case WONDERSWAN:
    WsCartridgeVerifyTask(this).go();
    break;
    
  default:
    break;
  }
}

void MainWindow::on_button_flash_rom_clicked()
{
  switch (m_target_system)
  {
  case NEO_GEO_POCKET:
    NgpCartridgeFlashTask(this).go();
    break;
    
  case WONDERSWAN:
    WsCartridgeFlashTask(this).go();
    break;
    
  default:
    break;
  }
}

void MainWindow::on_button_backup_save_clicked()
{
  switch (m_target_system)
  {
  case NEO_GEO_POCKET:
    NgpCartridgeBackupSaveTask(this).go();
    break;
    
  default:
    WsCartridgeBackupSaveTask(this).go();
    break;
  }
}

void MainWindow::on_button_restore_save_clicked()
{
  switch (m_target_system)
  {
  case NEO_GEO_POCKET:
    NgpCartridgeRestoreSaveTask(this).go();
    break;
    
  default:
    WsCartridgeRestoreSaveTask(this).go();
    break;
  }
}



void MainWindow::enable_buttons(int buttons)
{
  Ui::MainWindow* ui= this->ui;
  
  ui->button_flash_rom->setEnabled((buttons & button_flags::ROM_FLASH) != 0);
  ui->button_backup_rom->setEnabled((buttons & button_flags::ROM_BACKUP) != 0);
  ui->button_verify_rom->setEnabled((buttons & button_flags::ROM_VERIFY) != 0);
  ui->button_restore_save->setEnabled((buttons & button_flags::SAVE_RESTORE) != 0);
  ui->button_backup_save->setEnabled((buttons & button_flags::SAVE_BACKUP) != 0);
  ui->button_cartridge_info->setEnabled((buttons & button_flags::CARTRIDGE_INFO) != 0);
  ui->button_preferences->setEnabled((buttons & button_flags::PREFERENCES) != 0);
}


