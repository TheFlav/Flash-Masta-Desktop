#include "main_window.h"
#include "ui_mainwindow.h"
#include <qfontdatabase.h>
#include <qfiledialog.h>
#include "../../hardware/PC-App-CLI/NeoLinkmasta.h"
#include "ngp_cartridge_backup_task.h"
#include "ngp_cartridge_verify_task.h"
#include "ngp_cartridge_flash_task.h"
#include "ngp_cartridge_backup_save_task.h"
#include "ngp_cartridge_restore_save_task.h"



MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  ui->textBrowser->setFont(fixedFont);
  
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

void MainWindow::on_button_backup_rom_clicked()
{
  NgpCartridgeBackupTask(this).go();
}

void MainWindow::on_button_verify_rom_clicked()
{
  NgpCartridgeVerifyTask(this).go();
}

void MainWindow::on_button_flash_rom_clicked()
{
  NgpCartridgeFlashTask(this).go();
}

void MainWindow::on_button_backup_save_clicked()
{
  NgpCartridgeBackupSaveTask(this).go();
}

void MainWindow::on_button_restore_save_clicked()
{
  NgpCartridgeRestoreSaveTask(this).go();
}
